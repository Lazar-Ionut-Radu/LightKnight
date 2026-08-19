#ifndef LIGHTKNIGHT_MOVEGEN_H
#define LIGHTKNIGHT_MOVEGEN_H

#include "types.h"
#include "board.h"
#include <array>
#include <vector>
#include <cstdint>
#include <cstddef>

namespace lightknight::movegen {   
    // For generating and making/unmaking castling moves.
    struct CastleInfo {
        uint64_t king_origin;
        uint64_t king_destination;
        uint64_t rook_origin;
        uint64_t rook_destination;
        uint64_t needed_empty; // Squares that need to be empty.
        uint64_t needed_safe; // Squares that need to be not attacked.
    };

    // To be indexed by enum Castle. I know, redundant, I don't care.
    static constexpr CastleInfo kCastleInfo[16] = {
        {}, // unused
        // White Queen Side Castle - 1
        {
            SquareToBitboard(Square::E1), SquareToBitboard(Square::C1), 
            SquareToBitboard(Square::A1), SquareToBitboard(Square::D1),
            SquareToBitboard(Square::B1) | SquareToBitboard(Square::C1) | SquareToBitboard(Square::D1),
            SquareToBitboard(Square::C1) | SquareToBitboard(Square::D1) | SquareToBitboard(Square::E1)
        },
        // White King Side Castle - 2
        {
            SquareToBitboard(Square::E1), SquareToBitboard(Square::G1), 
            SquareToBitboard(Square::H1), SquareToBitboard(Square::F1),
            SquareToBitboard(Square::F1) | SquareToBitboard(Square::G1),
            SquareToBitboard(Square::F1) | SquareToBitboard(Square::G1) | SquareToBitboard(Square::E1)
        }, 
        {}, // unused
        // BLack Queen Side Castle - 4
        {
            SquareToBitboard(Square::E8), SquareToBitboard(Square::C8), 
            SquareToBitboard(Square::A8), SquareToBitboard(Square::D8),
            SquareToBitboard(Square::B8) | SquareToBitboard(Square::C8) | SquareToBitboard(Square::D8),
            SquareToBitboard(Square::C8) | SquareToBitboard(Square::D8) | SquareToBitboard(Square::E8)
        }, 
        {}, {}, {}, // unused
        // King Side Castle - 8
        {
            SquareToBitboard(Square::E8), SquareToBitboard(Square::G8), 
            SquareToBitboard(Square::H8), SquareToBitboard(Square::F8),
            SquareToBitboard(Square::F8) | SquareToBitboard(Square::G8),
            SquareToBitboard(Square::F8) | SquareToBitboard(Square::G8) | SquareToBitboard(Square::E8)
        },
        {}, {}, {}, {}, {}, {}, {} // unused
    };

    enum class MoveGenType {
        kAll,
        kTactical,  // Moves that change the material balance i.e. captures & promotions.
        kCapture,
        kQuiet      // Moves that are not tactical.
    };

    // [TODO]: I'm gonna need to change vector at some point to something faster, for now it's gonna do.
    // Also, this is gon' be straight legal moves.
    
    template<MoveGenType Type>
    size_t GeneratePawnMoves(Board& board, std::vector<Move>& moves) {
        // Booleans for type of move generated
        constexpr bool generate_captures =
            Type == MoveGenType::kAll ||
            Type == MoveGenType::kTactical ||
            Type == MoveGenType::kCapture;

        constexpr bool generate_quiet_promotions =
            Type == MoveGenType::kAll ||
            Type == MoveGenType::kTactical;

        constexpr bool generate_quiet_moves =
            Type == MoveGenType::kAll ||
            Type == MoveGenType::kQuiet;

        // Useful values.
        const Color my_color = board.turn;
        const Color opposite_color = static_cast<Color>(1 - my_color);

        const uint64_t pawns = board.piece_bitboards[Piece::kWhitePawn + 6 * my_color];
        const uint64_t empty = board.piece_bitboards[Piece::kEmpty];
        const uint64_t enemy_pieces = board.color_bitboards[opposite_color];
        const uint64_t blockers = board.color_bitboards[0] | board.color_bitboards[1];
        const Square king_sq = BitboardToSquare(board.piece_bitboards[Piece::kWhiteKing + 6 * my_color]);
        
        const uint64_t enemy_rooks_queens =
            board.piece_bitboards[Piece::kWhiteRook + 6 * opposite_color] |
            board.piece_bitboards[Piece::kWhiteQueen + 6 * opposite_color];
        const uint64_t enemy_bishops_queens =
            board.piece_bitboards[Piece::kWhiteBishop + 6 * opposite_color] |
            board.piece_bitboards[Piece::kWhiteQueen + 6 * opposite_color];
        
            const uint64_t enemy_pawns = board.piece_bitboards[Piece::kWhitePawn + 6 * opposite_color];
        const uint64_t enemy_knights = board.piece_bitboards[Piece::kWhiteKnight + 6 * opposite_color];
            
        size_t new_moves_count = 0;

        // Helper funcs.
        const auto leaves_king_in_check =
            [&](uint64_t origin_bb,
                uint64_t destination_bb,
                uint64_t captured_bb = 0ULL) {
                const uint64_t relevant_blockers = (blockers & ~origin_bb & ~captured_bb) | destination_bb;
                const uint64_t straight_attacks = RookAttackBB(king_sq, relevant_blockers);
                const uint64_t diagonal_attacks = BishopAttackBB(king_sq, relevant_blockers);

                const bool straight_check = straight_attacks & (enemy_rooks_queens & ~captured_bb);
                const bool diagonal_check = diagonal_attacks & (enemy_bishops_queens & ~captured_bb);
                const bool pawn_check = PawnAttackBB(king_sq, my_color) & (enemy_pawns & ~captured_bb);
                const bool knight_check = kKnightAttacksBB[king_sq] & (enemy_knights & ~captured_bb);

                return straight_check || diagonal_check || pawn_check || knight_check;
            };

        const auto add_promotions =
            [&](Square origin, Square destination) {
                moves.push_back(
                    Move(
                        origin,
                        destination,
                        PromotionPieceType::kQueen,
                        MoveType::kPromotion
                    )
                );
                moves.push_back(
                    Move(
                        origin,
                        destination,
                        PromotionPieceType::kRook,
                        MoveType::kPromotion
                    )
                );
                moves.push_back(
                    Move(
                        origin,
                        destination,
                        PromotionPieceType::kBishop,
                        MoveType::kPromotion
                    )
                );
                moves.push_back(
                    Move(
                        origin,
                        destination,
                        PromotionPieceType::kKnight,
                        MoveType::kPromotion
                    )
                );

                new_moves_count += 4;
            };
        
        // Generate captures.
        if constexpr (generate_captures) {
            uint64_t remaining_pawns = pawns;

            // Normal captures.
            while (remaining_pawns) {
                const uint64_t pawn_bb = LSB(remaining_pawns);
                const Square origin_sq = BitboardToSquare(pawn_bb);

                uint64_t captures = PawnAttackBB(origin_sq, my_color) & enemy_pieces;

                while (captures) {
                    const uint64_t destination_bb = LSB(captures);
                    const Square destination_sq = BitboardToSquare(destination_bb);

                    if (!leaves_king_in_check(pawn_bb, destination_bb, destination_bb)) {
                        if (destination_bb & kRankPromotion[my_color]
                        ) {
                            add_promotions(origin_sq, destination_sq);
                        } else {
                            moves.push_back(Move(origin_sq, destination_sq));
                            ++new_moves_count;
                        }
                    }
                    captures &= ~destination_bb;
                }
                remaining_pawns &= ~pawn_bb;
            }

            // En passants.
            if (board.en_passant) {
                uint64_t en_passant_takers = pawns & 
                    (Backward(West(board.en_passant), my_color) | Backward(East(board.en_passant), my_color));

                while (en_passant_takers) {
                    const uint64_t origin_bb = LSB(en_passant_takers);
                    const uint64_t destination_bb = board.en_passant;
                    const uint64_t captured_bb = Backward(destination_bb, my_color);

                    if (!leaves_king_in_check(origin_bb, destination_bb, captured_bb)
                    ) {
                        moves.push_back(Move(BitboardToSquare(origin_bb), BitboardToSquare(destination_bb), PromotionPieceType::kKnight, MoveType::kEnPassant));
                        ++new_moves_count;
                    }

                    en_passant_takers &= ~origin_bb;
                }
            }
        }

        // Generate 1 square pawn pushes.
        if constexpr (generate_quiet_moves || generate_quiet_promotions) {
            uint64_t remaining_pawns = pawns;

            while (remaining_pawns) {
                const uint64_t pawn_bb = LSB(remaining_pawns);
                const uint64_t destination_bb = Forward(pawn_bb, my_color);

                if (destination_bb & empty) {
                    const bool is_promotion = destination_bb & kRankPromotion[my_color];

                    if (!leaves_king_in_check(pawn_bb, destination_bb)) {
                        if (is_promotion) {
                            if constexpr (generate_quiet_promotions) {
                                add_promotions(BitboardToSquare(pawn_bb), BitboardToSquare(destination_bb));
                            }
                        } else {
                            if constexpr (generate_quiet_moves) {
                                moves.push_back(Move(BitboardToSquare(pawn_bb), BitboardToSquare(destination_bb)));
                                ++new_moves_count;
                            }
                        }
                    }
                }

                remaining_pawns &= ~pawn_bb;
            }
        }

        // Generate double pushes.
        if constexpr (generate_quiet_moves) {
            uint64_t remaining_pawns = pawns;

            while (remaining_pawns) {
                const uint64_t pawn_bb = LSB(remaining_pawns);

                if (pawn_bb & kRankPawnDoublePush[my_color]) {
                    const uint64_t middle_bb = Forward(pawn_bb, my_color);
                    const uint64_t destination_bb = Forward(middle_bb, my_color);

                    if ((middle_bb & empty) && (destination_bb & empty) && !leaves_king_in_check(pawn_bb, destination_bb)) {
                        moves.push_back(Move(BitboardToSquare(pawn_bb), BitboardToSquare(destination_bb)));
                        ++new_moves_count;
                    }
                }

                remaining_pawns &= ~pawn_bb;
            }
        }

        return new_moves_count;
    }

    template <MoveGenType type>
    size_t GenerateKnightMoves(Board &board, std::vector<Move> &moves) {
        Color my_color = board.turn;
        Color opposite_color = (Color)(1 - my_color);

        // For generating candidate moves for a knight.
        uint64_t knights_bb = board.piece_bitboards[Piece::kWhiteKnight + 6 * my_color];
        uint64_t good_dests_bb = 0ULL;
        if constexpr (type == movegen::MoveGenType::kAll) {
            good_dests_bb = board.color_bitboards[opposite_color] | board.piece_bitboards[Piece::kEmpty];
        }
        else if constexpr (type == movegen::MoveGenType::kQuiet) {
            good_dests_bb = board.piece_bitboards[Piece::kEmpty];
        } 
        else if constexpr (
            type == movegen::MoveGenType::kTactical ||
            type == movegen::MoveGenType::kCapture
        ) {
            good_dests_bb = board.color_bitboards[opposite_color];
        }

        // For checking later if a move leaves you in check
        uint64_t blockers = (board.color_bitboards[0] | board.color_bitboards[1]);
        uint64_t king_bb = board.piece_bitboards[Piece::kWhiteKing + 6 * my_color];
        Square king_sq = BitboardToSquare(king_bb); 
        
        size_t new_moves_count = 0;

        // Itterate through all knight of that color.
        while (knights_bb) {
            uint64_t origin_bb = LSB(knights_bb);

            // Get the moves that don't capture one of your pieces.
            uint64_t attacks_bb = kKnightAttacksBB[BitboardToSquare(origin_bb)];
            attacks_bb &= good_dests_bb;

            // Itterate through them
            while (attacks_bb) {
                uint64_t dest_bb = LSB(attacks_bb);

                uint64_t relevant_blockers = (blockers | dest_bb) & ~origin_bb;
                uint64_t straights = RookAttackBB(king_sq, relevant_blockers) & ~dest_bb;
                uint64_t diagonals = BishopAttackBB(king_sq, relevant_blockers) & ~dest_bb;

                bool check_on_straights = straights & (board.piece_bitboards[Piece::kWhiteRook + 6*opposite_color] | board.piece_bitboards[Piece::kWhiteQueen + 6*opposite_color]);
                bool check_on_diagonals = diagonals & (board.piece_bitboards[Piece::kWhiteBishop + 6*opposite_color] | board.piece_bitboards[Piece::kWhiteQueen + 6*opposite_color]);
                bool pawn_check = (PawnAttackBB(BitboardToSquare(king_bb), my_color) & ~dest_bb) & board.piece_bitboards[Piece::kWhitePawn + 6*opposite_color];
                bool knight_check = (kKnightAttacksBB[BitboardToSquare(king_bb)] & ~dest_bb) & board.piece_bitboards[Piece::kWhiteKnight + 6*opposite_color];

                if (check_on_diagonals || check_on_straights || pawn_check || knight_check) {
                    attacks_bb &= ~dest_bb;
                    continue;
                }

                // Add to move list.
                moves.push_back(Move(BitboardToSquare(origin_bb), BitboardToSquare(dest_bb)));
                new_moves_count++;

                // Remove LSB
                attacks_bb &= ~dest_bb;
            }

            // Remove LSB
            knights_bb &= ~origin_bb;
        }

        return new_moves_count;
    }

    template<MoveGenType type>
    size_t GenerateBishopMoves(Board &board, std::vector<Move> &moves) {
        Color my_color = board.turn;
        Color opposite_color = (Color)(1 - my_color);

        // For generating candidate moves for a bishop
        uint64_t bishops_bb = board.piece_bitboards[Piece::kWhiteBishop + 6 * my_color];
        uint64_t good_dests_bb = 0ULL;
        if constexpr (type == movegen::MoveGenType::kAll) {
            good_dests_bb = board.color_bitboards[opposite_color] | board.piece_bitboards[Piece::kEmpty];
        }
        else if constexpr (type == movegen::MoveGenType::kQuiet) {
            good_dests_bb = board.piece_bitboards[Piece::kEmpty];
        } 
        else if constexpr (
            type == movegen::MoveGenType::kTactical ||
            type == movegen::MoveGenType::kCapture
        ) {
            good_dests_bb = board.color_bitboards[opposite_color];
        }

        // For checking later if a move leaves you in check.
        uint64_t king_bb = board.piece_bitboards[Piece::kWhiteKing + 6 * my_color];
        Square king_sq = BitboardToSquare(king_bb); 
        uint64_t blockers = (board.color_bitboards[0] | board.color_bitboards[1]);
                
        // Count number of new moves added.
        size_t new_moves_count = 0;

        // Itterate through all the bishops.
        while (bishops_bb) {
            uint64_t origin_bb = LSB(bishops_bb);

            // Get the possible destination squares for this move, that don't capture one of your pieces.
            uint64_t attacks_bb = BishopAttackBB(BitboardToSquare(origin_bb), blockers);
            attacks_bb &= good_dests_bb;

            // Itterate through them
            while (attacks_bb) {
                uint64_t dest_bb = LSB(attacks_bb);

                // Make sure moving this bishop does not leave your king in check.
                uint64_t relevant_blockers = (blockers | dest_bb) & ~origin_bb;
                uint64_t straights = RookAttackBB(king_sq, relevant_blockers) & ~dest_bb;
                uint64_t diagonals = BishopAttackBB(king_sq, relevant_blockers) & ~dest_bb;
                
                bool check_on_straights = straights & (board.piece_bitboards[Piece::kWhiteRook + 6*opposite_color] | board.piece_bitboards[Piece::kWhiteQueen + 6*opposite_color]);
                bool check_on_diagonals = diagonals & (board.piece_bitboards[Piece::kWhiteBishop + 6*opposite_color] | board.piece_bitboards[Piece::kWhiteQueen + 6*opposite_color]);
                bool pawn_check = (PawnAttackBB(BitboardToSquare(king_bb), my_color) & ~dest_bb) & board.piece_bitboards[Piece::kWhitePawn + 6*opposite_color];
                bool knight_check = (kKnightAttacksBB[BitboardToSquare(king_bb)] & ~dest_bb) & board.piece_bitboards[Piece::kWhiteKnight + 6*opposite_color];

                if (check_on_diagonals || check_on_straights || pawn_check || knight_check) {
                    attacks_bb &= ~dest_bb;
                    continue;
                }
                
                // Move is legal.
                moves.push_back(Move(BitboardToSquare(origin_bb), BitboardToSquare(dest_bb)));
                new_moves_count++;

                // Pop LSB
                attacks_bb &= ~dest_bb;
            }

            // Pop LSB
            bishops_bb &= ~origin_bb;
        }

        return new_moves_count;
    }

    template <MoveGenType type>
    size_t GenerateRookMoves(Board &board, std::vector<Move> &moves) {
        Color my_color = board.turn;
        Color opposite_color = (Color)(1 - my_color);

        // For generating candidate moves for a rook
        uint64_t rooks_bb = board.piece_bitboards[Piece::kWhiteRook + 6 * my_color];
        uint64_t good_dests_bb = 0ULL; 
        if constexpr (type == movegen::MoveGenType::kAll) {
            good_dests_bb = board.color_bitboards[opposite_color] | board.piece_bitboards[Piece::kEmpty];
        }
        else if constexpr (type == movegen::MoveGenType::kQuiet) {
            good_dests_bb = board.piece_bitboards[Piece::kEmpty];
        } 
        else if constexpr (
            type == movegen::MoveGenType::kTactical ||
            type == movegen::MoveGenType::kCapture
        ) {
            good_dests_bb = board.color_bitboards[opposite_color];
        }

        // For checking later if a move leaves you in check.
        uint64_t king_bb = board.piece_bitboards[Piece::kWhiteKing + 6 * my_color];
        Square king_sq = BitboardToSquare(king_bb); 
        uint64_t blockers = (board.color_bitboards[0] | board.color_bitboards[1]);
                
        // Count number of new moves added.
        size_t new_moves_count = 0;

        // Itterate through all the bishops.
        while (rooks_bb) {
            uint64_t origin_bb = LSB(rooks_bb);

            // Get the possible destination squares for this move, that don't capture one of your pieces.
            uint64_t attacks_bb = RookAttackBB(BitboardToSquare(origin_bb), blockers);
            attacks_bb &= good_dests_bb;

            // Itterate through them
            while (attacks_bb) {
                uint64_t dest_bb = LSB(attacks_bb);

                // Make sure moving this bishop does not leave your king in check.
                uint64_t relevant_blockers = (blockers | dest_bb) & ~origin_bb;
                uint64_t straights = RookAttackBB(king_sq, relevant_blockers) & ~dest_bb;
                uint64_t diagonals = BishopAttackBB(king_sq, relevant_blockers) & ~dest_bb;
                
                bool check_on_straights = straights & (board.piece_bitboards[Piece::kWhiteRook + 6*opposite_color] | board.piece_bitboards[Piece::kWhiteQueen + 6*opposite_color]);
                bool check_on_diagonals = diagonals & (board.piece_bitboards[Piece::kWhiteBishop + 6*opposite_color] | board.piece_bitboards[Piece::kWhiteQueen + 6*opposite_color]);
                bool pawn_check = (PawnAttackBB(BitboardToSquare(king_bb), my_color) & ~dest_bb) & board.piece_bitboards[Piece::kWhitePawn + 6*opposite_color];
                bool knight_check = (kKnightAttacksBB[BitboardToSquare(king_bb)] & ~dest_bb) & board.piece_bitboards[Piece::kWhiteKnight + 6*opposite_color];

                if (check_on_diagonals || check_on_straights || pawn_check || knight_check) {
                    attacks_bb &= ~dest_bb;
                    continue;
                }
                
                // Move is legal.
                moves.push_back(Move(BitboardToSquare(origin_bb), BitboardToSquare(dest_bb)));
                new_moves_count++;

                // Pop LSB
                attacks_bb &= ~dest_bb;
            }

            // Pop LSB
            rooks_bb &= ~origin_bb;
        }

        return new_moves_count;
    }

    template<MoveGenType type>
    size_t GenerateQueenMoves(Board &board, std::vector<Move> &moves) {
        Color my_color = board.turn;
        Color opposite_color = (Color)(1 - my_color);

        // For generating candidate moves for a queen.
        uint64_t queens_bb = board.piece_bitboards[Piece::kWhiteQueen + 6 * my_color];
        uint64_t good_dests_bb = 0ULL; 
        if constexpr (type == movegen::MoveGenType::kAll) {
            good_dests_bb = board.color_bitboards[opposite_color] | board.piece_bitboards[Piece::kEmpty];
        }
        else if constexpr (type == movegen::MoveGenType::kQuiet) {
            good_dests_bb = board.piece_bitboards[Piece::kEmpty];
        } 
        else if constexpr (
            type == movegen::MoveGenType::kTactical ||
            type == movegen::MoveGenType::kCapture
        ) {
            good_dests_bb = board.color_bitboards[opposite_color];
        }

        // For checking later if a move leaves you in check.
        Square king_sq = BitboardToSquare(board.piece_bitboards[Piece::kWhiteKing + 6 * my_color]); 
        uint64_t blockers = (board.color_bitboards[0] | board.color_bitboards[1]);
                
        // Count number of new moves added.
        size_t new_moves_count = 0;

        // Itterate through all the bishops.
        while (queens_bb) {
            uint64_t origin_bb = LSB(queens_bb);

            // Get the possible destination squares for this move, that don't capture one of your pieces.
            uint64_t attacks_bb = QueenAttackBB(BitboardToSquare(origin_bb), blockers);
            attacks_bb &= good_dests_bb;

            // Itterate through them
            while (attacks_bb) {
                uint64_t dest_bb = LSB(attacks_bb);

                // Make sure moving this bishop does not leave your king in check.
                uint64_t relevant_blockers = (blockers | dest_bb) & ~origin_bb;
                uint64_t straights = RookAttackBB(king_sq, relevant_blockers) & ~dest_bb;
                uint64_t diagonals = BishopAttackBB(king_sq, relevant_blockers) & ~dest_bb;
                
                bool check_on_straights = straights & (board.piece_bitboards[Piece::kWhiteRook + 6*opposite_color] | board.piece_bitboards[Piece::kWhiteQueen + 6*opposite_color]);
                bool check_on_diagonals = diagonals & (board.piece_bitboards[Piece::kWhiteBishop + 6*opposite_color] | board.piece_bitboards[Piece::kWhiteQueen + 6*opposite_color]);
                bool pawn_check = (PawnAttackBB(king_sq, my_color) & ~dest_bb) & board.piece_bitboards[Piece::kWhitePawn + 6*opposite_color];
                bool knight_check = (kKnightAttacksBB[king_sq] & ~dest_bb) & board.piece_bitboards[Piece::kWhiteKnight + 6*opposite_color];

                if (check_on_diagonals || check_on_straights || knight_check || pawn_check) {
                    attacks_bb &= ~dest_bb;
                    continue;
                }
                
                // Move is legal.
                moves.push_back(Move(BitboardToSquare(origin_bb), BitboardToSquare(dest_bb)));
                new_moves_count++;

                // Pop LSB
                attacks_bb &= ~dest_bb;
            }

            // Pop LSB
            queens_bb &= ~origin_bb;
        }

        return new_moves_count;
    }

    template<MoveGenType type>
    size_t GenerateKingMoves(Board &board, std::vector<Move> &moves) {
        Color my_color = board.turn;
        Color opposite_color = (Color)(1 - my_color);
        Square king_sq = BitboardToSquare(board.piece_bitboards[Piece::kWhiteKing + 6 * my_color]);
        Square enemy_king_sq = BitboardToSquare(board.piece_bitboards[Piece::kWhiteKing + 6 * opposite_color]);
        uint64_t blockers = (board.color_bitboards[0] | board.color_bitboards[1]);
        
        // Get king destination squares that don't put the king near the enemy king or take its own pieces.
        uint64_t king_attacks_bb = kKingAttacksBB[king_sq] & ~kKingAttacksBB[enemy_king_sq];
        king_attacks_bb &= ~board.color_bitboards[my_color];

        // Count the numver of new moves.
        size_t new_moves_count = 0;

        // To generate captures only.
        if constexpr (
            type == movegen::MoveGenType::kTactical ||
            type == movegen::MoveGenType::kCapture
        ) {
            king_attacks_bb &= board.color_bitboards[opposite_color];
        }

        // To generate quiet moves only.
        if constexpr (
            type == movegen::MoveGenType::kQuiet
        ) {
            king_attacks_bb &= ~board.color_bitboards[opposite_color];
        }

        // Itterate through them.
        while (king_attacks_bb) {
            uint64_t dest_bb = LSB(king_attacks_bb);

            // Check if this move leaves the king in check.
            uint64_t relevant_blockers = (blockers & ~SquareToBitboard(king_sq)) & ~dest_bb;
            uint64_t straights = RookAttackBB(BitboardToSquare(dest_bb), relevant_blockers);
            uint64_t diagonals = BishopAttackBB(BitboardToSquare(dest_bb), relevant_blockers); 

            bool check_on_staights = straights & (board.piece_bitboards[Piece::kWhiteRook + 6*opposite_color] | board.piece_bitboards[Piece::kWhiteQueen + 6*opposite_color]);
            bool check_on_diagonals = diagonals & (board.piece_bitboards[Piece::kWhiteBishop + 6*opposite_color] | board.piece_bitboards[Piece::kWhiteQueen + 6*opposite_color]);
            bool pawn_check = PawnAttackBB(BitboardToSquare(dest_bb), my_color) & board.piece_bitboards[Piece::kWhitePawn + 6*opposite_color];
            bool knight_check = kKnightAttacksBB[BitboardToSquare(dest_bb)] & board.piece_bitboards[Piece::kWhiteKnight + 6*opposite_color];

            if (check_on_diagonals || check_on_staights || pawn_check || knight_check) {
                king_attacks_bb &= ~dest_bb;
                continue;
            }

            // Move is legal.
            moves.push_back(Move(BitboardToSquare(SquareToBitboard(king_sq)), BitboardToSquare(dest_bb)));
            new_moves_count++;

            // Pop LSB
            king_attacks_bb &= ~dest_bb;
        }

        // Castling
        uint8_t relevant_castles = board.castling & kCastlesByColor[my_color];
        
        // Castles are quiet moves.
        if constexpr (
            type == lightknight::movegen::MoveGenType::kAll ||
            type == lightknight::movegen::MoveGenType::kQuiet
        ) {
            while (relevant_castles) {
                uint8_t current_castle = LSB(relevant_castles);
                
                // Check the needed squares are empty.
                if (kCastleInfo[current_castle].needed_empty & (board.color_bitboards[0] | board.color_bitboards[1])) {
                    relevant_castles &= ~current_castle;
                    continue;
                }

                // Check the square that need to be safe are safe.
                uint64_t needed_safe = kCastleInfo[current_castle].needed_safe;
                bool safe = true;
                while (safe && needed_safe) {
                    uint64_t needed_safe_lsb = LSB(needed_safe);
                    
                    if (board.IsSquareAttacked(needed_safe_lsb, my_color)) {
                        safe = false;
                    }
                
                    // Pop LSB
                    needed_safe &= ~needed_safe_lsb;
                }
                if (!safe) {
                    relevant_castles &= ~current_castle;
                    continue;
                }

                // We trust that the board.castles flag ensures the king / rook have not been moved.
                // As such, this castle is legal.
                Square origin_sq = BitboardToSquare(kCastleInfo[current_castle].king_origin);
                Square dest_sq = BitboardToSquare(kCastleInfo[current_castle].king_destination);
                moves.push_back(Move(origin_sq, dest_sq, PromotionPieceType::kKnight, MoveType::kCastling));
                new_moves_count++;

                // Pop LSB
                relevant_castles &= ~current_castle;
            }    
        }
        return new_moves_count;
    }

    template<MoveGenType type>
    size_t GenerateMoves(Board &board, std::vector<Move> &moves) {
        size_t moves_count = 0;

        moves_count += GeneratePawnMoves<type>(board, moves);
        moves_count += GenerateKnightMoves<type>(board, moves);
        moves_count += GenerateBishopMoves<type>(board, moves);
        moves_count += GenerateRookMoves<type>(board, moves);
        moves_count += GenerateQueenMoves<type>(board, moves);
        moves_count += GenerateKingMoves<type>(board, moves);
        
        return moves_count;
    }
} // namespace lightknight::movegen

#endif // LIGHTKNIGHT_MOVEGEN_H