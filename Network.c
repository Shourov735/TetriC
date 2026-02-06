#include "TetriC.h"

int netInit() {
	WSADATA wsa;
	return WSAStartup(MAKEWORD(2,2), &wsa) == 0;
}

void netClose() {
	if (netSock != INVALID_SOCKET) {
		closesocket(netSock);
		netSock = INVALID_SOCKET;
	}
	if (listenSock != INVALID_SOCKET) {
		closesocket(listenSock);
		listenSock = INVALID_SOCKET;
	}
}

void netCleanup() {
	netClose();
	WSACleanup();
}

int netStartServer() {
	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSock == INVALID_SOCKET) return 0;

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(NET_PORT);

	if (bind(listenSock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) return 0;
	if (listen(listenSock, 1) == SOCKET_ERROR) return 0;

	netSock = accept(listenSock, NULL, NULL);
	if (netSock == INVALID_SOCKET) return 0;
	closesocket(listenSock);
	listenSock = INVALID_SOCKET;
	return 1;
}

int netStartClient(const char* ip) {
	netSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (netSock == INVALID_SOCKET) return 0;

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(NET_PORT);
	addr.sin_addr.s_addr = inet_addr(ip);

	if (connect(netSock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) return 0;
	return 1;
}

int netHandshake() {
	char buf[NET_BUF];
	int n = 0;
	if (netRole == 1) {
		n = recv(netSock, buf, NET_BUF - 1, 0);
		if (n <= 0) return 0;
		buf[n] = '\0';
		if (strcmp(buf, "PING") != 0) return 0;
		if (send(netSock, "PONG", 4, 0) <= 0) return 0;
		return 1;
	} else {
		if (send(netSock, "PING", 4, 0) <= 0) return 0;
		n = recv(netSock, buf, NET_BUF - 1, 0);
		if (n <= 0) return 0;
		buf[n] = '\0';
		return strcmp(buf, "PONG") == 0;
	}
}
