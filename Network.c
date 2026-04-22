#include "TetriC.h"

#define NET_HEADER_SIZE 3
#define NET_MAX_PAYLOAD 1024
#define NET_RECV_BUFFER_SIZE 4096
#define NET_MSG_SETUP 'U'
#define NET_MSG_STATE 'S'
#define NET_MSG_CONTROL 'C'
#define NET_MSG_WINNER 'W'
#define NET_SETUP_PAYLOAD_SIZE 6
#define NET_CONTROL_PAYLOAD_SIZE 2
#define NET_STATE_PAYLOAD_SIZE (1 + 1 + 1 + 1 + 1 + 1 + 1 + 16 + 4 + 4 + HEIGHT * WIDTH)

static unsigned char recvBuffers[MAX_PLAYERS][NET_RECV_BUFFER_SIZE];
static int recvSizes[MAX_PLAYERS] = {0};

static int remotePlayerStart(void) {
	return gameMode == 1 ? 1 : 2;
}

static int remotePlayerEnd(void) {
	if (!networkSessionActive()) return 0;
	return gameMode == 1 ? 1 : totalPlayers;
}

static void markClientConnectionClosed(void) {
	PlayerState* localPlayer = getPlayerState(localPlayerId);

	if (localPlayer != NULL) {
		localPlayer->connected = 0;
		localPlayer->gameOver = 1;
	}
	if (gameMode != 1 && winnerPlayerId == 0) {
		winnerPlayerId = findWinningPlayer();
		if (winnerPlayerId == 0) winnerPlayerId = -1;
	}
}

static void writeU32(unsigned char* dst, unsigned int value) {
	dst[0] = (unsigned char)((value >> 24) & 0xFF); // 0xFF in binary is:11111111
	dst[1] = (unsigned char)((value >> 16) & 0xFF);
	dst[2] = (unsigned char)((value >> 8) & 0xFF);
	dst[3] = (unsigned char)(value & 0xFF);
}

static unsigned int readU32(const unsigned char* src) {
	return ((unsigned int)src[0] << 24) |
		((unsigned int)src[1] << 16) |
		((unsigned int)src[2] << 8) |
		(unsigned int)src[3];
}

static void resetNetBuffers() {
	int i;
	for (i = 0; i < MAX_PLAYERS; i++) recvSizes[i] = 0;
	memset(recvBuffers, 0, sizeof(recvBuffers));
}

static int sendAllSocket(SOCKET sock, const unsigned char* data, int len) {
	int sent = 0;

	while (sent < len) {
		int n = send(sock, (const char*)data + sent, len - sent, 0);
		if (n == SOCKET_ERROR || n == 0) return 0;
		sent += n;
	}
	return 1;
}

static int recvAllSocket(SOCKET sock, unsigned char* data, int len) {
	int received = 0;

	while (received < len) {
		int n = recv(sock, (char*)data + received, len - received, 0);
		if (n <= 0) return 0;
		received += n;
	}
	return 1;
}

static int setSocketNonBlocking(SOCKET sock) {
	u_long mode = 1;// 1 -> non-blocking(recv() don't wait), 0 -> blocking mode
	if (sock == INVALID_SOCKET) return 1;
	return ioctlsocket(sock, FIONBIO, &mode) == 0;
}

static int netSendPacketTo(SOCKET sock, unsigned char type, const unsigned char* payload, unsigned short payloadLen) {
	unsigned char packet[NET_HEADER_SIZE + NET_MAX_PAYLOAD];

	if (sock == INVALID_SOCKET) return 0;
	if (payloadLen > NET_MAX_PAYLOAD) return 0;
	packet[0] = type;
	packet[1] = (unsigned char)((payloadLen >> 8) & 0xFF);
	packet[2] = (unsigned char)(payloadLen & 0xFF);
	if (payloadLen > 0 && payload != NULL) {
		memcpy(packet + NET_HEADER_SIZE, payload, payloadLen);
	}
	return sendAllSocket(sock, packet, NET_HEADER_SIZE + payloadLen);
}

static int serverBroadcastPacketExcept(int excludePlayerId, unsigned char type, const unsigned char* payload, unsigned short payloadLen) {
	int playerId;
	int ok = 1;

	for (playerId = remotePlayerStart(); playerId <= remotePlayerEnd(); playerId++) {
		if (playerId == excludePlayerId) continue;
		if (clientSocks[playerId - 1] != INVALID_SOCKET) {
			if (!netSendPacketTo(clientSocks[playerId - 1], type, payload, payloadLen)) {
				ok = 0;
			}
		}
	}
	return ok;
}

static int serverBroadcastPacket(unsigned char type, const unsigned char* payload, unsigned short payloadLen) {
	return serverBroadcastPacketExcept(0, type, payload, payloadLen);
}

static int packPlayerState(int playerId, unsigned char* out) {
	PlayerState* player = getPlayerState(playerId);
	int offset = 0;
	int r, c;

	if (player == NULL) return 0;
	out[offset++] = (unsigned char)playerId;
	out[offset++] = (unsigned char)(player->connected ? 1 : 0);
	out[offset++] = (unsigned char)(player->gameOver ? 1 : 0);
	out[offset++] = (unsigned char)((player->nextType >= 0 && player->nextType < 7) ? player->nextType : 255);
	out[offset++] = (unsigned char)(player->current.x + 128);
	out[offset++] = (unsigned char)(player->current.y + 128); // x,y can be negtive
	out[offset++] = (unsigned char)(player->current.type & 0xFF);
	for (r = 0; r < 4; r++) {
		for (c = 0; c < 4; c++) {
			out[offset++] = (unsigned char)(player->current.shape[r][c] ? 1 : 0);
		}
	}
	writeU32(out + offset, (unsigned int)player->score);
	offset += 4;
	writeU32(out + offset, (unsigned int)player->lines);
	offset += 4;
	for (r = 0; r < HEIGHT; r++) {
		for (c = 0; c < WIDTH; c++) {
			out[offset++] = (unsigned char)(player->board[r][c] ? 1 : 0);
		}
	}
	return offset;
}

static int unpackPlayerState(int forcedPlayerId, const unsigned char* payload, int payloadLen) {
	int playerId;
	PlayerState* player;
	int offset = 0;
	int r, c;

	if (payloadLen != NET_STATE_PAYLOAD_SIZE) return 0;
	playerId = forcedPlayerId > 0 ? forcedPlayerId : (int)payload[offset];
	if (playerId < 1 || playerId > MAX_PLAYERS) return 0;
	player = getPlayerState(playerId);
	if (player == NULL) return 0;
	offset++;
	player->connected = payload[offset++] ? 1 : 0;
	player->gameOver = payload[offset++] ? 1 : 0;
	player->nextType = payload[offset] == 255 ? -1 : (int)payload[offset];
	offset++;
	player->current.x = (int)payload[offset++] - 128;
	player->current.y = (int)payload[offset++] - 128;
	player->current.type = (int)payload[offset++];
	for (r = 0; r < 4; r++) {
		for (c = 0; c < 4; c++) {
			player->current.shape[r][c] = payload[offset++] ? 1 : 0;
		}
	}
	player->score = (int)readU32(payload + offset);
	offset += 4;
	player->lines = (int)readU32(payload + offset);
	offset += 4;
	for (r = 0; r < HEIGHT; r++) {
		for (c = 0; c < WIDTH; c++) {
			player->board[r][c] = payload[offset++] ? 1 : 0;
		}
	}
	player->active = playerId <= totalPlayers;
	return playerId;
}

static int broadcastPlayerState(int playerId, int excludePlayerId) {
	unsigned char payload[NET_STATE_PAYLOAD_SIZE];
	int payloadLen = packPlayerState(playerId, payload);
	return serverBroadcastPacketExcept(excludePlayerId, NET_MSG_STATE, payload, (unsigned short)payloadLen);
}

static void serverCheckWinner() {
	if (gameMode == 1) return;
	if (winnerPlayerId == 0) {
		int winner = findWinningPlayer();
		if (winner != 0) {
			winnerPlayerId = winner;
			netBroadcastWinner(winnerPlayerId);
		}
	}
}

static void closeClientSocket(int playerId) {
	if (playerId < 1 || playerId > MAX_PLAYERS) return;
	if (clientSocks[playerId - 1] != INVALID_SOCKET) {
		closesocket(clientSocks[playerId - 1]);
		clientSocks[playerId - 1] = INVALID_SOCKET;
	}
}

static void markPlayerDisconnected(int playerId) {
	PlayerState* player = getPlayerState(playerId);
	if (player == NULL) return;
	player->connected = 0;
	player->gameOver = 1;
	broadcastPlayerState(playerId, 0);
	serverCheckWinner();
	closeClientSocket(playerId);
}

static int processServerPacket(int playerId, unsigned char type, const unsigned char* payload, int payloadLen) {
	if (type == NET_MSG_STATE) {
		if (!unpackPlayerState(playerId, payload, payloadLen)) return 0;
		return broadcastPlayerState(playerId, playerId);
	}
	if (type == NET_MSG_CONTROL) {
		char code;
		if (payloadLen != NET_CONTROL_PAYLOAD_SIZE) return 0;
		code = (char)payload[1];
		if (code == 'P' || code == 'p') {
			paused = !paused;
			return serverBroadcastPacketExcept(playerId, NET_MSG_CONTROL, payload, payloadLen);
		}
		if (code == 'Q' || code == 'q') {
			markPlayerDisconnected(playerId);
			return 1;
		}
	}
	return 1;
}

static int processClientPacket(unsigned char type, const unsigned char* payload, int payloadLen) {
	if (type == NET_MSG_STATE) {
		if (payloadLen == NET_STATE_PAYLOAD_SIZE && (int)payload[0] == localPlayerId) {
			return 1;
		}
		return unpackPlayerState(0, payload, payloadLen) != 0;
	}
	if (type == NET_MSG_CONTROL) {
		if (payloadLen != NET_CONTROL_PAYLOAD_SIZE) return 0;
		applyRemoteInput((char)payload[1]);
		return 1;
	}
	if (type == NET_MSG_WINNER) {
		if (payloadLen != 1) return 0;
		winnerPlayerId = (int)payload[0];
		return 1;
	}
	return 1;
}

static int processReceiveBuffer(int bufferIndex, int sourcePlayerId, int serverSide) {
	while (recvSizes[bufferIndex] >= NET_HEADER_SIZE) {
		unsigned char type = recvBuffers[bufferIndex][0];
		unsigned short payloadLen = (unsigned short)(((unsigned short)recvBuffers[bufferIndex][1] << 8) | recvBuffers[bufferIndex][2]);
		int packetLen = NET_HEADER_SIZE + (int)payloadLen;
		int ok;

		if (payloadLen > NET_MAX_PAYLOAD) return 0;
		if (recvSizes[bufferIndex] < packetLen) return 1;
		if (serverSide) ok = processServerPacket(sourcePlayerId, type, recvBuffers[bufferIndex] + NET_HEADER_SIZE, payloadLen);
		else ok = processClientPacket(type, recvBuffers[bufferIndex] + NET_HEADER_SIZE, payloadLen);
		if (!ok) return 0;
		if (recvSizes[bufferIndex] > packetLen) {
		  //memmove(*dest,*src, size);
			memmove(recvBuffers[bufferIndex], recvBuffers[bufferIndex] + packetLen, recvSizes[bufferIndex] - packetLen);
		}
		recvSizes[bufferIndex] -= packetLen;
	}
	return 1;
}

static int receiveSetupPacket() {
	unsigned char header[NET_HEADER_SIZE];
	unsigned char payload[NET_SETUP_PAYLOAD_SIZE];

	if (!recvAllSocket(netSock, header, NET_HEADER_SIZE)) return 0;
	if (header[0] != NET_MSG_SETUP) return 0;
	if ((((unsigned short)header[1] << 8) | header[2]) != NET_SETUP_PAYLOAD_SIZE) return 0;
	if (!recvAllSocket(netSock, payload, NET_SETUP_PAYLOAD_SIZE)) return 0;
	localPlayerId = (int)payload[0];
	totalPlayers = (int)payload[1];
	sessionSeed = readU32(payload + 2);
	return 1;
}

int netInit() {
	WSADATA wsa;
	int i;

	resetNetBuffers();
	for (i = 0; i < MAX_PLAYERS; i++) clientSocks[i] = INVALID_SOCKET;
	netSock = INVALID_SOCKET;
	listenSock = INVALID_SOCKET;
	return WSAStartup(MAKEWORD(2, 2), &wsa) == 0; //Winsock version 2.2
}

void netClose() {
	int i;

	if (netSock != INVALID_SOCKET) {
		closesocket(netSock);
		netSock = INVALID_SOCKET;
	}
	if (listenSock != INVALID_SOCKET) {
		closesocket(listenSock);
		listenSock = INVALID_SOCKET;
	}
	for (i = 0; i < MAX_PLAYERS; i++) {
		if (clientSocks[i] != INVALID_SOCKET) {
			closesocket(clientSocks[i]);
			clientSocks[i] = INVALID_SOCKET;
		}
	}
	resetNetBuffers();
}

void netCleanup() {
	netClose();
	WSACleanup();
}

int netStartServer(int expectedPlayers) {
	struct sockaddr_in addr;
	unsigned char payload[NET_SETUP_PAYLOAD_SIZE];
	int remoteCount;
	int playerId;
	int opt = 1;

	totalPlayers = expectedPlayers;
	localPlayerId = 1;
	sessionSeed = (unsigned int)time(NULL);
	remoteCount = networkRemoteCount();

	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSock == INVALID_SOCKET) return 0;
	setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt)); // Allow address/port reuse more easily
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET; // IPv4
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(NET_PORT); // host to network short
	if (bind(listenSock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		netClose();
		return 0;
	}
	if (listen(listenSock, remoteCount > 0 ? remoteCount : 1) == SOCKET_ERROR) {
		netClose();
		return 0;
	}

	for (playerId = remotePlayerStart(); playerId <= remotePlayerEnd(); playerId++) {
		clientSocks[playerId - 1] = accept(listenSock, NULL, NULL);
		if (clientSocks[playerId - 1] == INVALID_SOCKET) {
			netClose();
			return 0;
		}
	}

	payload[1] = (unsigned char)expectedPlayers;
	writeU32(payload + 2, sessionSeed);
	for (playerId = remotePlayerStart(); playerId <= remotePlayerEnd(); playerId++) {
		payload[0] = (unsigned char)playerId;
		if (!netSendPacketTo(clientSocks[playerId - 1], NET_MSG_SETUP, payload, NET_SETUP_PAYLOAD_SIZE)) {
			netClose();
			return 0;
		}
	}

	closesocket(listenSock);
	listenSock = INVALID_SOCKET;
	return 1;
}

int netStartClient(const char* ip) {
	struct addrinfo hints;
	struct addrinfo* result = NULL;
	struct addrinfo* it;
	char port[16];

	snprintf(port, sizeof(port), "%d", NET_PORT); // uses port 1609 automatically
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	if (getaddrinfo(ip, port, &hints, &result) != 0) return 0;

	for (it = result; it != NULL; it = it->ai_next) {
		SOCKET sock = socket(it->ai_family, it->ai_socktype, it->ai_protocol); // Create a socket
		if (sock == INVALID_SOCKET) continue;
		if (connect(sock, it->ai_addr, (int)it->ai_addrlen) == 0) {
			netSock = sock;
			break;
		}
		closesocket(sock);
	}
	freeaddrinfo(result);
	if (netSock == INVALID_SOCKET) return 0;
	return receiveSetupPacket();
}

unsigned int netSyncSeed() {
	if (sessionSeed == 0) sessionSeed = (unsigned int)time(NULL);
	return sessionSeed;
}

int netSetNonBlocking() {
	int ok = 1;
	int playerId;

	if (netRole == ROLE_SERVER) {
		for (playerId = remotePlayerStart(); playerId <= remotePlayerEnd(); playerId++) {
			if (!setSocketNonBlocking(clientSocks[playerId - 1])) ok = 0;
		}
		return ok;
	}
	return setSocketNonBlocking(netSock);
}

int netSendInput(char code) {
	unsigned char payload[NET_CONTROL_PAYLOAD_SIZE];

	if (!networkSessionActive()) return 1;
	payload[0] = (unsigned char)localPlayerId;
	payload[1] = (unsigned char)code;
	if (netRole == ROLE_SERVER) {
		if (code == 'P' || code == 'p') {
			return serverBroadcastPacket(NET_MSG_CONTROL, payload, NET_CONTROL_PAYLOAD_SIZE) ? 1 : -1;
		}
		return 1;
	}
	return netSendPacketTo(netSock, NET_MSG_CONTROL, payload, NET_CONTROL_PAYLOAD_SIZE) ? 1 : -1;
}

int netSendState(int playerId) {
	unsigned char payload[NET_STATE_PAYLOAD_SIZE];
	int payloadLen;

	if (!networkSessionActive()) return 1;
	payloadLen = packPlayerState(playerId, payload);
	if (netRole == ROLE_SERVER) {
		return serverBroadcastPacket(NET_MSG_STATE, payload, (unsigned short)payloadLen) ? 1 : -1;
	}
	return netSendPacketTo(netSock, NET_MSG_STATE, payload, (unsigned short)payloadLen) ? 1 : -1;
}

int netBroadcastWinner(int winnerId) {
	unsigned char payload[1];

	if (netRole != ROLE_SERVER || !networkSessionActive() || gameMode == 1) return 1;
	payload[0] = (unsigned char)winnerId;
	return serverBroadcastPacket(NET_MSG_WINNER, payload, 1);
}

//Check whether any network input has arrived, and process it
void netPollInputs() {
	if (!networkSessionActive()) return;

	if (netRole == ROLE_SERVER) {
		fd_set readfds;
		struct timeval timeout;
		int playerId;
		int readyCount;

		FD_ZERO(&readfds);
		for (playerId = remotePlayerStart(); playerId <= remotePlayerEnd(); playerId++) {
			if (clientSocks[playerId - 1] != INVALID_SOCKET) {
				FD_SET(clientSocks[playerId - 1], &readfds);
			}

		}
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
		readyCount = select(0, &readfds, NULL, NULL, &timeout);
		if (readyCount == SOCKET_ERROR || readyCount == 0) return;

		for (playerId = remotePlayerStart(); playerId <= remotePlayerEnd(); playerId++) {
			SOCKET sock = clientSocks[playerId - 1];
			if (sock != INVALID_SOCKET && FD_ISSET(sock, &readfds)) {
				char temp[NET_BUF * 8];
				int n = recv(sock, temp, sizeof(temp), 0);
				if (n == 0) {
					markPlayerDisconnected(playerId);
					continue;
				}
				if (n == SOCKET_ERROR) {
					int err = WSAGetLastError();
					if (err != WSAEWOULDBLOCK) markPlayerDisconnected(playerId);
					continue;
				}
				if (recvSizes[playerId - 1] + n > NET_RECV_BUFFER_SIZE) {
					markPlayerDisconnected(playerId);
					continue;
				}
				memcpy(recvBuffers[playerId - 1] + recvSizes[playerId - 1], temp, n);
				recvSizes[playerId - 1] += n;
				if (!processReceiveBuffer(playerId - 1, playerId, 1)) {
					markPlayerDisconnected(playerId);
				}
			}
		}
		serverCheckWinner();
		return;
	}

	{
		fd_set readfds;
		struct timeval timeout;
		int readyCount;

		FD_ZERO(&readfds);
		FD_SET(netSock, &readfds);
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
		// select() is a socket function that checks socket readiness.
		readyCount = select(0, &readfds, NULL, NULL, &timeout);
		if (readyCount == SOCKET_ERROR || readyCount == 0) return;
		if (FD_ISSET(netSock, &readfds)) {
			char temp[NET_BUF * 8];
			int n = recv(netSock, temp, sizeof(temp), 0);
			if (n == 0) {
				markClientConnectionClosed();
				return;
			}
			if (n == SOCKET_ERROR) {
				int err = WSAGetLastError();
				if (err != WSAEWOULDBLOCK) markClientConnectionClosed();
				return;
			}
			if (recvSizes[0] + n > NET_RECV_BUFFER_SIZE) {
				markClientConnectionClosed();
				return;
			}
			memcpy(recvBuffers[0] + recvSizes[0], temp, n);
			recvSizes[0] += n;
			if (!processReceiveBuffer(0, 0, 0)) {
				markClientConnectionClosed();
			}
		}
	}
}