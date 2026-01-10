#include<stdio.h>
#include<stdlib.h>
#include<windows.h>
#include<conio.h>
#include<time.h>

#define WIDTH 10
#define HEIGHT 20

int board[HEIGHT][WIDTH] = {0};

typedef struct{
	int x, y;
	int type;
	int shape[4][4];
} Piece;

Piece current;
int score = 0;
int gameOver = 0;


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

int collision(int x, int y, int shape[4][4]) {
	for (int r=0;r<4;r++)
		for (int c=0;c<4;c++)
			if (shape[r][c]) {
				int nx = x + c;
				int ny = y + r;
				if (nx < 0 || nx >= WIDTH || ny >= HEIGHT) return 1;
				if (ny >= 0 && board[ny][nx]) return 1;
			}
	return 0;
}

void newPiece() {
	current.type = rand()%7;
	for (int r=0;r<4;r++)
		for (int c=0;c<4;c++)
			current.shape[r][c] = shapes[current.type][r][c];
	current.x = WIDTH/2 - 2;
	current.y = -2;
	if (collision(current.x, current.y, current.shape))
		gameOver = 1;
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
	for (int r=0;r<HEIGHT;r++) {
		printf("|");
		for (int c=0;c<WIDTH;c++) {
			int filled = board[r][c];
			int active = 0;
			int ar = r - current.y;
			int ac = c - current.x;
			if (ar>=0 && ar<4 && ac>=0 && ac<4 && current.shape[ar][ac])
				active=1;
			printf(filled || active ? "[]" : "  ");
		}
		printf("|\n");
	}
	printf("+--------------------+\n");
	printf("Score: %d\n", score);
}

void mergePiece() {
	for (int r=0;r<4;r++)
		for (int c=0;c<4;c++)
			if (current.shape[r][c]) {
				int nx = current.x + c;
				int ny = current.y + r;
				if (ny>=0 && ny<HEIGHT && nx>=0 && nx<WIDTH)
					board[ny][nx] = 1;
			}
}

void clearLines() {
	int cleared = 0;
	for (int r=HEIGHT-1;r>=0;r--) {
		int full=1;
		for (int c=0;c<WIDTH;c++)
			if (!board[r][c]) full=0;
		
		if (full) {
			cleared++;
			for (int rr=r;rr>0;rr--)
				for (int cc=0;cc<WIDTH;cc++)
					board[rr][cc]=board[rr-1][cc];
			for (int cc=0;cc<WIDTH;cc++) board[0][cc]=0;
			r++;
		}
	}
	if (cleared>0) {
		score += cleared*100;
	}
}
void moveDown() {
	if (!collision(current.x, current.y+1, current.shape))
		current.y++;
	else {
		mergePiece();
		clearLines();
		newPiece();	
	}
}

int main() {
	srand(time(NULL));
	hidecursor();
	newPiece();

	int speed = 500; // 0.5 seconds
	DWORD lastTick = GetTickCount();

	while (gameOver==0) {
		
		drawBoard();

		if (_kbhit()) {
			char ch = _getch();
			if(ch=='a' && !collision(current.x-1, current.y, current.shape))
				current.x--;
			else if(ch=='d' && !collision(current.x+1, current.y, current.shape))
				current.x++;
			else if(ch=='s')
				moveDown();
			else if(ch=='w') {
				int tmp[4][4];
				copyShape(tmp, current.shape);
				rotate(tmp);
				if(!collision(current.x, current.y, tmp))
					copyShape(current.shape, tmp);
			}
			else if(ch==' ') {
				while(!collision(current.x, current.y+1, current.shape))
					current.y++;
				;
			}
			else if(ch=='q') 
				break;
		}
		DWORD now = GetTickCount();
		if (now - lastTick > speed) {
			moveDown();
			lastTick = now;
		}
		Sleep(300);
	}
	gotoxy(0, HEIGHT+3);
	printf("Game Over! Final Score: %d\n", score);
	return 0;
}	