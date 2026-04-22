#ifndef TETRIS_H
#define TETRIS_H

#ifndef WINVER
#define WINVER 0x0600
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <conio.h>
#include <time.h>
#include <string.h>

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif

#define WIDTH 10
#define HEIGHT 20
#define SIDE_W 50
#define NET_PORT 1609
#define NET_BUF 64
#define MAX_PLAYERS 6

#define PLAY_LOCAL 0
#define PLAY_NETWORK 1

#define ROLE_SERVER 1
#define ROLE_CLIENT 2

typedef struct {
	int x, y;
	int type;
	int shape[4][4];
} Piece;

typedef struct {
	int board[HEIGHT][WIDTH];
	Piece current;
	int score;
	int lines;
	int gameOver;
	int nextType;
	int connected;
	int active;
} PlayerState;

extern PlayerState players[MAX_PLAYERS];
extern int totalPlayers;
extern int localPlayerId;
extern int winnerPlayerId;

#define board1 (players[0].board)
#define board2 (players[1].board)
#define current1 (players[0].current)
#define current2 (players[1].current)
#define score1 (players[0].score)
#define score2 (players[1].score)
#define lines1 (players[0].lines)
#define lines2 (players[1].lines)
#define gameOver1 (players[0].gameOver)
#define gameOver2 (players[1].gameOver)
#define nextType1 (players[0].nextType)
#define nextType2 (players[1].nextType)

extern int gameMode;
extern int difficulty;
extern int level;
extern int highScore1;
extern int highScore2;
extern int paused;

extern int speed;
extern int baseSpeed;
extern int minSpeed;
extern int speedStep;
extern int speedInterval;

extern int networkMode;
extern int netRole;
extern SOCKET netSock;
extern SOCKET listenSock;
extern SOCKET clientSocks[MAX_PLAYERS];
extern unsigned int sessionSeed;

extern char highScoreFile1[64];
extern char highScoreFile2[64];

extern const char* pieceNames[7];
extern const char* diffNames[3];
extern int shapes[7][4][4];

/* Game/Config */
void getHighScoreFileForPlayer(int diff, int playerId, char* file, size_t size);
void setHighScoreFiles(int diff);
int loadHighScore(const char* file);
void saveHighScore(const char* file, int score);
void setupDifficulty(int diff);
void resetPlayerState(int playerId);
void resetAllPlayers();
void initNextPieces();
int selectPlayMode();
int selectMode();
int selectDifficulty();
int selectNetworkGameMode();
int selectNetworkRole();
int selectPlayerCount();
int playerIsAlive(int playerId);
int countAlivePlayers();
int findWinningPlayer();
int networkSessionActive();
int networkRemoteCount();
int localControlsNetworkBoard();
void applyRemoteInput(char code);

/* Network */
int netInit();
void netClose();
void netCleanup();
int netStartServer(int expectedPlayers);
int netStartClient(const char* ip);
int netHandshake();
unsigned int netSyncSeed();
int netSetNonBlocking();
int netSendInput(char code);
int netSendState(int playerId);
int netBroadcastWinner(int winnerId);
void netPollInputs();

/* Board/Piece */
PlayerState* getPlayerState(int playerId);
int (*getPlayerBoard(int playerId))[WIDTH];
Piece* getPlayerPiece(int playerId);
int collisionPlayer(int playerId, int x, int y, int shape[4][4]);
void newPieceForPlayer(int playerId);
void copyShape(int dst[4][4], int src[4][4]);
void rotate(int mat[4][4]);
void mergePieceForPlayer(int playerId);
int clearLinesForPlayer(int playerId);
int moveDownForPlayer(int playerId);
void hardDropForPlayer(int playerId);

/* UI */
void gotoxy(int x, int y);
void hidecursor();
void clearScreen();
void clearSide(char side[HEIGHT][SIDE_W + 1]);
void setSideLine(char side[HEIGHT][SIDE_W + 1], int row, const char* text);
void buildPreviewRow(int type, int row, char* rowLine);
void setPreviewBox(char side[HEIGHT][SIDE_W + 1], int startRow, int type);
void drawBoard();

#endif
