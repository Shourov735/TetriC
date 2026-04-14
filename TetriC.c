#include "TetriC.h"

static void applyActionToPlayer(int playerId, char action) {
	PlayerState* player = getPlayerState(playerId);

	if (player == NULL || !player->active || !player->connected || player->gameOver) return;
	if (action == 'L') {
		if (!collisionPlayer(playerId, player->current.x - 1, player->current.y, player->current.shape)) player->current.x--;
	} else if (action == 'R') {
		if (!collisionPlayer(playerId, player->current.x + 1, player->current.y, player->current.shape)) player->current.x++;
	} else if (action == 'D') {
		if (moveDownForPlayer(playerId)) player->score += 1;
	} else if (action == 'T') {
		int tmp[4][4];
		copyShape(tmp, player->current.shape);
		rotate(tmp);
		if (!collisionPlayer(playerId, player->current.x, player->current.y, tmp)) copyShape(player->current.shape, tmp);
	} else if (action == 'H') {
		hardDropForPlayer(playerId);
	}
}

static char actionFromWasdKey(int ch) {
	if (ch == 'a' || ch == 'A') return 'L';
	if (ch == 'd' || ch == 'D') return 'R';
	if (ch == 's' || ch == 'S') return 'D';
	if (ch == 'w' || ch == 'W') return 'T';
	if (ch == 'z' || ch == 'Z') return 'H';
	return 0;
}

static char actionFromArrowKey(int key) {
	if (key == 75) return 'L';
	if (key == 77) return 'R';
	if (key == 80) return 'D';
	if (key == 72) return 'T';
	return 0;
}

static void initialisePlayersForGame(void) {
	int i;

	resetAllPlayers();
	if (networkMode && totalPlayers > 1) {
		players[localPlayerId - 1].nextType = rand() % 7;
		newPieceForPlayer(localPlayerId);
		for (i = 1; i <= totalPlayers; i++) {
			if (i != localPlayerId) {
				players[i - 1].nextType = -1;
				memset(players[i - 1].board, 0, sizeof(players[i - 1].board));
				memset(&players[i - 1].current, 0, sizeof(players[i - 1].current));
			}
		}
	} else {
		initNextPieces();
		for (i = 1; i <= totalPlayers; i++) newPieceForPlayer(i);
	}
}

static int shouldContinue(void) {
	if (networkMode && totalPlayers > 1) return winnerPlayerId == 0;
	if (gameMode == 1) return !gameOver1;
	return !gameOver1 && !gameOver2;
}

static int resolvedWinnerId(void) {
	if (winnerPlayerId > 0) return winnerPlayerId;
	return findWinningPlayer();
}

static void finalizeNetworkResult(void) {
	DWORD startTick;
	int fallbackWinner;

	if (!(networkMode && totalPlayers > 1)) return;

	if (netRole == ROLE_SERVER) {
		if (winnerPlayerId > 0) {
			startTick = GetTickCount();
			while (GetTickCount() - startTick < 500) {
				netBroadcastWinner(winnerPlayerId);
				Sleep(50);
			}
		}
		return;
	}

	startTick = GetTickCount();
	while (winnerPlayerId <= 0 && GetTickCount() - startTick < 750) {
		netPollInputs();
		if (winnerPlayerId > 0) break;
		Sleep(20);
	}

	if (winnerPlayerId <= 0) {
		fallbackWinner = findWinningPlayer();
		if (fallbackWinner > 0) {
			winnerPlayerId = fallbackWinner;
		}
	}
}

static void printMatchSummary(void) {
	int i;

	if (networkMode && totalPlayers > 1) {
		int winner = resolvedWinnerId();
		if (winner > 0) printf("Winner: Player %d\n", winner);
		else printf("Match ended without a confirmed winner.\n");
		for (i = 1; i <= totalPlayers; i++) {
			PlayerState* player = getPlayerState(i);
			printf("Player %d - Score: %d | Lines: %d | Status: %s\n", i, player != NULL ? player->score : 0, player != NULL ? player->lines : 0, playerIsAlive(i) ? "Alive" : (player != NULL && !player->connected ? "Disconnected" : "Out"));
		}
		return;
	}

	if (gameMode == 1) {
		printf("Game Over!\n");
		printf("Final Score: %d\n", score1);
		printf("Lines Cleared: %d\n", lines1);
		return;
	}

	printf("Game Over!\n");
	if (gameOver1) printf("Player 2 Wins!\n");
	else if (gameOver2) printf("Player 1 Wins!\n");
	else printf("It's a Draw!\n");
	printf("Final Score - Player 1: %d | Player 2: %d\n", score1, score2);
	printf("Lines Cleared - Player 1: %d | Player 2: %d\n", lines1, lines2);
}

int main() {
	char localHighScoreFile[64];
	unsigned int seed;

	srand((unsigned int)time(NULL));
	clearScreen();
	printf("\n\n");
	printf("         ====================\n");
	printf("                TETRIS\n");
	printf("         ====================\n\n");
	printf("         Select Play Mode:\n\n");
	printf("         1. Local\n");
	printf("         2. Network\n\n");
	printf("         Enter your choice (1 or 2): ");
	networkMode = selectPlayMode();

	if (networkMode == PLAY_LOCAL) {
		printf("\n         Local Game Mode:\n\n");
		printf("         1. Single Player\n");
		printf("         2. Two Players\n\n");
		printf("         Enter your choice (1 or 2): ");
		gameMode = selectMode();
		totalPlayers = gameMode;
		localPlayerId = 1;
	} else {
		printf("\n         Network Game Mode:\n\n");
		printf("         1. Single\n");
		printf("         2. Two Player\n");
		printf("         3. Multiplayer\n\n");
		printf("         Enter your choice (1 to 3): ");
		gameMode = selectNetworkGameMode();
		if (gameMode == 1) {
			totalPlayers = 1;
			localPlayerId = 1;
			netRole = 0;
		} else {
			char ip[64];
			printf("\n         Online Role:\n\n");
			printf("         1. Server\n");
			printf("         2. Client\n\n");
			printf("         Enter your choice (1 or 2): ");
			netRole = selectNetworkRole();
			if (netRole == ROLE_SERVER) {
				if (gameMode == 3) {
					printf("\n         Total Players (3 to %d): ", MAX_PLAYERS);
					totalPlayers = selectPlayerCount();
				} else {
					totalPlayers = 2;
				}
				localPlayerId = 1;
			} else {
				totalPlayers = gameMode == 2 ? 2 : 3;
			}

			if (!netInit()) {
				printf("Network init failed.\n");
				return 1;
			}

			if (netRole == ROLE_SERVER) {
				printf("Waiting for %d remote player(s) on port %d...\n", totalPlayers - 1, NET_PORT);
				if (!netStartServer(totalPlayers)) {
					printf("Server setup failed.\n");
					netCleanup();
					return 1;
				}
			} else {
				printf("Enter server IP: ");
				scanf("%63s", ip);
				while (getchar() != '\n');
				if (!netStartClient(ip)) {
					printf("Client connect failed.\n");
					netCleanup();
					return 1;
				}
			}

			if (!netSetNonBlocking()) {
				printf("Failed to switch sockets to non-blocking mode.\n");
				netCleanup();
				return 1;
			}
			printf("Network ready. You are Player %d of %d.\n", localPlayerId, totalPlayers);
			Sleep(1000);
		}
	}

	printf("\n         Select Difficulty:\n\n");
	printf("         1. Easy\n");
	printf("         2. Medium\n");
	printf("         3. Hard\n\n");
	printf("         Enter your choice (1 to 3): ");
	difficulty = selectDifficulty();

	setHighScoreFiles(difficulty);
	setupDifficulty(difficulty);
	if (!networkMode && gameMode == 2) {
		highScore1 = loadHighScore(highScoreFile1);
		highScore2 = loadHighScore(highScoreFile2);
	} else {
		getHighScoreFileForPlayer(difficulty, localPlayerId, localHighScoreFile, sizeof(localHighScoreFile));
		highScore1 = loadHighScore(localHighScoreFile);
		highScore2 = 0;
	}

	seed = (networkMode && totalPlayers > 1) ? netSyncSeed() : (unsigned int)time(NULL);
	srand(seed);
	initialisePlayersForGame();
	fflush(stdout);
	clearScreen();
	hidecursor();

	if (networkMode && totalPlayers > 1) {
		if (netSendState(localPlayerId) < 0) {
			gotoxy(0, HEIGHT + 7);
			printf("Unable to send the first network state.\n");
			netCleanup();
			return 1;
		}
	}

	{
		DWORD lastTick = GetTickCount();
		DWORD lastSpeedTick = lastTick;

		while (shouldContinue()) {
			if (networkMode && totalPlayers > 1) netPollInputs();
			drawBoard();

			if (_kbhit()) {
				int ch = _getch();
				if (ch == 'q' || ch == 'Q') {
					if (networkMode && totalPlayers > 1 && netRole == ROLE_CLIENT) netSendInput('Q');
					break;
				}
				if (ch == 'p' || ch == 'P') {
					paused = !paused;
					if (networkMode && totalPlayers > 1) netSendInput('P');
				}
				if (!paused) {
					if (networkMode && totalPlayers > 1) {
						char action = 0;
						if (playerIsAlive(localPlayerId)) {
							if (ch == 0 || ch == 224) action = actionFromArrowKey(_getch());
							else if (ch == ' ') action = 'H';
							else action = actionFromWasdKey(ch);
							if (action) applyActionToPlayer(localPlayerId, action);
						}
					} else if (gameMode == 1) {
						char action = actionFromWasdKey(ch);
						if (action) applyActionToPlayer(1, action);
					} else {
						if (ch == 0 || ch == 224) {
							char action = actionFromArrowKey(_getch());
							if (action) applyActionToPlayer(2, action);
						} else if (ch == ' ') {
							applyActionToPlayer(2, 'H');
						} else {
							char action = actionFromWasdKey(ch);
							if (action) applyActionToPlayer(1, action);
						}
					}
				}
			}

			{
				DWORD now = GetTickCount();
				if (!paused && now - lastTick > (DWORD)speed) {
					if (networkMode && totalPlayers > 1) {
						if (playerIsAlive(localPlayerId)) moveDownForPlayer(localPlayerId);
					} else {
						if (playerIsAlive(1)) moveDownForPlayer(1);
						if (gameMode == 2 && playerIsAlive(2)) moveDownForPlayer(2);
					}
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
			}

			if (networkMode && totalPlayers > 1) {
				if (netRole == ROLE_SERVER && winnerPlayerId == 0) {
					int winner = findWinningPlayer();
					if (winner != 0) {
						winnerPlayerId = winner;
						netBroadcastWinner(winnerPlayerId);
					}
				}
				if (netSendState(localPlayerId) < 0) {
					if (netRole == ROLE_CLIENT) {
						int fallbackWinner = findWinningPlayer();
						winnerPlayerId = fallbackWinner > 0 ? fallbackWinner : -1;
					} else if (winnerPlayerId == 0) {
						winnerPlayerId = findWinningPlayer();
					}
				}
			}
			Sleep(20);
		}
	}

	finalizeNetworkResult();
	gotoxy(0, HEIGHT + 8);
	printMatchSummary();

	if (!networkMode && gameMode == 2) {
		if (score1 > highScore1) {
			highScore1 = score1;
			saveHighScore(highScoreFile1, highScore1);
			printf("New High Score (P1): %d\n", highScore1);
		} else {
			printf("High Score (P1): %d\n", highScore1);
		}
		if (score2 > highScore2) {
			highScore2 = score2;
			saveHighScore(highScoreFile2, highScore2);
			printf("New High Score (P2): %d\n", highScore2);
		} else {
			printf("High Score (P2): %d\n", highScore2);
		}
	} else {
		PlayerState* localPlayer = getPlayerState(localPlayerId);
		int localScore = localPlayer != NULL ? localPlayer->score : 0;
		if (localScore > highScore1) {
			highScore1 = localScore;
			saveHighScore(localHighScoreFile, highScore1);
			printf("New Local High Score: %d\n", highScore1);
		} else {
			printf("Local High Score: %d\n", highScore1);
		}
	}

	if (networkMode && totalPlayers > 1) netCleanup();
	return 0;
}



