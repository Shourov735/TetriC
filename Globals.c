#include "TetriC.h"

int board1[HEIGHT][WIDTH] = {0};
int board2[HEIGHT][WIDTH] = {0};

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

int networkMode = 0;
int netRole = 0;
SOCKET netSock = INVALID_SOCKET;
SOCKET listenSock = INVALID_SOCKET;

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
