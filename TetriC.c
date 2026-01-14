#include<stdio.h>
#include<stdlib.h>
#include<windows.h>
#include<conio.h>
#include<time.h>

#define WIDTH 10
#define HEIGHT 20

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
int gameOver1 = 0;
int gameOver2 = 0;
int gameMode = 1;

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
	current1.type = rand()%7;
	for (int r=0;r<4;r++)
		for (int c=0;c<4;c++)
			current1.shape[r][c] = shapes[current1.type][r][c];
	current1.x = WIDTH/2 - 2;
	current1.y = -2;
	if (collision1(current1.x, current1.y, current1.shape))
		gameOver1 = 1;
}

void newPiece2() {
	current2.type = rand()%7;
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

void drawBoard() {
    gotoxy(0,0);
	if (gameMode == 1) {
        printf("Player 1                         \n");
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
            printf("|\n");
        }
        printf("+--------------------+\n");
        printf("Score: %d\n", score1);
    } else {
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
        printf("|\n");
    }
    printf("+--------------------+  +--------------------+\n");
    printf("Score: %d              Score: %d\n", score1, score2);
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

void clearLines1() {
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
		score1 += cleared*100;
	}
}

void clearLines2() {
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
		score2 += cleared*100;
	}
}

void moveDown1() {
	if (!collision1(current1.x, current1.y+1, current1.shape))
		current1.y++;
	else {
		mergePiece1();
		clearLines1();
		newPiece1();	
	}
}
void moveDown2() {
	if (!collision2(current2.x, current2.y+1, current2.shape))
		current2.y++;
	else {
		mergePiece2();
		clearLines2();
		newPiece2();
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

int main() {
	srand(time(NULL));
	clearScreen();
	printf("\n\n");
    printf("         ===== TETRIS GAME =====\n\n");
    printf("         Select Game Mode:\n\n");
    printf("         1. Single Player\n");
    printf("         2. Two Players\n\n");
    printf("         Enter your choice (1 or 2): ");
    gameMode = selectMode();
	if (gameMode == 1) {
        gameOver2 = 1; 
    }
	fflush(stdout);
	clearScreen();
	hidecursor();
	newPiece1();
	newPiece2();

	int speed = 500; // 0.5 seconds
	DWORD lastTick = GetTickCount();

	while (gameMode==1 && gameOver1==0 || gameMode==2 && gameOver1==0 && gameOver2==0) {
		
		drawBoard();

		if (_kbhit()) {
            int ch = _getch();
            if(ch=='a' && !collision1(current1.x-1, current1.y, current1.shape))
                current1.x--;
            else if(ch=='d' && !collision1(current1.x+1, current1.y, current1.shape))
                current1.x++;
            else if(ch=='s')
                moveDown1();
            else if(ch=='w') {
                int tmp[4][4];
                copyShape(tmp, current1.shape);
                rotate(tmp);
                if(!collision1(current1.x, current1.y, tmp))
                    copyShape(current1.shape, tmp);
            }
			else if(ch=='z') {
				while(!collision1(current1.x, current1.y+1, current1.shape))
					current1.y++;
				;
			}
			else if(ch==' ') {
				while(!collision2(current2.x, current2.y+1, current2.shape))
					current2.y++;
				;
			}

            else if(gameMode==2 && (ch==0 || ch==224)) {
                int arrow = _getch();
                if(arrow==75 && !collision2(current2.x-1, current2.y, current2.shape))
                    current2.x--;
                else if(arrow==77 && !collision2(current2.x+1, current2.y, current2.shape))
                    current2.x++;
                else if(arrow==80)
                    moveDown2();
                else if(arrow==72) {
                    int tmp[4][4];
                    copyShape(tmp, current2.shape);
                    rotate(tmp);
                    if(!collision2(current2.x, current2.y, tmp))
                        copyShape(current2.shape, tmp);
                }
            }

            else if(ch=='q') 
                break;
        }
		DWORD now = GetTickCount();
		if (now - lastTick > speed) {
			moveDown1();
			if (gameMode == 2)
				moveDown2();
			lastTick = now;
		}
		Sleep(300);
	}
	gotoxy(0, HEIGHT+5);
	if (gameMode == 1) {
        printf("Game Over!\n");
        printf("Final Score: %d\n", score1);
    } else {
	printf("Game Over!\n");
	if (gameOver1)
		printf("Player 2 Wins!\n");
	else if (gameOver2)
		printf("Player 1 Wins!\n");
	else
		printf("It's a Draw!\n");

	printf("Final Score - Player 1: %d | Player 2: %d\n", score1, score2);
	}
	return 0;
}	