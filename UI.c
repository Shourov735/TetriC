#include "TetriC.h"

static const char* safePieceName(int type) {
	if (type < 0 || type > 6) return "?";
	return pieceNames[type];
}

static const char* playerStatusText(int playerId) {
	PlayerState* player = getPlayerState(playerId);

	if (player == NULL || !player->active) return "---";
	if (!player->connected) return "DISC";
	if (player->gameOver) return "OUT";
	return "LIVE";
}

void gotoxy(int x, int y) {
	COORD coord = {x, y};
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(consoleHandle, coord);
}

void hidecursor() {
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO info;
	info.dwSize = 1;
	info.bVisible = FALSE;
	SetConsoleCursorInfo(consoleHandle, &info);
}

void clearScreen() {
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	DWORD cellCount;
	DWORD count;
	COORD homeCoords = {0, 0};

	if (GetConsoleScreenBufferInfo(consoleHandle, &csbi)) {
		cellCount = csbi.dwSize.X * csbi.dwSize.Y;
		FillConsoleOutputCharacter(consoleHandle, ' ', cellCount, homeCoords, &count);
		FillConsoleOutputAttribute(consoleHandle, csbi.wAttributes, cellCount, homeCoords, &count);
		SetConsoleCursorPosition(consoleHandle, homeCoords);
	}
}

void clearSide(char side[HEIGHT][SIDE_W + 1]) {
	int r, c;
	for (r = 0; r < HEIGHT; r++) {
		for (c = 0; c < SIDE_W; c++) side[r][c] = ' ';
		side[r][SIDE_W] = '\0';
	}
}

void setSideLine(char side[HEIGHT][SIDE_W + 1], int row, const char* text) {
	int i;
	if (row < 0 || row >= HEIGHT) return;
	for (i = 0; i < SIDE_W && text[i]; i++) side[row][i] = text[i];
	for (; i < SIDE_W; i++) side[row][i] = ' ';
	side[row][SIDE_W] = '\0';
}

void buildPreviewRow(int type, int row, char* rowLine) {
	int idx = 0;
	int c;

	for (c = 0; c < 4; c++) {
		if (type >= 0 && type < 7 && shapes[type][row][c]) {
			rowLine[idx++] = '[';
			rowLine[idx++] = ']';
		} else {
			rowLine[idx++] = ' ';
			rowLine[idx++] = ' ';
		}
	}
	rowLine[idx] = '\0';
}

void setPreviewBox(char side[HEIGHT][SIDE_W + 1], int startRow, int type) {
	char line[32];
	char rowLine[16];
	int r;

	setSideLine(side, startRow, "+--------+");
	for (r = 0; r < 4; r++) {
		buildPreviewRow(type, r, rowLine);
		snprintf(line, sizeof(line), "|%s|", rowLine);
		setSideLine(side, startRow + 1 + r, line);
	}
	setSideLine(side, startRow + 5, "+--------+");
}

static void printBoardCells(int playerId, int row) {
	PlayerState* player = getPlayerState(playerId);
	int c;

	printf("|");
	for (c = 0; c < WIDTH; c++) {
		int filled = 0;
		int active = 0;

		if (player != NULL) {
			int ar = row - player->current.y;
			int ac = c - player->current.x;
			filled = player->board[row][c];
			if (!player->gameOver && ar >= 0 && ar < 4 && ac >= 0 && ac < 4 && player->current.shape[ar][ac]) {
				active = 1;
			}
		}
		printf(filled || active ? "[]" : "  ");
	}
	printf("|");
}

static void drawSingleBoardView(void) {
	char side[HEIGHT][SIDE_W + 1];
	char line[96];
	int r;

	clearSide(side);
	snprintf(line, sizeof(line), "Difficulty: %s", diffNames[difficulty - 1]);
	setSideLine(side, 0, line);
	snprintf(line, sizeof(line), "Score: %d", score1);
	setSideLine(side, 1, line);
	snprintf(line, sizeof(line), "High: %d", highScore1);
	setSideLine(side, 2, line);
	snprintf(line, sizeof(line), "Lines: %d", lines1);
	setSideLine(side, 3, line);
	snprintf(line, sizeof(line), "Level: %d", level);
	setSideLine(side, 4, line);
	snprintf(line, sizeof(line), "Speed: %dms", speed);
	setSideLine(side, 5, line);
	snprintf(line, sizeof(line), "Status: %s", paused ? "PAUSED" : "RUNNING");
	setSideLine(side, 6, line);
	snprintf(line, sizeof(line), "Next Piece: %s", safePieceName(nextType1));
	setSideLine(side, 7, line);
	setPreviewBox(side, 8, nextType1);
	setSideLine(side, 14, "Controls:");
	setSideLine(side, 15, "A/D: Move");
	setSideLine(side, 16, "W: Rotate");
	setSideLine(side, 17, "S: Soft drop (+1)");
	setSideLine(side, 18, "Z: Hard drop (+2/row)");
	setSideLine(side, 19, "P: Pause   Q: Quit");

	printf("Player 1\n");
	for (r = 0; r < HEIGHT; r++) {
		printBoardCells(1, r);
		printf("  %-*s\n", SIDE_W, side[r]);
	}
	printf("+--------------------+\n");
}

static void drawTwoBoardView(void) {
	char side[HEIGHT][SIDE_W + 1];
	char line[96];
	int r;

	clearSide(side);
	snprintf(line, sizeof(line), "Difficulty: %s  Level: %d", diffNames[difficulty - 1], level);
	setSideLine(side, 0, line);
	snprintf(line, sizeof(line), "Speed: %dms  Status: %s", speed, paused ? "PAUSED" : "RUNNING");
	setSideLine(side, 1, line);
	snprintf(line, sizeof(line), "Score Player1: %d  Player2: %d", score1, score2);
	setSideLine(side, 2, line);
	if (!networkMode) {
		snprintf(line, sizeof(line), "High  Player1: %d  Player2: %d", highScore1, highScore2);
		setSideLine(side, 3, line);
	} else {
		snprintf(line, sizeof(line), "Online: You=P%d  Winner: %s", localPlayerId, winnerPlayerId > 0 ? "FOUND" : "TBD");
		setSideLine(side, 3, line);
	}
	snprintf(line, sizeof(line), "Lines Player1: %d  Player2: %d", lines1, lines2);
	setSideLine(side, 4, line);
	snprintf(line, sizeof(line), "Player1 Next: %s", safePieceName(nextType1));
	setSideLine(side, 5, line);
	setPreviewBox(side, 6, nextType1);
	snprintf(line, sizeof(line), "Player2 Next: %s", safePieceName(nextType2));
	setSideLine(side, 12, line);
	setPreviewBox(side, 13, nextType2);
	if (networkMode) {
		setSideLine(side, 19, "Online: WASD/Z or Arrows/Space  P/Q");
	} else {
		setSideLine(side, 19, "P1: WASD/Z  P2: Arrows/Space  P/Q");
	}

	printf("Player 1              Player 2\n");
	for (r = 0; r < HEIGHT; r++) {
		printBoardCells(1, r);
		printf("  ");
		printBoardCells(2, r);
		printf("  %-*s\n", SIDE_W, side[r]);
	}
	printf("+--------------------+  +--------------------+\n");
}

static void drawMultiplayerView(void) {
	char side[HEIGHT][SIDE_W + 1];
	char line[96];
	PlayerState* localPlayer = getPlayerState(localPlayerId);
	int r;
	int sideRow = 14;

	clearSide(side);
	snprintf(line, sizeof(line), "Players: %d  Alive: %d", totalPlayers, countAlivePlayers());
	setSideLine(side, 0, line);
	snprintf(line, sizeof(line), "You: P%d  Status: %s", localPlayerId, playerStatusText(localPlayerId));
	setSideLine(side, 1, line);
	if (winnerPlayerId > 0) {
		snprintf(line, sizeof(line), "Winner: Player %d", winnerPlayerId);
	} else if (winnerPlayerId < 0) {
		snprintf(line, sizeof(line), "Winner: Match Aborted");
	} else {
		snprintf(line, sizeof(line), "Winner: TBD");
	}
	setSideLine(side, 2, line);
	snprintf(line, sizeof(line), "Difficulty: %s  Level: %d", diffNames[difficulty - 1], level);
	setSideLine(side, 3, line);
	snprintf(line, sizeof(line), "Speed: %dms  Status: %s", speed, paused ? "PAUSED" : "RUNNING");
	setSideLine(side, 4, line);
	snprintf(line, sizeof(line), "Local Score: %d", localPlayer != NULL ? localPlayer->score : 0);
	setSideLine(side, 5, line);
	snprintf(line, sizeof(line), "Local Lines: %d", localPlayer != NULL ? localPlayer->lines : 0);
	setSideLine(side, 6, line);
	snprintf(line, sizeof(line), "Next Piece: %s", localPlayer != NULL ? safePieceName(localPlayer->nextType) : "?");
	setSideLine(side, 7, line);
	setPreviewBox(side, 8, localPlayer != NULL ? localPlayer->nextType : -1);
	setSideLine(side, 13, "Standings:");

	for (r = 1; r <= totalPlayers && sideRow < HEIGHT; r++, sideRow++) {
		PlayerState* player = getPlayerState(r);
		snprintf(line, sizeof(line), "P%d %-4s Score:%4d Lines:%3d", r, playerStatusText(r), player != NULL ? player->score : 0, player != NULL ? player->lines : 0);
		setSideLine(side, sideRow, line);
	}

	printf("Player %d\n", localPlayerId);
	for (r = 0; r < HEIGHT; r++) {
		printBoardCells(localPlayerId, r);
		printf("  %-*s\n", SIDE_W, side[r]);
	}
	printf("+--------------------+\n");
}

void drawBoard() {
	gotoxy(0, 0);
	if (gameMode == 1 && totalPlayers == 1) {
		drawSingleBoardView();
	} else if (totalPlayers <= 2) {
		drawTwoBoardView();
	} else {
		drawMultiplayerView();
	}
}
