#include<stdio.h>
#include<stdlib.h>
#include<windows.h>
#include<conio.h>
#include<time.h>

#define WIDTH 10
#define HEIGHT 20
#define SIDE_W 50

int board1[HEIGHT][WIDTH] = {0};
int board2[HEIGHT][WIDTH] = {0};

typedef struct{
	int x, y;
	int type;
	int shape[4][4];
} Piece;

Piece current1;
Piece current2;
int score1 = 0;
int score2 = 0;
int lines1 = 0;
int lines2 = 0;
int gameOver1 = 0;
int gameOver2 = 0;
int gameMode = 1;
int difficulty = 2;
int level = 1;
int highScore1 = 0;
int highScore2 = 0;
int paused = 0;

int speed = 500;
int baseSpeed = 500;
int minSpeed = 120;
int speedStep = 20;
int speedInterval = 15000;

int nextType1 = -1;
int nextType2 = -1;

char highScoreFile1[64] = "";
char highScoreFile2[64] = "";

const char* pieceNames[7] = {"I","O","T","S","Z","J","L"};
const char* diffNames[3] = {"Easy","Medium","Hard"};

int shapes[7][4][4] = {
	// I
	{{0,0,0,0},
	 {1,1,1,1},
	 {0,0,0,0},
	 {0,0,0,0}},
	// O
	{{0,0,0,0},
	 {0,1,1,0},
	 {0,1,1,0},
	 {0,0,0,0}},
	// T
	{{0,0,0,0},
	 {1,1,1,0},
	 {0,1,0,0},
	 {0,0,0,0}},
	// S
	{{0,0,0,0},
	 {0,1,1,0},
	 {1,1,0,0},
	 {0,0,0,0}},
	// Z
	{{0,0,0,0},
	 {1,1,0,0},
	 {0,1,1,0},
	 {0,0,0,0}},
	// J
	{{0,0,0,0},
	 {1,1,1,0},
	 {0,0,1,0},
	 {0,0,0,0}},
	// L
	{{0,0,0,0},
	 {1,1,1,0},
	 {1,0,0,0},
	 {0,0,0,0}}
};

void setHighScoreFiles(int diff) {
	if (diff == 1) {
		strcpy(highScoreFile1, "highscore_easy_p1.txt");
		strcpy(highScoreFile2, "highscore_easy_p2.txt");
	} else if (diff == 3) {
		strcpy(highScoreFile1, "highscore_hard_p1.txt");
		strcpy(highScoreFile2, "highscore_hard_p2.txt");
	} else {
		strcpy(highScoreFile1, "highscore_medium_p1.txt");
		strcpy(highScoreFile2, "highscore_medium_p2.txt");
	}
}

int loadHighScore(const char* file) {
	FILE *f = fopen(file, "r");
	int s = 0;
	if (f) {
		if (fscanf(f, "%d", &s) != 1) s = 0;
		fclose(f);
	}
	return s;
}

void saveHighScore(const char* file, int score) {
	FILE *f = fopen(file, "w");
	if (!f) return;
	fprintf(f, "%d", score);
	fclose(f);
}

void setupDifficulty(int diff) {
	if (diff == 1) {
		baseSpeed = 650;
		minSpeed = 200;
		speedStep = 20;
		speedInterval = 20000;
	} else if (diff == 2) {
		baseSpeed = 500;
		minSpeed = 140;
		speedStep = 25;
		speedInterval = 15000;
		
	} else if( diff == 3) {
		baseSpeed = 350;
		minSpeed = 90;
		speedStep = 30;
		speedInterval = 12000;
	}
	speed = baseSpeed;
	level = 1;
}

void initNextPieces() {
	nextType1 = rand()%7;
	nextType2 = rand()%7;
}

int collision1(int x, int y, int shape[4][4]) {
	for (int r=0;r<4;r++)
		for (int c=0;c<4;c++)
			if (shape[r][c]) {
				int nx = x + c;
				int ny = y + r;
				if (nx < 0 || nx >= WIDTH || ny >= HEIGHT) return 1;
				if (ny >= 0 && board1[ny][nx]) return 1;
			}
	return 0;
}

int collision2(int x, int y, int shape[4][4]) {
	for (int r=0;r<4;r++)
		for (int c=0;c<4;c++)
			if (shape[r][c]) {
				int nx = x + c;
				int ny = y + r;
				if (nx < 0 || nx >= WIDTH || ny >= HEIGHT) return 1;
				if (ny >= 0 && board2[ny][nx]) return 1;
			}
	return 0;
}

void newPiece1() {
	if (nextType1 < 0) nextType1 = rand()%7;
	current1.type = nextType1;
	nextType1 = rand()%7;
	for (int r=0;r<4;r++)
		for (int c=0;c<4;c++)
			current1.shape[r][c] = shapes[current1.type][r][c];
	current1.x = WIDTH/2 - 2;
	current1.y = -2;
	if (collision1(current1.x, current1.y, current1.shape))
		gameOver1 = 1;
}

void newPiece2() {
	if (nextType2 < 0) nextType2 = rand()%7;
	current2.type = nextType2;
	nextType2 = rand()%7;
	for (int r=0;r<4;r++)
		for (int c=0;c<4;c++)
			current2.shape[r][c] = shapes[current2.type][r][c];
	current2.x = WIDTH/2 - 2;
	current2.y = -2;
	if (collision2(current2.x, current2.y, current2.shape))
		gameOver2 = 1;
}

void copyShape(int dst[4][4], int src[4][4]) {
	for (int r=0;r<4;r++)
		for (int c=0;c<4;c++)
			dst[r][c]=src[r][c];
}

void rotate(int mat[4][4]) {
	int temp[4][4];
	copyShape(temp, mat);
	for (int r=0;r<4;r++)
		for (int c=0;c<4;c++)
			mat[c][3-r] = temp[r][c];
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
		sprintf(line, "Next: %s", pieceNames[nextType1]);
		setSideLine(side, 7, line);
		setSideLine(side, 8, "Controls:");
		setSideLine(side, 9, "A/D: Move");
		setSideLine(side, 10, "W: Rotate");
		setSideLine(side, 11, "S: Soft drop (+1)");
		setSideLine(side, 12, "Z: Hard drop (+2/row)");
		setSideLine(side, 13, "P: Pause   Q: Quit");

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
		sprintf(line, "Player2 Next: %s", pieceNames[nextType2]);
		setSideLine(side, 6, line);
		setSideLine(side, 7, "Controls:");
		setSideLine(side, 8, "A/D: Move");
		setSideLine(side, 9, "W: Rotate");
		setSideLine(side, 10, "S: Soft drop (+1)");
		setSideLine(side, 11, "Z: Hard drop (+2/row)");
		setSideLine(side, 12, "P: Pause   Q: Quit");

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

void mergePiece1() {
	for (int r=0;r<4;r++)
		for (int c=0;c<4;c++)
			if (current1.shape[r][c]) {
				int nx = current1.x + c;
				int ny = current1.y + r;
				if (ny>=0 && ny<HEIGHT && nx>=0 && nx<WIDTH)
					board1[ny][nx] = 1;
			}
}
void mergePiece2() {
	for (int r=0;r<4;r++)
		for (int c=0;c<4;c++)
			if (current2.shape[r][c]) {
				int nx = current2.x + c;
				int ny = current2.y + r;
				if (ny>=0 && ny<HEIGHT && nx>=0 && nx<WIDTH)
					board2[ny][nx] = 1;
			}
}

int clearLines1() {
	int cleared = 0;
	for (int r=HEIGHT-1;r>=0;r--) {
		int full=1;
		for (int c=0;c<WIDTH;c++)
			if (!board1[r][c]) full=0;
		
		if (full) {
			cleared++;
			for (int rr=r;rr>0;rr--)
				for (int cc=0;cc<WIDTH;cc++)
					board1[rr][cc]=board1[rr-1][cc];
			for (int cc=0;cc<WIDTH;cc++) board1[0][cc]=0;
			r++;
		}
	}
	if (cleared>0) {
		int add = 0;
		if (cleared == 1) add = 100;
		else if (cleared == 2) add = 300;
		else if (cleared == 3) add = 500;
		else if (cleared >= 4) add = 800;
		score1 += add * level;
		lines1 += cleared;
	}
	return cleared;
}

int clearLines2() {
	int cleared = 0;
	for (int r=HEIGHT-1;r>=0;r--) {
		int full=1;
		for (int c=0;c<WIDTH;c++)
			if (!board2[r][c]) full=0;
		
		if (full) {
			cleared++;
			for (int rr=r;rr>0;rr--)
				for (int cc=0;cc<WIDTH;cc++)
					board2[rr][cc]=board2[rr-1][cc];
			for (int cc=0;cc<WIDTH;cc++) board2[0][cc]=0;
			r++;
		}
	}
	if (cleared>0) {
		int add = 0;
		if (cleared == 1) add = 100;
		else if (cleared == 2) add = 300;
		else if (cleared == 3) add = 500;
		else if (cleared >= 4) add = 800;
		score2 += add * level;
		lines2 += cleared;
	}
	return cleared;
}

int moveDown1() {
	if (!collision1(current1.x, current1.y+1, current1.shape)) {
		current1.y++;
		return 1;
	} else {
		mergePiece1();
		clearLines1();
		newPiece1();
		return 0;
	}
}
int moveDown2() {
	if (!collision2(current2.x, current2.y+1, current2.shape)) {
		current2.y++;
		return 1;
	} else {
		mergePiece2();
		clearLines2();
		newPiece2();
		return 0;
	}
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

int selectMode() {
    int choice;
    scanf("%d", &choice);
	while (getchar() != '\n');
    if (choice == 1 || choice == 2) {
        return choice;
    }
    printf("Invalid choice! Defaulting to Single-Player Mode.\n");
    Sleep(1500);
    return 1;
}

int selectDifficulty() {
	int choice;
	scanf("%d", &choice);
	while (getchar() != '\n');
	if (choice >= 1 && choice <= 3) {
		return choice;
	}
	printf("Invalid choice! Defaulting to Medium Difficulty.\n");
	Sleep(1500);
	return 2;
}

int main() {
	srand(time(NULL));
	clearScreen();
	printf("\n\n");
    printf("         ====================\n");
    printf("                TETRIS\n");
    printf("         ====================\n\n");
    printf("         Select Game Mode:\n\n");
    printf("         1. Single Player\n");
    printf("         2. Two Players\n\n");
    printf("         Enter your choice (1 or 2): ");
    gameMode = selectMode();

    printf("\n         Select Difficulty:\n\n");
    printf("         1. Easy\n");
    printf("         2. Medium\n");
    printf("         3. Hard\n\n");
    printf("         Enter your choice (1 to 3): ");
    difficulty = selectDifficulty();

	setHighScoreFiles(difficulty);
    setupDifficulty(difficulty);
    highScore1 = loadHighScore(highScoreFile1);
	highScore2 = loadHighScore(highScoreFile2);

	if (gameMode == 1) {
        gameOver2 = 1; 
    }
	fflush(stdout);
	clearScreen();
	hidecursor();
	initNextPieces();
	newPiece1();
	newPiece2();

	DWORD lastTick = GetTickCount();
	DWORD lastSpeedTick = lastTick;

	while (gameMode==1 && gameOver1==0 || gameMode==2 && gameOver1==0 && gameOver2==0) {
		
		drawBoard();

		if (_kbhit()) {
            int ch = _getch();
			if (ch=='q')
				break;
			if (ch=='p') {
				paused = !paused;
			}
			if (!paused) {
				if(ch=='a' && !collision1(current1.x-1, current1.y, current1.shape))
					current1.x--;
				else if(ch=='d' && !collision1(current1.x+1, current1.y, current1.shape))
					current1.x++;
				else if(ch=='s') {
					if (moveDown1()) score1 += 1;
				}
				else if(ch=='w') {
					int tmp[4][4];
					copyShape(tmp, current1.shape);
					rotate(tmp);
					if(!collision1(current1.x, current1.y, tmp))
						copyShape(current1.shape, tmp);
				}
				else if(ch=='z') {
					int dropped = 0;
					while(!collision1(current1.x, current1.y+1, current1.shape)) {
						current1.y++;
						dropped++;
					}
					if (dropped > 0) score1 += dropped * 2;
				}
				else if(gameMode==2 && ch==' ') {
					int dropped = 0;
					while(!collision2(current2.x, current2.y+1, current2.shape)) {
						current2.y++;
						dropped++;
					}
					if (dropped > 0) score2 += dropped * 2;
				}

				else if(gameMode==2 && (ch==0 || ch==224)) {
					int arrow = _getch();
					if(arrow==75 && !collision2(current2.x-1, current2.y, current2.shape))
						current2.x--;
					else if(arrow==77 && !collision2(current2.x+1, current2.y, current2.shape))
						current2.x++;
					else if(arrow==80) {
						if (moveDown2()) score2 += 1;
					}
					else if(arrow==72) {
						int tmp[4][4];
						copyShape(tmp, current2.shape);
						rotate(tmp);
						if(!collision2(current2.x, current2.y, tmp))
							copyShape(current2.shape, tmp);
					}
				}
			}
        }
		DWORD now = GetTickCount();
		if (!paused && now - lastTick > (DWORD)speed) {
			moveDown1();
			if (gameMode == 2)
				moveDown2();
			lastTick = now;
		}
		if (!paused && now - lastSpeedTick > (DWORD)speedInterval) {
			if (speed > minSpeed) {
				speed -= speedStep;
				if (speed < minSpeed) speed = minSpeed;
				level++;
			}
			lastSpeedTick = now;
		}
		if (paused) {
			lastTick = now;
			lastSpeedTick = now;
		}
		Sleep(20);
	}
	gotoxy(0, HEIGHT+7);
	if (gameMode == 1) {
        printf("Game Over!\n");
        printf("Final Score: %d\n", score1);
        printf("Lines Cleared: %d\n", lines1);
    } else {
	printf("Game Over!\n");
	if (gameOver1)
		printf("Player 2 Wins!\n");
	else if (gameOver2)
		printf("Player 1 Wins!\n");
	else
		printf("It's a Draw!\n");

	printf("Final Score - Player 1: %d | Player 2: %d\n", score1, score2);
	printf("Lines Cleared - Player 1: %d | Player 2: %d\n", lines1, lines2);
	}

	if (score1 > highScore1) {
		highScore1 = score1;
		saveHighScore(highScoreFile1, highScore1);
		printf("New High Score (P1): %d\n", highScore1);
	} else {
		printf("High Score (P1): %d\n", highScore1);
	}
	if (gameMode == 2) {
		if (score2 > highScore2) {
			highScore2 = score2;
			saveHighScore(highScoreFile2, highScore2);
			printf("New High Score (P2): %d\n", highScore2);
		} else {
			printf("High Score (P2): %d\n", highScore2);
		}
	}
	return 0;
}	