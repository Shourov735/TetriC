#include "TetriC.h"

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
					hardDrop1();
				}
				else if(gameMode==2 && ch==' ') {
					hardDrop2();
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
