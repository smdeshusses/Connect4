// Connect4.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <array>
#include <fstream>
#include <math.h>
#include <algorithm>
#include <chrono>

enum class PieceName { blank, p1, p2 };

void printBoard(std::array<PieceName, 42> board);
int findGameState(std::array<PieceName, 42> board);
int findGameState2(std::array<PieceName, 42> board, bool lastPlayer, int last_x, int last_y);
std::array<PieceName, 42> generateBoard(std::string position);
void testPositions(std::string fileName);
int evaluatePosition(std::array<PieceName, 42> board, int numMoves, int alpha, int beta);
std::array<PieceName, 42> board = { PieceName::blank };
void local_play(std::array<PieceName, 42> board);

int main()
{

	auto started = std::chrono::high_resolution_clock::now();
	std::array<PieceName, 42> board = { PieceName::blank };
	//local_play(board);
	testPositions("Test_L3_R1.txt");
	//testPositions("miniSet.txt");
	auto step1 = std::chrono::high_resolution_clock::now();
	std::cout << "Searched test set in: " << std::chrono::duration_cast<std::chrono::milliseconds>(step1 - started).count() / 1000.0 << " seconds" << std::endl;

}


int evaluatePosition(std::array<PieceName, 42> board, int numMoves, int alpha, int beta) { //need to implement minimax for optimal opponent gameplay in order to search correctly


	bool p1_turn = numMoves % 2 == 0 ? true : false;

	for (int i = 0; i < 7; i++) { //try every possible column
		int y = 6;
		for (int j = 5; j >= 0; j--) {//get next move
			if (board[7 * j + i] == PieceName::blank) {
				y = j;
			}
		}
		if (y < 6) {//if legal, try this move
			std::array<PieceName, 42> b2 = board;
			b2[7 * y + i] = p1_turn ? PieceName::p1 : PieceName::p2;
			int n2 = numMoves + 1;
			int gameState = findGameState2(b2, p1_turn, i, y);
			if (gameState == 1 || gameState == 2) {
				return 22 - (n2 + 1) / 2;
			}
		}
	}

	int result = -999;
	for (int k = 0; k < 7; k++) { //try every possible column
		int i = k % 2 == 0 ? (3 + (k + 1) / 2) : (3 - (k + 1) / 2);

		int y = 6;
		for (int j = 5; j >= 0; j--) {//get next move
			if (board[7 * j + i] == PieceName::blank) {
				y = j;
			}
		}
		if (y < 6) {//if legal, try this move
			std::array<PieceName, 42> b2 = board;
			b2[7 * y + i] = p1_turn ? PieceName::p1 : PieceName::p2;
			int n2 = numMoves + 1;
			int val = -evaluatePosition(b2, n2, -beta, -alpha);
			int gameState = findGameState2(b2, p1_turn, i, y);
			if (gameState == 0) {
				if (val >= beta) {
					return beta;
				}
				if (val > alpha) {
					alpha = val;
				}
			}
			else if (gameState == 1 || gameState == 2) {//p1/2 win
				return 22 - (n2 + 1) / 2;
			}
			else { //tie
				return 0;
			}

		}
	}
	return alpha;

}

void testPositions(std::string fileName) {
	std::string line;
	std::ifstream myfile(fileName); //ex: "Test_L1_R1.txt"
	if (myfile.is_open())
	{
		while (getline(myfile, line))
		{
			int pos = line.find(" ");
			std::string position = line.substr(0, pos);
			std::string score = line.substr(pos + 1, line.size());
			int result = evaluatePosition(generateBoard(position), position.length(), -42, 42);
			std::cout << position << " " << result << '\n';
		}
		myfile.close();
	}
}

std::array<PieceName, 42> generateBoard(std::string position) {
	std::array<PieceName, 42> board = { PieceName::blank };
	for (int q = 0; q < position.size(); q++) {
		int i = (int)position[q] - 48 - 1;
		int y = 5;

		for (int j = 5; j >= 0; j--) {
			if (board[j * 7 + i] == PieceName::blank) {
				y = j;
			}
			else {
				break;
			}
		}
		board[y * 7 + i] = (q % 2 == 0 ? PieceName::p1 : PieceName::p2);
	}
	return board;
}


void printBoard(std::array<PieceName, 42> board) {
	std::cout << "---------" << std::endl;
	for (int j = 5; j >= 0; j--) {
		std::cout << "|";
		for (int i = 0; i < 7; i++) {
			if (board[j * 7 + i] == PieceName::blank) {
				std::cout << " ";
			}
			else if (board[j * 7 + i] == PieceName::p1) {
				std::cout << "X";
			}
			else if (board[j * 7 + i] == PieceName::p2) {
				std::cout << "O";
			}
		}
		std::cout << "|" << std::endl;
	}

	std::cout << "---------" << std::endl;
	std::cout << " 1234567 " << std::endl;
}

int findGameState2(std::array<PieceName, 42> board, bool lastPlayer, int last_x, int last_y) {
	int gameState = 0;
	int count = 0;
	PieceName player = lastPlayer ? PieceName::p1 : PieceName::p2;

	//diagonal 1 /
	int i1 = last_x - 1;
	int j1 = last_y-1;
	int i2 = last_x + 1;
	int j2 = last_y+1;
	while (i1 >= 0 && j1>= 0) {
		if (board[i1 + 7 * j1] != player) {
			break;
		}
		i1--;
		j1--;
	}
	while (i2 <= 6 && j2<=5 ) {
		if (board[i2 + 7 * j2] != player) {
			break;
		}
		i2++;
		j2++;
	}
	if (i2 - i1 >= 5) {
		return lastPlayer ? 1 : 2;
	}

	//diagonal 2 \ //
	i1 = last_x - 1;
	j1 = last_y + 1;
	i2 = last_x + 1;
	j2 = last_y - 1;
	while (i1 >= 0 && j1 <= 5) {
		if (board[i1 + 7 * j1] != player) {
			break;
		}
		i1--;
		j1++;
	}
	while (i2 <= 6 && j2 >= 0) {
		if (board[i2 + 7 * j2] != player) {
			break;
		}
		i2++;
		j2--;
	}
	if (i2 - i1 >= 5) {
		return lastPlayer ? 1 : 2;
	}


	//horizontal
	i1 = last_x - 1;
	i2 = last_x + 1;
	while (i1 >= 0) {
		if (board[i1 + 7 * last_y] != player) {
			break;
		}
		i1--;
	}
	while (i2 <= 6) {
		if (board[i2 + 7 * last_y] != player) {
			break;
		}
		i2++;
	}
	if (i2 - i1 >= 5) {
		return lastPlayer ? 1 : 2;
	}

	//vertical
	j1 = last_y - 1;
	j2 = last_y + 1;
	while (j1 >= 0) {
		if (board[j1 * 7 + last_x] != player) {
			break;
		}
		j1--;
	}
	while (j2 <= 5) {
		if (board[j2 * 7 + last_x] != player) {
			break;
		}
		j2++;
	}
	if (j2 - j1 >= 5) {
		return lastPlayer ? 1 : 2;
	}

	if (last_y == 5) {
		bool tie = true;
		for (int i = 0; i < 7; i++) {
			if (board[35 + i] == PieceName::blank) {
				tie = false;
			}
		}
		if (tie) return 3;
	}

	return 0;
}

int findGameState(std::array<PieceName, 42> board) {
	int gameState = 0;

	//horizontal
	for (int i = 0; i < 7; i++) {
		PieceName lastSquare = PieceName::blank;
		int counter = 1;
		for (int j = 0; j < 6; j++) {
			if (board[j * 7 + i] == lastSquare && lastSquare != PieceName::blank) {
				counter++;
			}
			else {
				lastSquare = board[j * 7 + i];
				counter = 1;
			}
			if (counter == 4) {
				gameState = (lastSquare == PieceName::p1 ? 1 : 2);
				break;
			}
		}
	}
	
	//vertical
	for (int j = 0; j < 6; j++) {
		PieceName lastSquare = PieceName::blank;
		int counter = 1;
		for (int i = 0; i < 7; i++) {
			if (board[j * 7 + i] == lastSquare && lastSquare != PieceName::blank) {
				counter++;
			}
			else {
				lastSquare = board[j * 7 + i];
				counter = 1;
			}
			if (counter == 4) {
				gameState = (lastSquare == PieceName::p1 ? 1 : 2);
				break;
			}
		}
	}
	
	//diagonal 1 
	for (int a = 0; a < 6; a++) {
		PieceName lastSquare = PieceName::blank;
		int counter = 1;
		int x_coord = a <= 2 ? 0 : a - 2;
		int y_coord = a >= 2 ? 5 : a + 3;
		//std::cout << "( " << x_coord << ", " << y_coord << ")" << std::endl;
	
		while (x_coord <= 6 && y_coord >= 0) {
			if (board[y_coord * 7 + x_coord] == lastSquare && lastSquare != PieceName::blank) {
				counter++;
			}
			else {
				lastSquare = board[y_coord * 7 + x_coord];
				counter = 1;
			}
			if (counter == 4) {
				gameState = (lastSquare == PieceName::p1 ? 1 : 2);
				break;
			}
	
			x_coord++;
			y_coord--;
		}
	}
	//diagonal 2
	for (int a = 0; a < 6; a++) {
		PieceName lastSquare = PieceName::blank;
		int counter = 1;
		int x_coord = a <= 2 ? 0 : a - 2;
		int y_coord = a >= 2 ? 0 : 2 - a;

		while (x_coord <= 6 && y_coord <= 5) {
			if (board[y_coord * 7 + x_coord] == lastSquare && lastSquare != PieceName::blank) {
				counter++;
			}
			else {
				lastSquare = board[y_coord * 7 + x_coord];
				counter = 1;
			}
			if (counter == 4) {
				gameState = (lastSquare == PieceName::p1 ? 1 : 2);
				break;
			}
	
			x_coord++;
			y_coord++;
		}
	}
	
	if (gameState == 0) {
		bool fullBoard = true;
		for (int i = 0; i < 7; i++) {
			if (board[35 + i] == PieceName::blank) {
				fullBoard = false;
			}
		}
		if (fullBoard) {
			gameState = 3;
		}
	}

	return gameState;;
}


void local_play(std::array<PieceName, 42> board) {
	printBoard(board);
	bool turn = true;
	int i;
	while (true) {
		std::cin >> i;
		int y = 6;
		if (i >= 1 && i <= 7) {
			i--;
			for (int j = 5; j >= 0; j--) {
				if (board[j * 7 + i] == PieceName::blank) {
					y = j;
				}
			}
			if (y != 6) {
				board[y * 7 + i] = turn ? PieceName::p1 : PieceName::p2;
				int state = findGameState2(board, turn, i, y);
				if (state != 0) {
					printBoard(board);
					if (state == 3) {
						std::cout << "Drawn Game! " << std::endl;
					}
					else {
						std::cout << "Player " << state << " has won!" << std::endl;
					}
					break;
				}
				turn = !turn;
			}
			else {
				std::cout << "column full" << std::endl;
			}
		}
		else {
			std::cout << "Input column value from 1 to 7" << std::endl;
		}
		printBoard(board);
	}
}