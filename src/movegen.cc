// movegen.cc
#include "movegen.h"
#include "types.h"
#include <iostream>
#include <vector>

namespace lightknight::movegen {
    MoveGenInfo GetMoveGenInfo(Board& board) {
        MoveGenInfo info;

        info.pin_info = board.GetAbsolutePinsInfo(board.turn);
        info.checkers_bb = board.AttackersBB(board.piece_bitboards[kWhiteKing + 6*board.turn], board.turn);
    
        return info;
    }
} // namespace lightknight::movegen