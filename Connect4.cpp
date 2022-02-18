// Connect4.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <fstream>
#include <chrono>
#include <iomanip>

const unsigned long long BOTTOM_ROW = 4432676798593;

int evaluatePosition(unsigned long long position, unsigned long long mask, short numMoves, short alpha, short beta);
std::pair<unsigned long long, unsigned long long> generateBitBoard(std::string movesPlayed);
void printBitBoard(unsigned long long position);
bool findGameState3(unsigned long long position, unsigned long long mask);
void testPositions(std::string fileName);
unsigned long long nodeCount = 0;
unsigned long long totalTime = 0;
unsigned long long totalNodeCount = 0;

int main()
{
	auto started = std::chrono::high_resolution_clock::now();
	testPositions("Test_L3_R1.txt");
	auto step1 = std::chrono::high_resolution_clock::now();
	std::cout << "Searched " <<  totalNodeCount << " nodes in: " << totalTime/1000.0 << " seconds" << std::endl;
	std::cout << "Elapsed time: " << std::chrono::duration_cast<std::chrono::milliseconds>(step1 - started).count() / 1000.0 << " seconds" << std::endl;

}


int evaluatePosition(unsigned long long position, unsigned long long mask, short numMoves, short alpha, short beta) { //need to implement minimax for optimal opponent gameplay in order to search correctly

	nodeCount++;
	bool p1_turn = numMoves % 2 == 0 ? true : false;

	for (int i = 0; i < 7; i++) { //try every possible column
		unsigned long long column = (mask >> (i * 7)) & 63ULL;
		if (column != 63) { //check column has room
			unsigned long long addition = (column + 1) << (i * 7);
			unsigned long long position2 = position;
			unsigned long long mask2 = mask;
			mask2 += addition;
			if (p1_turn) {
				position2 += addition;
			}
			if (!p1_turn) {
				position2 ^= mask2;
			}

			bool gameState = findGameState3(position2, mask2);
			if (gameState) {
				return 22 - (numMoves + 2) / 2;
			}
		}
	}

	int result = -42;
	for (int k = 0; k < 7; k++) { //try every possible column
		int i = k % 2 == 0 ? (3 + (k + 1) / 2) : (3 - (k + 1) / 2);

		unsigned long long column = (mask >> (i * 7)) & 63ULL;
		if (column != 63) { //check column has room
			unsigned long long addition = (column + 1) << (i * 7);
			unsigned long long position2 = position;
			unsigned long long mask2 = mask;
			mask2 += addition;
			if (p1_turn) {
				position2 += addition;
			}

			int n2 = numMoves + 1;
			int val = -evaluatePosition(position2, mask2, n2, -beta, -alpha);

			if ((mask2 & 141845657554976ULL) == 141845657554976ULL) return 0;
			if (!p1_turn) {
				position2 ^= mask2;
			}

			bool gameState = findGameState3(position2, mask2);
			if (gameState) {
				return 22 - (n2 + 1) / 2;
			}
			else {
				if (val >= beta) {
					return beta;
				}
				if (val > alpha) {
					alpha = val;
				}
			}
		}
	}
	return alpha;

}

std::pair<unsigned long long, unsigned long long> generateBitBoard(std::string movesPlayed) { //this should be improved but since this is only generating positions it is not a priority

	unsigned long long position = 0;
	unsigned long long mask = 0;

	for (int q = 0; q < movesPlayed.size(); q++) {
		int i = (int)movesPlayed[q] - 48 - 1;

		int y = 5;

		for (int j = 5; j >= 0; j--) {
			if (((mask >> (j + 7 * i)) & 1ULL) == 0) {
				y = j;
			}
			else {
				break;
			}
		}
		mask |= 1ULL << (y + 7 * i);
		position |= ((q - 1) % 2 & 1ULL) << (y + 7 * i);
	}
	std::pair<unsigned long long, unsigned long long> p {position, mask };
	return p;
}



void printBitBoard(unsigned long long position) {
	for (int j = 6; j >= 0; j--) {
		for (int i = 0; i < 7; i++) {
			std::cout << (position >> ((i * 7 + j)) & 1ULL);
		}
		std::cout << std::endl;
	}
}

void testPositions(std::string fileName) {
	int i = 1;
	std::string line;
	std::ifstream myfile(fileName); //ex: "Test_L1_R1.txt"
	if (myfile.is_open())
	{
		while (getline(myfile, line))
		{
			int pos = line.find(" ");
			std::string movesPlayed = line.substr(0, pos);
			unsigned long long position = 0;
			unsigned long long mask = 0;

			for (int q = 0; q < movesPlayed.size(); q++) {
				int i = (int)movesPlayed[q] - 48 - 1;

				int y = 5;

				for (int j = 5; j >= 0; j--) {
					if (((mask >> (j + 7 * i)) & 1ULL) == 0) {
						y = j;
					}
					else {
						break;
					}
				}
				mask |= 1ULL << (y + 7 * i);
				position |= ((q - 1) % 2 & 1ULL) << (y + 7 * i);
			}
			nodeCount = 0;
			auto started = std::chrono::high_resolution_clock::now();
			int result = evaluatePosition(position, mask, (short)(movesPlayed.length()), -42, 42);
			auto step1 = std::chrono::high_resolution_clock::now();
			totalTime += std::chrono::duration_cast<std::chrono::milliseconds>(step1 - started).count();
			totalNodeCount += nodeCount;
			std::cout << "Case " << std::setw(4) << i << "   NodeCount: " << std::setw(8) << nodeCount << "   Time: " << std::setw(12) << std::chrono::duration_cast<std::chrono::microseconds>(step1 - started).count() << "us" << std::endl;
			i++;
		}
		myfile.close();
	}
}

bool findGameState3(unsigned long long position, unsigned long long mask) {

	unsigned long long m = 0;

	//diagonal /
	m = position & (position >> 8);
	if (m & (m >> 16)) {
		return true;
	}

	//diagonal \  
	m = position & (position >> 6);
	if (m & (m >> 12)) {
		return true;
	}

	//horizontal
	m = position & (position >> 7);
	if (m & (m >> 14)) {
		return true;
	}

	//vertical
	m = position & (position >> 1);
	if (m & (m >> 2)) {
		return true;
	}
	return false;
}