// Connect4.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <bitset>
#include <unordered_set>

const uint64_t BOTTOM_ROW = 4432676798593;

int evaluatePosition(uint64_t position, uint64_t mask, short numMoves, short alpha, short beta);
void printBitBoard(uint64_t position);
bool findGameState(uint64_t position, uint64_t mask); //now unused!
void testPositions(std::string fileName);
void printBitBoard(uint64_t position, uint64_t mask);
int runPosition(uint64_t position, uint64_t mask, short numMoves);
uint64_t checkFutureLoss(uint64_t position, uint64_t mask);
uint64_t nodeCount = 0;
uint64_t totalTime = 0;
uint64_t totalNodeCount = 0;
bool checkAnswers = true;
unsigned const int table_size = 16777213; //8388593 //4015091 //1007957 //507937 //51449 //5023
uint64_t table[table_size] = { 0 };
unsigned int lookupBits[256]; //unused
std::unordered_set<uint64_t> bitCount; //unused



/* Future optimizations:
* Better lookahead for losing moves. So far have not been able to make this fast
* Would checking for and avoiding horizontally symmetric positions give improved performance? Doubtful
* Improved move ordering using heat map. Might be too computationally expensive if curent movecount is high
* Improved move ordering to proritize moves that could lead to 4 in a row
* Parallelize. My computer has 12 threads and 6 cores
* Opening book. Perhaps this is "cheating" but could store the first 6 or so optimal moves
* When complete, can use [-1, 1] as initial search window to pick the first winning move found
*/

int main()
{
	/*
	uint64_t pos = 9013612334262;
	uint64_t mask = 66503138336703;
	uint64_t pos_p2 = pos ^ mask;
	printBitBoard(pos);
	std::cout << "\n";
	printBitBoard(mask);
	std::cout << "\n";
	printBitBoard(pos_p2);
	std::cout << "\n";
	uint64_t r = checkFutureLoss(pos_p2, mask);
	uint64_t r2 = r & (r - 1);
	printBitBoard(r);
	std::cout << "\n";
	printBitBoard(r2);*/

	//uint64_t r = 16;
	//uint64_t c1 = lookupBits[r & 127UL] + lookupBits[(r >> 7) & 127UL] + lookupBits[(r >> 14) & 127UL] +
	//	lookupBits[(r >> 21) & 127UL] + lookupBits[(r >> 28) & 127UL] + lookupBits[(r >> 35) & 127UL] + lookupBits[(r >> 42) & 127UL];
	//std::cout << (c1);
	
	auto started = std::chrono::high_resolution_clock::now();
	testPositions("Test_L2_R2.txt");
	auto step1 = std::chrono::high_resolution_clock::now();
	std::cout << "Searched " << totalNodeCount << " nodes in: " << totalTime / 1000.0 << " seconds" << std::endl;
	std::cout << "Elapsed time: " << std::chrono::duration_cast<std::chrono::milliseconds>(step1 - started).count() / 1000.0 << " seconds" << std::endl;
}


int evaluatePosition(uint64_t position, uint64_t mask, short numMoves, short alpha, short beta) {
	uint64_t key = position + mask + BOTTOM_ROW;
	nodeCount++; //count nodes searched
	bool p1_turn = numMoves % 2 == 0 ? true : false;
	int max = 21 - (numMoves) / 2;	// upper bound of score since player cannot win immediately
	int min = -21 + (numMoves) / 2;	// lower bound of score since player cannot win immediately
	uint64_t hit = table[key % table_size]; //search transposition table for key
	if (hit >> 7 == key) { // transposition table is stored as a 64 bit value: '0000 0000' '49 bit key' '7 bit alpha or beta'
		int val = (int)(hit & 127ULL);
		if (val <= 44) {
			max = val - 22;
			if (beta > max) { // there is no need to keep beta above our max possible score.
				beta = max;
				if (alpha >= beta) return beta;
			}
		}
		else {
			min = val - 66;
			if (alpha < min) {  // there is no need to keep alpha below our min possible score.
				alpha = min;
				if (alpha >= beta) return alpha;
			}
		}
	}

	// Lookahead. If no win, check if the original position has 2+ availabilities for the opponent and if so, return a loss. So far implementation has been too slow.
	uint64_t temp_pos = position;
	if (!p1_turn) {
		temp_pos ^= mask;
	}
	if (checkFutureLoss(temp_pos, mask)!=0) { //check if player has a win
		int alpha = 21 - (numMoves) / 2;
		return alpha;
	}
	else {//no win for current player, check opposite player
		temp_pos ^= mask;
		uint64_t temp_result = checkFutureLoss(temp_pos, mask);
		if (temp_result!=0) {
			if ((temp_result & (temp_result - 1)) != 0) { //if at least two 1s
				int beta = -21 + (numMoves +1) / 2;
				return beta;
			}
		}
	}

//sort column priority by forming column values as: score_col   'score bits' 'new piece height' 'column number'. Sort by value to get largest scores first, then extract the piece hieght and col value
	for (int k = 0; k < 7; k++) { //try every possible column
		int i = k % 2 == 0 ? (3 + (k + 1) / 2) : (3 - (k + 1) / 2); //search center columns first

		uint64_t column = (mask >> (i * 7)) & 63ULL; // Get value of the mask of the column in question. For example: 
															   // A column with 3 pieces in it and an empty 4th row would be "0000111". 0111111 = 63 = full
		if (column != 63) { //check column has room
			uint64_t addition = (column + 1) << (i * 7); //get value for adding a piece
			uint64_t position2 = position;
			uint64_t mask2 = mask;
			mask2 += addition; //add piece
			if (p1_turn) {
				position2 += addition; //get p2 board
			}
			if ((mask2 & 141845657554976ULL) == 141845657554976ULL) return 0; // if the top row is full, return a tie immediately 

			int n2 = numMoves + 1;
			int val = -evaluatePosition(position2, mask2, n2, -beta, -alpha); // recursive negamax call

			if (val >= beta) { // prune paths where val >= beta
				uint64_t entry = key << 7 | (val + 66);
				table[key % table_size] = entry; //save lower bound
				return val;
			}
			if (val > alpha) { // increase alpha if there is a better move found
				alpha = val;
			}

		}
	}
	uint64_t entry = key << 7 | (alpha + 22); //make entry with offset so that alpha+22 is always positive (no signed bit)
	table[key % table_size] = entry; //save upper bound
	return alpha; //note that alpha and beta switch every turn, so alpha is always returned because that is always the best move of the maximizing player

}

int runPosition(uint64_t position, uint64_t mask, short numMoves) {
	int min = -21;
	int max = 21;
	while (min < max) {                    // narrow minimax window. From Pascal Pons
		int med = min + (max - min) / 2;
		if (med <= 0 && min / 2 < med) med = min / 2;
		else if (med >= 0 && max / 2 > med) med = max / 2;
		int r = evaluatePosition(position, mask, numMoves, med, med + 1);   // use a null depth window to know if the actual score is greater or smaller than med
		if (r <= med) max = r;
		else min = r;
	}
	return min;
}

void printBitBoard(uint64_t position) { // prints the bitboard for debugging 
	for (int j = 6; j >= 0; j--) {
		for (int i = 0; i < 7; i++) {
			std::cout << (position >> ((i * 7 + j)) & 1ULL);
		}
		std::cout << std::endl;
	}
}

void printBitBoard(uint64_t position, uint64_t mask) { // prints the bitboard for debugging 
	for (int j = 6; j >= 0; j--) {
		for (int i = 0; i < 7; i++) {
			uint64_t position2 = position ^ mask;
			if (((position & mask) >> ((i * 7 + j)) & 1ULL)) {
				std::cout << "X";
			}
			else if (((position2 & mask) >> ((i * 7 + j)) & 1ULL)) {
				std::cout << "O";
			}
			else {
				std::cout << " ";
			}
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
			uint64_t position = 0;
			uint64_t mask = 0;
			memset(table, 0, table_size); //reset cache table

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
			int result = runPosition(position, mask, short(movesPlayed.length()));
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

bool findGameState(uint64_t position, uint64_t mask) {
	// Really cool way to determine if a position has a win. The position will always come in with the winning player's pieces being "1"s in the bitboard. 
	// The vertical check is the easiest and is explained below, the rest follow the same logic
	uint64_t m = 0;

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

uint64_t checkFutureLoss(uint64_t position, uint64_t mask) {
	//checks for player 1
	uint64_t sq = (position << 1) & (position << 2) & (position << 3); //checks 3 squares under the one it flags

	//horizontal
	uint64_t sq2 = (position << 7) & (position << 14); // _XX
	sq |= sq2 & (position << 21); // _XXX
	sq |= sq2 & (position >> 7); // X_XX
	sq2 >>= 21; // fucking genius, replaces commented line below
	//sq2 = (position >> 7) & (position >> 14); 
	sq |= sq2 & (position >> 21); // XXX_
	sq |= sq2 & (position << 7); // XX_X

	//diagonal 1 \ 
	sq2 = (position << 6) & (position << 12); // _XX
	sq |= sq2 & (position << 18); // _XXX
	sq |= sq2 & (position >> 6); // X_XX
	sq2 >>= 18;
	sq |= sq2 & (position >> 18); // XXX_
	sq |= sq2 & (position << 6); // XX_X

	//diagonal 2 /
	sq2 = (position << 8) & (position << 16); // _XX
	sq |= sq2 & (position << 24); // _XXX
	sq |= sq2 & (position >> 8); // X_XX
	sq2 >>= 24;
	sq |= sq2 & (position >> 24); // XXX_
	sq |= sq2 & (position << 8); // XX_X

	return sq & 279258638311359ULL & (~mask) & (mask + BOTTOM_ROW); // not ideal...
}