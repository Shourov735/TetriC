#include "TetriC.h"

PlayerState* getPlayerState(int playerId) {
	if (playerId < 1 || playerId > MAX_PLAYERS) return NULL;
	return &players[playerId - 1];
}

int (*getPlayerBoard(int playerId))[WIDTH] {
	PlayerState* player = getPlayerState(playerId);
	if (player == NULL) return NULL;
	return player->board;
}

Piece* getPlayerPiece(int playerId) {
	PlayerState* player = getPlayerState(playerId);
	if (player == NULL) return NULL;
	return &player->current;
}

int collisionPlayer(int playerId, int x, int y, int shape[4][4]) {
	PlayerState* player = getPlayerState(playerId);
	int r, c;

	if (player == NULL) return 1;
	for (r = 0; r < 4; r++) {
		for (c = 0; c < 4; c++) {
			if (shape[r][c]) {
				int nx = x + c;
				int ny = y + r;
				if (nx < 0 || nx >= WIDTH || ny >= HEIGHT) return 1;
				if (ny >= 0 && player->board[ny][nx]) return 1;
			}
		}
	}
	return 0;
}

void copyShape(int dst[4][4], int src[4][4]) {
	int r, c;
	for (r = 0; r < 4; r++) {
		for (c = 0; c < 4; c++) {
			dst[r][c] = src[r][c];
		}
	}
}

void rotate(int mat[4][4]) {
	int temp[4][4];
	int r, c;

	copyShape(temp, mat);
	for (r = 0; r < 4; r++) {
		for (c = 0; c < 4; c++) {
			mat[c][3 - r] = temp[r][c];
		}
	}
}

void newPieceForPlayer(int playerId) {
	PlayerState* player = getPlayerState(playerId);
	int r, c;

	if (player == NULL || !player->active) return;
	if (player->nextType < 0) player->nextType = rand() % 7;
	player->current.type = player->nextType;
	player->nextType = rand() % 7;
	for (r = 0; r < 4; r++) {
		for (c = 0; c < 4; c++) {
			player->current.shape[r][c] = shapes[player->current.type][r][c];
		}
	}
	player->current.x = WIDTH / 2 - 2;
	player->current.y = -2;
	if (collisionPlayer(playerId, player->current.x, player->current.y, player->current.shape)) {
		player->gameOver = 1;
	}
}

void mergePieceForPlayer(int playerId) {
	PlayerState* player = getPlayerState(playerId);
	int r, c;

	if (player == NULL) return;
	for (r = 0; r < 4; r++) {
		for (c = 0; c < 4; c++) {
			if (player->current.shape[r][c]) {
				int nx = player->current.x + c;
				int ny = player->current.y + r;
				if (ny >= 0 && ny < HEIGHT && nx >= 0 && nx < WIDTH) {
					player->board[ny][nx] = 1;
				}
			}
		}
	}
}

int clearLinesForPlayer(int playerId) {
	PlayerState* player = getPlayerState(playerId);
	int cleared = 0;
	int r, c;

	if (player == NULL) return 0;
	for (r = HEIGHT - 1; r >= 0; r--) {
		int full = 1;
		for (c = 0; c < WIDTH; c++) {
			if (!player->board[r][c]) full = 0;
		}
		if (full) {
			int rr, cc;
			cleared++;
			for (rr = r; rr > 0; rr--) {
				for (cc = 0; cc < WIDTH; cc++) {
					player->board[rr][cc] = player->board[rr - 1][cc];
				}
			}
			for (cc = 0; cc < WIDTH; cc++) player->board[0][cc] = 0;
			r++;
		}
	}

	if (cleared > 0) {
		int add = 0;
		if (cleared == 1) add = 100;
		else if (cleared == 2) add = 300;
		else if (cleared == 3) add = 500;
		else add = 800;
		player->score += add * level;
		player->lines += cleared;
	}
	return cleared;
}

int moveDownForPlayer(int playerId) {
	PlayerState* player = getPlayerState(playerId);

	if (player == NULL || player->gameOver) return 0;
	if (!collisionPlayer(playerId, player->current.x, player->current.y + 1, player->current.shape)) {
		player->current.y++;
		return 1;
	}
	mergePieceForPlayer(playerId);
	clearLinesForPlayer(playerId);
	newPieceForPlayer(playerId);
	return 0;
}

void hardDropForPlayer(int playerId) {
	PlayerState* player = getPlayerState(playerId);
	int dropped = 0;

	if (player == NULL || player->gameOver) return;
	while (!collisionPlayer(playerId, player->current.x, player->current.y + 1, player->current.shape)) {
		player->current.y++;
		dropped++;
	}
	if (dropped > 0) player->score += dropped * 2;
	moveDownForPlayer(playerId);
}