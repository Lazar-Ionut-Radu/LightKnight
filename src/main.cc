#include <board.h>
#include <move_search.h>
#include <transposition_table.h>

#include <chrono>
#include <iostream>

int main()
{
    lightknight::Board position(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
        //"r2q2k1/2p3p1/p3p2p/bp1pB3/6P1/2PQ3P/PP1N1r2/2KR2R1 w - - 0 20"
    );

    std::vector<std::vector<lightknight::Move>> move_lists(256);
    for (std::vector<lightknight::Move>& moves : move_lists)
        moves.reserve(256);

    lightknight::search::TranspositionTable tt(16);

    const auto time1 = std::chrono::steady_clock::now();
    int score = lightknight::search::IterativeDeepening(position, 7, tt, 1'000);
    const auto time2 = std::chrono::steady_clock::now();

    std::cout
        << "\nTime elapsed: "
        << std::chrono::duration_cast<std::chrono::milliseconds>(
               time2 - time1
           ).count()
        << "ms.\n\n";
    
    std::cout << "Score: " << score << '\n';
    
    lightknight::search::TTEntry *tt_entry;
    while (tt_entry = tt.Probe(position.zobrist_hash)) {
        if (tt_entry->move.IsNull())
            break;

        std::cout << tt_entry->move << " ";

        lightknight::UndoMoveInfo undo{};
        position.MakeMove(tt_entry->move, undo);
    }
    std::cout << '\n';
   
    return 0;
}