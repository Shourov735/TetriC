#include "TetriC.h"

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
	if (nextType1 < 0) nextType1 = rand()%7;
	current1.type = nextType1;
	nextType1 = rand()%7;
	for (int r=0;r<4;r++)
		for (int c=0;c<4;c++)
			current1.shape[r][c] = shapes[current1.type][r][c];
	current1.x = WIDTH/2 - 2;
	current1.y = -2;
	if (collision1(current1.x, current1.y, current1.shape))
		gameOver1 = 1;
}

void newPiece2() {
	if (nextType2 < 0) nextType2 = rand()%7;
	current2.type = nextType2;
	nextType2 = rand()%7;
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

int clearLines1() {
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
		int add = 0;
		if (cleared == 1) add = 100;
		else if (cleared == 2) add = 300;
		else if (cleared == 3) add = 500;
		else if (cleared >= 4) add = 800;
		score1 += add * level;
		lines1 += cleared;
	}
	return cleared;
}

int clearLines2() {
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
		int add = 0;
		if (cleared == 1) add = 100;
		else if (cleared == 2) add = 300;
		else if (cleared == 3) add = 500;
		else if (cleared >= 4) add = 800;
		score2 += add * level;
		lines2 += cleared;
	}
	return cleared;
}

int moveDown1() {
	if (!collision1(current1.x, current1.y+1, current1.shape)) {
		current1.y++;
		return 1;
	} else {
		mergePiece1();
		clearLines1();
		newPiece1();
		return 0;
	}
}

int moveDown2() {
	if (!collision2(current2.x, current2.y+1, current2.shape)) {
		current2.y++;
		return 1;
	} else {
		mergePiece2();
		clearLines2();
		newPiece2();
		return 0;
	}
}

void hardDrop1() {
	int dropped = 0;
	while(!collision1(current1.x, current1.y+1, current1.shape)) {
		current1.y++;
		dropped++;
	}
	if (dropped > 0) score1 += dropped * 2;
	moveDown1();
}

void hardDrop2() {
	int dropped = 0;
	while(!collision2(current2.x, current2.y+1, current2.shape)) {
		current2.y++;
		dropped++;
	}
	if (dropped > 0) score2 += dropped * 2;
	moveDown2();
}
