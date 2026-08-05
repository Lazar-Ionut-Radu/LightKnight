# Changelog

All notable changes to LightKnight are documented in this file.

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
