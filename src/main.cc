// main.cc
#include "board.h"
#include "move_search.h"
#include "transposition_table.h"
#include "uci.h"

#include <chrono>
#include <iostream>

int main()
{
    lightknight::Engine engine = lightknight::Engine();
    engine.SaveParameters("params.csv");

    lightknight::uci::UCI uci;
    uci.Loop();
    
    return 0;
}