#include "TetriC.h"

PlayerState players[MAX_PLAYERS] = {0};
int totalPlayers = 1;
int localPlayerId = 1;
int winnerPlayerId = 0;

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

int networkMode = 0;
int netRole = 0;
SOCKET netSock = INVALID_SOCKET;
SOCKET listenSock = INVALID_SOCKET;
SOCKET clientSocks[MAX_PLAYERS] = {
	INVALID_SOCKET,
	INVALID_SOCKET,
	INVALID_SOCKET,
	INVALID_SOCKET,
	INVALID_SOCKET,
	INVALID_SOCKET
};
unsigned int sessionSeed = 0;

char highScoreFile1[64] = "";
char highScoreFile2[64] = "";

const char* pieceNames[7] = {"I", "O", "T", "S", "Z", "J", "L"};
const char* diffNames[3] = {"Easy", "Medium", "Hard"};

int shapes[7][4][4] = {
	{{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
	{{0,0,0,0}, {0,1,1,0}, {0,1,1,0}, {0,0,0,0}},
	{{0,0,0,0}, {1,1,1,0}, {0,1,0,0}, {0,0,0,0}},
	{{0,0,0,0}, {0,1,1,0}, {1,1,0,0}, {0,0,0,0}},
	{{0,0,0,0}, {1,1,0,0}, {0,1,1,0}, {0,0,0,0}},
	{{0,0,0,0}, {1,1,1,0}, {0,0,1,0}, {0,0,0,0}},
	{{0,0,0,0}, {1,1,1,0}, {1,0,0,0}, {0,0,0,0}}
};
