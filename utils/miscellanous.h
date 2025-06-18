#pragma once
#include <cstdio>
#include"../moveHelp/encoder.h"

// Function declarations
void print_move_info_extended(unsigned int mv);
void print_move_info(unsigned int mv);
void print_mini_board(const int board[], bool turn, int castle_right, int en_passant);
void zero_board(int board[]);
void parse_fen_str(const char fen[], int board[], bool &turn, int &castle_right, int &en_passant);
void print_attack_map(const int board[], const bool turn);
bool is_square_attacked(const int *board, const bool turn, const int idx);