#include "TetriC.h"

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

void clearSide(char side[HEIGHT][SIDE_W+1]) {
	for (int r=0;r<HEIGHT;r++) {
		for (int c=0;c<SIDE_W;c++) side[r][c] = ' ';
		side[r][SIDE_W] = '\0';
	}
}

void setSideLine(char side[HEIGHT][SIDE_W+1], int row, const char* text) {
	if (row < 0 || row >= HEIGHT) return;
	int i = 0;
	for (; i < SIDE_W && text[i]; i++) side[row][i] = text[i];
	for (; i < SIDE_W; i++) side[row][i] = ' ';
	side[row][SIDE_W] = '\0';
}

void buildPreviewRow(int type, int row, char* rowLine) {
	int idx = 0;
	for (int c=0;c<4;c++) {
		if (shapes[type][row][c]) {
			rowLine[idx++] = '[';
			rowLine[idx++] = ']';
		} else {
			rowLine[idx++] = ' ';
			rowLine[idx++] = ' ';
		}
	}
	rowLine[idx] = '\0';
}

void setPreviewBox(char side[HEIGHT][SIDE_W+1], int startRow, int type) {
	char line[32];
	char rowLine[16];
	setSideLine(side, startRow, "+--------+");
	for (int r=0;r<4;r++) {
		buildPreviewRow(type, r, rowLine);
		sprintf(line, "|%s|", rowLine);
		setSideLine(side, startRow + 1 + r, line);
	}
	setSideLine(side, startRow + 5, "+--------+");
}

void drawBoard() {
    gotoxy(0,0);
	char side[HEIGHT][SIDE_W+1];
	char line[64];
	clearSide(side);
	if (gameMode == 1) {
		sprintf(line, "Difficulty: %s", diffNames[difficulty-1]);
		setSideLine(side, 0, line);
		sprintf(line, "Score: %d", score1);
		setSideLine(side, 1, line);
		sprintf(line, "High: %d", highScore1);
		setSideLine(side, 2, line);
		sprintf(line, "Lines: %d", lines1);
		setSideLine(side, 3, line);
		sprintf(line, "Level: %d", level);
		setSideLine(side, 4, line);
		sprintf(line, "Speed: %dms", speed);
		setSideLine(side, 5, line);
		sprintf(line, "Status: %s", paused ? "PAUSED" : "RUNNING");
		setSideLine(side, 6, line);
		sprintf(line, "Next Piece: %s", pieceNames[nextType1]);
		setSideLine(side, 7, line);
		setPreviewBox(side, 8, nextType1);
		setSideLine(side, 14, "Controls:");
		setSideLine(side, 15, "A/D: Move");
		setSideLine(side, 16, "W: Rotate");
		setSideLine(side, 17, "S: Soft drop (+1)");
		setSideLine(side, 18, "Z: Hard drop (+2/row)");
		setSideLine(side, 19, "P: Pause   Q: Quit");

        printf("Player 1\n");
        for (int r=0;r<HEIGHT;r++) {
            printf("|");
            for (int c=0;c<WIDTH;c++) {
                int filled = board1[r][c];
                int active = 0;
                int ar = r - current1.y;
                int ac = c - current1.x;
                if (ar>=0 && ar<4 && ac>=0 && ac<4 && current1.shape[ar][ac])
                    active=1;
                printf(filled || active ? "[]" : "  ");
            }
            printf("|  %-*s\n", SIDE_W, side[r]);
        }
        printf("+--------------------+\n");
    } else {
		sprintf(line, "Difficulty: %s  Level: %d", diffNames[difficulty-1], level);
		setSideLine(side, 0, line);
		sprintf(line, "Speed: %dms  Status: %s", speed, paused ? "PAUSED" : "RUNNING");
		setSideLine(side, 1, line);
		sprintf(line, "Score Player1: %d  Player2: %d", score1, score2);
		setSideLine(side, 2, line);
		sprintf(line, "High  Player1: %d  Player2: %d", highScore1, highScore2);
		setSideLine(side, 3, line);
		sprintf(line, "Lines Player1: %d  Player2: %d", lines1, lines2);
		setSideLine(side, 4, line);
		sprintf(line, "Player1 Next: %s", pieceNames[nextType1]);
		setSideLine(side, 5, line);
		setPreviewBox(side, 6, nextType1);
		sprintf(line, "Player2 Next: %s", pieceNames[nextType2]);
		setSideLine(side, 12, line);
		setPreviewBox(side, 13, nextType2);
		setSideLine(side, 19, "P1: WASD/Z  P2: Arrows/Space  P=Pause | Q=Quit");

    	printf("Player 1              Player 2            \n");
    	for (int r=0;r<HEIGHT;r++) {
        	printf("|");
        	for (int c=0;c<WIDTH;c++) {
            	int filled = board1[r][c];
            	int active = 0;
            	int ar = r - current1.y;
            	int ac = c - current1.x;
            	if (ar>=0 && ar<4 && ac>=0 && ac<4 && current1.shape[ar][ac])
                	active=1;
            	printf(filled || active ? "[]" : "  ");
        	}
        	printf("|  |");
        	for(int c=0;c<WIDTH;c++) {
            	int filled = board2[r][c];
            	int active = 0;
            	int ar = r - current2.y;
            	int ac = c - current2.x;
            	if (ar>=0 && ar<4 && ac>=0 && ac<4 && current2.shape[ar][ac])
                	active=1;
            	printf(filled || active ? "[]" : "  ");
        	}
        	printf("|  %-*s\n", SIDE_W, side[r]);
    	}
    	printf("+--------------------+  +--------------------+\n");
	}
}
