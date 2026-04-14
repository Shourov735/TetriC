#include "TetriC.h"

static int validPlayerId(int playerId) {
	return playerId >= 1 && playerId <= MAX_PLAYERS;
}

void getHighScoreFileForPlayer(int diff, int playerId, char* file, size_t size) {
	const char* diffName = "medium";

	if (diff == 1) diffName = "easy";
	else if (diff == 3) diffName = "hard";

	snprintf(file, size, "highscore_%s_p%d.txt", diffName, playerId);
}

void setHighScoreFiles(int diff) {
	getHighScoreFileForPlayer(diff, 1, highScoreFile1, sizeof(highScoreFile1));
	getHighScoreFileForPlayer(diff, 2, highScoreFile2, sizeof(highScoreFile2));
}

int loadHighScore(const char* file) {
	FILE* f = fopen(file, "r");
	int score = 0;

	if (f != NULL) {
		if (fscanf(f, "%d", &score) != 1) score = 0;
		fclose(f);
	}
	return score;
}

void saveHighScore(const char* file, int score) {
	FILE* f = fopen(file, "w");
	if (f == NULL) return;
	fprintf(f, "%d", score);
	fclose(f);
}

void setupDifficulty(int diff) {
	if (diff == 1) {
		baseSpeed = 650;
		minSpeed = 200;
		speedStep = 20;
		speedInterval = 20000;
	} else if (diff == 3) {
		baseSpeed = 350;
		minSpeed = 90;
		speedStep = 30;
		speedInterval = 12000;
	} else {
		baseSpeed = 500;
		minSpeed = 140;
		speedStep = 25;
		speedInterval = 15000;
	}
	speed = baseSpeed;
	level = 1;
}

void resetPlayerState(int playerId) {
	PlayerState* player;

	if (!validPlayerId(playerId)) return;
	player = &players[playerId - 1];
	memset(player->board, 0, sizeof(player->board));
	memset(&player->current, 0, sizeof(player->current));
	player->current.x = WIDTH / 2 - 2;
	player->current.y = -2;
	player->current.type = 0;
	player->score = 0;
	player->lines = 0;
	player->gameOver = 0;
	player->nextType = -1;
	player->connected = 1;
	player->active = 1;
}

void resetAllPlayers() {
	int i;

	for (i = 1; i <= MAX_PLAYERS; i++) {
		resetPlayerState(i);
		if (i > totalPlayers) {
			players[i - 1].active = 0;
			players[i - 1].connected = 0;
			players[i - 1].gameOver = 1;
		}
	}
	winnerPlayerId = 0;
	paused = 0;
}

void initNextPieces() {
	int i;

	for (i = 1; i <= totalPlayers; i++) {
		if (players[i - 1].active) {
			players[i - 1].nextType = rand() % 7;
		}
	}
}

static int readChoice(void) {
	int choice;

	if (scanf("%d", &choice) != 1) choice = 0;
	while (getchar() != '\n');
	return choice;
}

int selectPlayMode() {
	int choice = readChoice();

	if (choice == 2) return PLAY_NETWORK;
	if (choice != 1) {
		printf("Invalid choice! Defaulting to Local Mode.\n");
		Sleep(1500);
	}
	return PLAY_LOCAL;
}

int selectMode() {
	int choice = readChoice();

	if (choice == 1 || choice == 2) return choice;
	printf("Invalid choice! Defaulting to Single-Player Mode.\n");
	Sleep(1500);
	return 1;
}

int selectDifficulty() {
	int choice = readChoice();

	if (choice >= 1 && choice <= 3) return choice;
	printf("Invalid choice! Defaulting to Medium Difficulty.\n");
	Sleep(1500);
	return 2;
}
int selectNetworkGameMode() {
	int choice = readChoice();

	if (choice >= 1 && choice <= 3) return choice;
	printf("Invalid choice! Defaulting to Online Two-Player Mode.\n");
	Sleep(1500);
	return 2;
}

int selectNetworkRole() {
	int choice = readChoice();

	if (choice == ROLE_SERVER || choice == ROLE_CLIENT) return choice;
	printf("Invalid choice! Defaulting to Server.\n");
	Sleep(1500);
	return ROLE_SERVER;
}

int selectPlayerCount() {
	int choice = readChoice();

	if (choice >= 3 && choice <= MAX_PLAYERS) return choice;
	printf("Invalid choice! Defaulting to 3 players.\n");
	Sleep(1500);
	return 3;
}

int playerIsAlive(int playerId) {
	PlayerState* player;

	if (!validPlayerId(playerId)) return 0;
	player = &players[playerId - 1];
	return player->active && player->connected && !player->gameOver;
}

int countAlivePlayers() {
	int i;
	int alive = 0;

	for (i = 1; i <= totalPlayers; i++) {
		if (playerIsAlive(i)) alive++;
	}
	return alive;
}

int findWinningPlayer() {
	int i;
	int aliveCount = 0;
	int lastAlive = 0;
	int fallbackWinner = 0;
	int bestScore = -1;

	for (i = 1; i <= totalPlayers; i++) {
		PlayerState* player = &players[i - 1];

		if (!player->active) continue;
		if (player->score > bestScore) {
			bestScore = player->score;
			fallbackWinner = i;
		}
		if (player->connected && !player->gameOver) {
			aliveCount++;
			lastAlive = i;
		}
	}

	if (aliveCount > 1) return 0;
	if (aliveCount == 1) return lastAlive;
	return fallbackWinner;
}

void applyRemoteInput(char code) {
	if (code == 'p' || code == 'P') {
		paused = !paused;
	}
}

