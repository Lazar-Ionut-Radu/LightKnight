# LightKnight

LightKnight is a UCI chess engine written in C++. 

I started the development of this engine only because of my interest in the subject, to get acquainted with the the algorithms, micro-optimizations and heuristics involved. It's kind of a rabbit-hole once you get into it.

In the beginning my goal was just to make something that can beat me at chess. It turns out that's not a hard task at all, so now I will try to make it as good as I can. We'll see how much I can improve it, however it's just a hobby project, thus the "Light" in its name.

## UCI Commands

LightKnight currently supports the following UCI commands:

* `uci`: identifies the engine
* `isready`: responds with `readyok`
* `ucinewgame`: resets the engine state for a new game
* `position startpos [moves <move1> <move2> ...]`: loads the starting position and optionally applies a sequence of moves
* `position fen <fen> [moves <move1> <move2> ...]`: loads a position from FEN and optionally applies a sequence of moves
* `go`: starts searching the current position

  * Supported options: `wtime`, `btime`, `winc`, `binc`, `movestogo`, `movetime`, `depth`, `infinite`
* `stop`: stops the current search and returns the best move found
* `quit`: stops the engine and exits

## Compilation
Building mode is controlled by the ```MODE``` varible, which can be either ```debug``` or ```release```. Debug build retain assertions and debugging information, while release build include optimisation flags. Keep in mind that compiling the engine might take longer than expected because move generation for sliding pieces is precomputed, at compile time.

```python 
make                        # Build debug engine
make MODE=release           # Build release engine
make release                # Build release engine
make tests                  # Build debug tests
make test                   # Build and run debug tests
make MODE=release test      # Build and run release tests
make benchmark              # Run benchmarks with release optimizations
make clean                  # Remove all generated files
```

Debug and release executables are placed in ```build/debug``` and ```build/release``` respectively. 

Building using ```NATIVE=1``` compiles using the -march=native GCC flag to use instruction sets supported by the processor used. As such, builds using ```NATIVE=1``` may not run on different machines.

```bash
make NATIVE=1 release       # Compile engine
make NATIVE=1 benchmark     # Run benchmarks
```

## Helper Scripts
### Perft Debugger
A small utility for testing move generation. It counts the number of leaf nodes in the game tree at a certain depth, from a specified position and compares the result to stockfish, and prints them in a human readable form.

```bash
make perft-debug                                    # Build 
./build/debug/perft-debug "<fen>" <depth> <trace>   # Run perft debug script
```
Running with TRACE=0 prints the leaf-node counts broken down per move, while running with TRACE=1 prints the first discrepancy found and where it occurs. 

Example output:

```
> ./build/debug/perft-debug "6k1/R7/4p2p/5b2/3P2p1/1Q4P1/K1q5/8 w - - 0 1" 3 0

Move      LightKnight       Stockfish         Difference
------------------------------------------------------------
a2a1      653               653               0
a2a3      650               650               0
b3b2      526               526               0
b3c2      321               321               0
------------------------------------------------------------
LightKnight total: 2150
Stockfish total:   2150
Difference:        +0
Result:            MATCH

> ./build/debug/perft-debug "6k1/R7/4p2p/5b2/1Q1P2p1/4q1P1/K7/8 b - - 0 1" 4 1

Final discrepancy
Line:         <root> e3a3
Depth:        4
Move:         e3a3
LightKnight:  <missing>
Stockfish:    881
```

## Acknowledgements
I made use of the [ChessProgramming Wiki](https://www.chessprogramming.org/) as well as the [TalkChess Forum](https://talkchess.com/) extensively for researching aspects of chess engine development. You can find everything you need there.

Also, [Sebastian Lague](https://www.youtube.com/c/SebastianLague) has a nice 2-part series on the subject which sparked my interest and is a great starting point.