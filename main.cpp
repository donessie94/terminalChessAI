#include"utils/miscellanous.h"
#include"logic/chess_0x88.h"
#include <chrono>

// Forward declaration of print functions, adjust signatures as needed:
void print_mini_board(const int board[128], bool turn, int castle_right);
void print_attack_map(const int board[128], bool turn);

int main()
{
    // Capture overall start time
    auto t_prog_start = std::chrono::high_resolution_clock::now();

    // Construct position from FEN and measure its cost
    auto t_construct_start = std::chrono::high_resolution_clock::now();
    Chess_0x88 state(tricky_position);
    auto t_construct_end = std::chrono::high_resolution_clock::now();

    // Time print_mini_board
    auto t_print1_start = std::chrono::high_resolution_clock::now();
    print_mini_board(state.board, state.turn, state.castle_right, state.en_passant);
    auto t_print1_end = std::chrono::high_resolution_clock::now();

    // Time print_attack_map
    auto t_print2_start = std::chrono::high_resolution_clock::now();
    print_attack_map(state.board, state.turn);
    auto t_print2_end = std::chrono::high_resolution_clock::now();

    // Capture overall end time
    auto t_prog_end = std::chrono::high_resolution_clock::now();

    // Helper to convert duration to double milliseconds
    auto to_ms = [](auto dur) {
        return std::chrono::duration<double, std::milli>(dur).count();
    };
    // or for microseconds:
    auto to_us = [](auto dur) {
        return std::chrono::duration<double, std::micro>(dur).count();
    };

    state.generate_moves();

    // Print timings
    printf("\n=== Timing Report ===\n");
    double dur_construct_ms = to_ms(t_construct_end - t_construct_start);
    printf("Construct Chess_0x88 from FEN: %.3f ms\n", dur_construct_ms);

    double dur_print1_ms = to_ms(t_print1_end - t_print1_start);
    printf("print_mini_board: %.3f ms\n", dur_print1_ms);

    double dur_print2_ms = to_ms(t_print2_end - t_print2_start);
    printf("print_attack_map: %.3f ms\n", dur_print2_ms);

    double dur_total_ms = to_ms(t_prog_end - t_prog_start);
    printf("Total run time: %.3f ms\n", dur_total_ms);
    printf("=====================\n");

    return 0;
}