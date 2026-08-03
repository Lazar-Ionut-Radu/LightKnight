// main.cc
#include <board.h>
#include <move_search.h>
#include <transposition_table.h>
#include <uci.h>

#include <chrono>
#include <iostream>

int main()
{
    lightknight::uci::UCI uci;
    uci.Loop();
    
    return 0;
}