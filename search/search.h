#pragma once
#include"../move/move_gen.h"


namespace RedStone{

namespace Search{

unsigned long long node_count;

// counts all final leafs at a given depth
void inline perft_test(int depth)
{
    // break rule when we reach max depth
    if(depth == MAX_DEPTH)
    {
        node_count++;
        return;
    }

    Move_Gen::generate_moves(depth);

    for(int mv=0; mv<Move_Gen::move_count[depth]; mv++)
    {
        UndoPacked undo_info = Move_Gen::do_move(Move_Gen::valid_moves[depth][mv]);

        perft_test(depth+1);

        Move_Gen::undo_move(Move_Gen::valid_moves[depth][mv], undo_info);
    }
}




}   // end Search namespace
}   // end RedStone namespace