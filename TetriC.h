#ifndef TETRIS_H
#define TETRIS_H

#include<stdio.h>
#include<stdlib.h>
#include<winsock2.h>
#include<ws2tcpip.h>
#include<windows.h>
#include<conio.h>
#include<time.h>
#include<string.h>
#pragma comment(lib, "ws2_32.lib")

#define WIDTH 10
#define HEIGHT 20
#define SIDE_W 50
#define NET_PORT 27015
#define NET_BUF 64

typedef struct{
	int x, y;
	int type;
	int shape[4][4];
} Piece;

extern int board1[HEIGHT][WIDTH];
extern int board2[HEIGHT][WIDTH];

extern Piece current1;
extern Piece current2;
extern int score1;
extern int score2;
extern int lines1;
extern int lines2;
extern int gameOver1;
extern int gameOver2;
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

extern int nextType1;
extern int nextType2;

extern int networkMode;
extern int netRole;
extern SOCKET netSock;
extern SOCKET listenSock;

extern char highScoreFile1[64];
extern char highScoreFile2[64];

extern const char* pieceNames[7];
extern const char* diffNames[3];
extern int shapes[7][4][4];

/* Game/Config */
void setHighScoreFiles(int diff);
int loadHighScore(const char* file);
void saveHighScore(const char* file, int score);
void setupDifficulty(int diff);
void initNextPieces();
int selectMode();
int selectDifficulty();
int selectNetworkMode();
int selectNetworkRole();

/* Board/Piece */
int collision1(int x, int y, int shape[4][4]);
int collision2(int x, int y, int shape[4][4]);
void newPiece1();
void newPiece2();
void copyShape(int dst[4][4], int src[4][4]);
void rotate(int mat[4][4]);
void mergePiece1();
void mergePiece2();
int clearLines1();
int clearLines2();
int moveDown1();
int moveDown2();
void hardDrop1();
void hardDrop2();

/* UI */
void gotoxy(int x, int y);
void hidecursor();
void clearScreen();
void clearSide(char side[HEIGHT][SIDE_W+1]);
void setSideLine(char side[HEIGHT][SIDE_W+1], int row, const char* text);
void buildPreviewRow(int type, int row, char* rowLine);
void setPreviewBox(char side[HEIGHT][SIDE_W+1], int startRow, int type);
void drawBoard();

#endif
