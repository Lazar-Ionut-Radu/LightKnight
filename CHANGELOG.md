# Changelog

All notable changes to LightKnight are documented in this file.

## [0.4.0] - 2026-08-17

### Changed
- Refactored engine parameters into their own file (params.h/params.cc). They are now owned by the engine class and given to the search / eval functions explicitly.

### Added
- Functions to save/load parameters from a .csv file.
- UCI option and setoptions command support.
- The following options:
    - Hash: for setting the transposition table size in MB
    - ParametersFile: specify path to the .csv file containing parameters
    - SaveParameters
    - LoadParameters
- Code for tuning parameters using the [Texel tuning method](https://chessprogramming.org/Texel%27s_Tuning_Method) with [hill climbing](https://en.wikipedia.org/wiki/Hill_climbing) and [SPSA](https://chessprogramming.org/SPSA). Whether or not it works is debatable, I had not had much success with it. 

## [0.3.4] - 2026-08-09

### Added
- Evaluation: small king safety evaluation, pawn shield.

## [0.3.3] - 2026-08-08

### Added
- Evaluation: passed, isolated, doubled, tripled, protected and connected pawns.

### Fixed
- Internally saving the en passant target square even if there is no piece that can do the capture. Fixes a rare bug where 3-fold repetition is not found.

## [0.3.2] - 2026-08-07

### Added
- Evaluation: Piece mobility.
- Evaluation: Small bonuses for tempo and having a bishop pair.

## [0.3.1] - 2026-08-07

### Added
- Game phase calculation for evaluation. Computed by counting material and rescaled to [0, 1024].

### Changed
- Tapered eval, with values for midgame and endgame that are interpolated between using the phase value.

## [0.3.0] - 2026-08-07

### Added
- TT aging mechanism. Entries encountered during the current search get refreshes (generation variable = current generation). Older entries gradually lose importance according to their age; Replacement based on depth is now: new_depth > old_depth - age_difference.

### Fixed
- TT is no longer cleared between moves of the same game.

## [0.2.1] - 2026-08-05

### Fixed
- Search now considers draw by 50-move rule.
- PV computed correctly from the TT in case of 3-fold repetition; it no longer repeats until max size is achieved. 
## [0.2.0] - 2026-08-04

### Added
- Move Ordering: MVV-LVA scoring: Moves are now sorted as follows:
    - TT move
    - Promotions to Q
    - Favourable *captures* (in the sense that the attacker is less valuable than the victim)
    - Equal captures
    - Underpromotions
    - Quiet moves
    - Losing captures

### Changed
- Move Picking: During the search moves are selected incrementally instead of sorting the complete move list. This has a larger asymptotic complexity, however, in practice, only a handful of moves are examined, thus being faster.

## [0.1.0] - 2026-08-03

### Added 
- Bitboard based board representation.
- Legal move generation using magic bitboards for sliding pieces.
- Make, Unmake move support.
- Iterative-deepening Principal Variation Search. 
- Quiescence Search.
- Zobrist hashing.
- Transposition Table.
- Detection of 3-fold repetition. A 2-fold repetition that is fully inside the search tree is scored as 0.
- Extremely simple move ordering, TT move first.
- Evaluation based on material evaluation and piece-square tables.
- UCI protocol support
- Minimal test suite, including a perft test.
- Perft debug utility for comparison with Stockfish.
