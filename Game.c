#include "TetriC.h"

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
	} else if (diff == 3) {
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

int selectNetworkMode() {
	int choice;
	scanf("%d", &choice);
	while (getchar() != '\n');
	if (choice == 2) return 1;
	return 0;
}

int selectNetworkRole() {
	int choice;
	scanf("%d", &choice);
	while (getchar() != '\n');
	if (choice == 1 || choice == 2) return choice;
	printf("Invalid choice! Defaulting to Server.\n");
	Sleep(1500);
	return 1;
}