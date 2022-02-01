// Connect4.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
void printBoard(int board[7][6]);
bool isGameOver(int board[7][6], bool lastPlayer, int last_x, int last_y);

int main()
{
    int board [7][6];
    for (int j = 0; j < 6; j++) {
        for (int i = 0; i < 7; i++) {
            board[i][j] = 0;
        }
    }
   // board[3][0] = 1;
    //board[2][0] = 2;
    //board[4][0] = 2;
    //board[3][1] = 1;
    //board[3][2] = 2;
    //board[3][3] = 2;
    bool turn = true;
    printBoard(board);

    int val;
    while (true) {
        std::cin >> val;
        int y = 6;
        if (val >= 1 && val <= 7) {
            val--;
            for (int j = 5; j >=0; j--) {
                if (board[val][j] == 0) {
                    y = j;
                }
            }
            if (y != 6) {
                board[val][y] = turn ? 1: 2;
                if (isGameOver(board, turn, val, y)) {
                    printBoard(board);
                    std::cout << "Player " << (turn ? 1: 2) << " has won!" << std::endl;
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



void printBoard(int board[7][6]) {
    std::cout << "---------" << std::endl;
    for (int j = 5; j >= 0; j--) {
        std::cout << "|";
        for (int i = 0; i <7; i++) {
            if (board[i][j] == 0) {
                std::cout << " ";
            }
            else if (board[i][j] == 1) {
                std::cout << "X";
            }
            else if (board[i][j] == 2) {
                std::cout << "O";
            }
        }
        std::cout << "|" << std::endl;
    }

    std::cout << "---------" << std::endl;
    std::cout << " 1234567 " << std::endl;
}

bool isGameOver(int board[7][6], bool lastPlayer, int last_x, int last_y) {
    bool isOver = false;
    int count = 0;
    int player = lastPlayer ? 1 : 2;
    //horizontal
    for (int i = 0; i < 7; i++) {
        if (board[i][last_y] == player) {
            count++;
        }
        else {
            count = 0;
        }
        if (count == 4) {
            isOver = true;
            break;
        }
    }

    //vertical
    count = 0;
    for (int j = 0; j < 6; j++) {
        if (board[last_x][j] == player) {
            count++;
        }
        else {
            count = 0;
        }
        if (count == 4) {
            isOver = true;
            break;
        }
    }

    //diagonal 1 /
    count = 0;
    int start_x = last_x;
    int start_y = last_y;
    while (start_x != 0 && start_y != 0) {
        start_x--;
        start_y--;
    }
    while (start_x < 7 && start_y < 6) {
        if (board[start_x][start_y] == player) {
            count++;
        }
        else {
            count = 0;
        }
        if (count == 4) {
            isOver = true;
            break;
        }

        start_x++;
        start_y++;
    }

    //diagonal 2 \ //
    count = 0;
    start_x = last_x;
    start_y = last_y;
    while (start_x != 0 && start_y != 5) {
        start_x--;
        start_y++;
    }
    while (start_x < 7 && start_y >=0) {
        if (board[start_x][start_y] == player) {
            count++;
        }
        else {
            count = 0;
        }
        if (count == 4) {
            isOver = true;
            break;
        }

        start_x++;
        start_y--;
    }

    return isOver;
}