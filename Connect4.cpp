// Connect4.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <bitset>

const unsigned long long BOTTOM_ROW = 4432676798593;

int evaluatePosition(unsigned long long position, unsigned long long mask, short numMoves, short alpha, short beta);
void printBitBoard(unsigned long long position);
bool findGameState(unsigned long long position, unsigned long long mask);
void testPositions(std::string fileName);
unsigned long long nodeCount = 0;
unsigned long long totalTime = 0;
unsigned long long totalNodeCount = 0;
bool checkAnswers = true;

/* Future optimizations:
* Better lookahead for losing moves
* Transposition table using Zobrist hashing. See if checking for and avoiding horizontally symmetric positions gives improved performance
* Iterative deepening to search more shallowly and narrow the search window
* Improved move ordering using heat map. Might be too computationally expensive if curent movecount is high
* Improved move ordering to proritize moves that could lead to 4 in a row
* Parallelize. My computer has 12 threads and 6 cores
* Opening book. Perhaps this is "cheating" but could store the first 6 or so optimal moves 
*/

int main()
{
	auto started = std::chrono::high_resolution_clock::now();
	testPositions("Test_L2_R1.txt");
	auto step1 = std::chrono::high_resolution_clock::now();
	std::cout << "Searched " << totalNodeCount << " nodes in: " << totalTime / 1000.0 << " seconds" << std::endl;
	std::cout << "Elapsed time: " << std::chrono::duration_cast<std::chrono::milliseconds>(step1 - started).count() / 1000.0 << " seconds" << std::endl;
	
}


int evaluatePosition(unsigned long long position, unsigned long long mask, short numMoves, short alpha, short beta) {

	nodeCount++; //count nodes searched
	bool p1_turn = numMoves % 2 == 0 ? true : false;
	unsigned short cols_to_run = 0;
	for (int i = 0; i < 7; i++) { //try every possible column
		unsigned long long column = (mask >> (i * 7)) & 63ULL; // Get value of the mask of the column in question. For example: 
															   // A column with 3 pieces in it and an empty 4th row would be "0000111". 0111111 = 63 = full
		if (column != 63) { //check column has room for a piece
			unsigned long long addition = (column + 1) << (i * 7); // create number equivalent to the next piece. For example "0000111" would give "0001000", which added or ORed with the mask will add a piece to it
			unsigned long long position2 = position;
			unsigned long long mask2 = mask;
			mask2 += addition; // add piece to mask
			if (p1_turn) {
				position2 += addition; // add piece to position, if it is p1's turn
			}
			if (!p1_turn) { //
				position2 ^= mask2; // XOR position if it is p2's turn, since the position input is always from p1's point of view
			}

			bool gameState = findGameState(position2, mask2); // if adding a piece would yield a win, return the heuristic here so that the other columns are not checked
			if (gameState) {
				return 22 - (numMoves + 2) / 2;
			}
			cols_to_run |= (1 << i); //note if a column should be checked
		}
	}

	int min = -22 + (numMoves + 2) / 2;	// lower bound of score since opponent cannot win next move
	if (alpha < min) {
		alpha = min; // update alpha to at least this min
		if (alpha >= beta) return alpha;  // extra pruning since we don't need to do better
	}
	int result = -22;
	for (int k = 0; k < 7; k++) { //try every possible column
		int i = k % 2 == 0 ? (3 + (k + 1) / 2) : (3 - (k + 1) / 2); //search center columns first

		if ((cols_to_run >> i) & 1UL) { // if a column should be checked (determined by lookahead code above), check it, ignoring the others
			unsigned long long column = (mask >> (i * 7)) & 63ULL; //get mask of a column (see above comments)
			if (column != 63) { //check column has room
				unsigned long long addition = (column + 1) << (i * 7); //get value for adding a piece
				unsigned long long position2 = position;
				unsigned long long mask2 = mask;
				mask2 += addition; //add piece
				if (p1_turn) {
					position2 += addition; //get p2 board
				}

				int n2 = numMoves + 1;
				int val = -evaluatePosition(position2, mask2, n2, -beta, -alpha); // recursive negamax recursive call

				if ((mask2 & 141845657554976ULL) == 141845657554976ULL) return 0; // if the top row is full, return a tie immediately 
				if (!p1_turn) {
					position2 ^= mask2; // XOR position if it is p2's turn, since the position input is always from p1's point of view
				}

				bool gameState = findGameState(position2, mask2); // test for a win
				if (gameState) { //if the player that just placed a piece wins, gameState will be true and we return the heuristic
					return 22 - (n2 + 1) / 2;
				}
				else {
					if (val >= beta) { // prune paths where val >= beta
						return beta;
					}
					if (val > alpha) { // increase alpha if there is a better move found
						alpha = val;
					}
				}
			}
		}
	}
	return alpha; //note that alpha and beta switch every turn, so alpha is always returned because that is always the best move of the maximizing player

}


void printBitBoard(unsigned long long position) { // prints the bitboard for debugging 
	for (int j = 6; j >= 0; j--) {
		for (int i = 0; i < 7; i++) {
			std::cout << (position >> ((i * 7 + j)) & 1ULL);
		}
		std::cout << std::endl;
	}
}

void testPositions(std::string fileName) { //this should be improved but since this is only generating positions it is not a priority
	int i = 1;
	std::string line;
	std::ifstream myfile(fileName); //ex: "Test_L1_R1.txt"
	if (myfile.is_open())
	{
		while (getline(myfile, line))
		{
			int pos = line.find(" ");
			std::string movesPlayed = line.substr(0, pos); // get position from test file
			unsigned long long position = 0;
			unsigned long long mask = 0;

			for (int q = 0; q < movesPlayed.size(); q++) {
				int i = (int)movesPlayed[q] - 48 - 1; // ascii int to actual int

				int y = 5;

				for (int j = 5; j >= 0; j--) {
					if (((mask >> (j + 7 * i)) & 1ULL) == 0) { // bad way to get the first available open space in a column
						y = j;
					}
					else {
						break;
					}
				}
				mask |= 1ULL << (y + 7 * i); // add piece to mask
				position |= ((q - 1) % 2 & 1ULL) << (y + 7 * i); // add piece to position, if p1 turn
			}
			nodeCount = 0;
			auto started = std::chrono::high_resolution_clock::now();
			int result = evaluatePosition(position, mask, (short)(movesPlayed.length()), -22, 22); //recursive call to determine position score
			auto step1 = std::chrono::high_resolution_clock::now();
			totalTime += std::chrono::duration_cast<std::chrono::milliseconds>(step1 - started).count(); // only count evaluatePosition() time
			totalNodeCount += nodeCount;
			if (checkAnswers) {
				std::cout << result << std::endl;
			}
			else {
				std::cout << "Case " << std::setw(4) << i << "   NodeCount: " << std::setw(8) << nodeCount << "   Time: " << std::setw(12) << std::chrono::duration_cast<std::chrono::microseconds>(step1 - started).count() << "us    result: " << result << std::endl;
			}
			i++;
		}
		myfile.close();
	}
}

bool findGameState(unsigned long long position, unsigned long long mask) {
	// Really cool way to determine if a position has a win. The position will always come in with the winning player's pieces being "1"s in the bitboard. 
	// The vertical check is the easiest and is explained below, the rest follow the same logic
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
	//The bitboard is vertically indexed, meaning the index increases by 1 as you move up a column and by 7 as you move to the right between columns
	m = position & (position >> 1); // m is a copy of the original board, except every 1 that does not have a 1 in the square directly above it is renoved
	// This removes any single 1s without any vertical neighbors, turns all two consective 1s into one 1, all three consecutive 1s into two, etc.
	// Now, 4 in a row in m is represented by three vertical neighboring 1s. We could repeat the process twice more to get the solution.
	if (m & (m >> 2)) {
	// But, we know that every 1 in m originally had a 1 directly above it. So, there is no way to get a gap of just one square between two 1s (try it!) 
	// Therefore, a 3 in a row in m (which was originally 4 in a row in position) only needs to be checked for the 1st and 3rd value. The middle value must be a 1.
		return true;
	}
	// The other checks work in the same way, except instead of checking vertically, the check is done horiztontally and on both diagonals

	return false; // if none of the checks are true, there is no win so return false
}