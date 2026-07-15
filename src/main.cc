#include <board.h>
#include <types.h>
#include <movegen.h>
#include <vector>
#include <iostream>

int main () 
{
    lightknight::Board pos = lightknight::Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    std::vector<lightknight::Move> moves;
    
    std::cout << lightknight::movegen::GenerateMoves(pos, moves) << std::endl;
    for (lightknight::Move move : moves) {
        std::cout << move << std::endl;
    }
    return 0;
}