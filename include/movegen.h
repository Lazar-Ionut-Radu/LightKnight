#ifndef LIGHTKNIGHT_MOVEGEN_H
#define LIGHTKNIGHT_MOVEGEN_H

#include "types.h"
#include "board.h"
#include <array>
#include <vector>
#include <cstdint>
#include <cstddef>


// Followed the following blog post when implementing legal move generation.
// https://peterellisjones.com/posts/generating-legal-chess-moves-efficiently/


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
        kTactical,  // captures + promotions
        kCapture,   // captures, including capture promotions
        kQuiet      // quiet moves.
    };


    // Stuff precomputed in GenerateMoves for move generation to avoid duplicating work.
    struct MoveGenInfo {
        PinInfo pin_info;
        uint64_t checkers_bb;
    };
    
    MoveGenInfo GetMoveGenInfo(Board& board);
    
    template<MoveGenType type>
    size_t GeneratePawnMoves(Board& board, std::vector<Move>& moves, const MoveGenInfo& precomputed_info) {
        size_t move_count = 0;

        // Helpers
        const Color my_color = board.turn;
        const Color enemy_color = OppositeColor(my_color);
        const uint64_t own_king = board.piece_bitboards[kWhiteKing + 6*my_color];
        const Square own_king_sq = BitboardToSquare(own_king);
        const size_t num_checkers = SetBitsCount(precomputed_info.checkers_bb);

        // Only king evasions are legal when in double check.
        if (num_checkers >= 2) 
            return 0;

        // Candidate destination square, in case there's a check to block.
        uint64_t capture_block_bb = ~0ull;
        if (num_checkers == 1) {
            capture_block_bb = 0ull;
            
            // Capture the checking piece.
            if constexpr (type != MoveGenType::kQuiet)
                capture_block_bb |= precomputed_info.checkers_bb;

            // Block the check.
            if constexpr (type == MoveGenType::kAll || type == MoveGenType::kQuiet) {
                if (precomputed_info.checkers_bb &
                    ( board.piece_bitboards[kWhiteBishop + 6*enemy_color]
                    | board.piece_bitboards[kWhiteRook + 6*enemy_color]
                    | board.piece_bitboards[kWhiteQueen + 6*enemy_color])
                ) {
                    capture_block_bb |= kSegmentBB[BitboardToSquare(precomputed_info.checkers_bb)][own_king_sq];
                }
            }
        }

        // Helpers.
        const uint64_t empty_bb = board.piece_bitboards[Piece::kEmpty];    
        const uint64_t pawns = board.piece_bitboards[Piece::kWhitePawn + 6*my_color];
        const uint64_t non_pinned_pawns = pawns & ~precomputed_info.pin_info.pinned_bb;
        const uint64_t pinned_pawns = pawns & precomputed_info.pin_info.pinned_bb;

        // ----- Quiet Moves -----
        if (type == MoveGenType::kAll || type == MoveGenType::kQuiet) {
            // ----- Double Pushes -----
            uint64_t double_movers = pawns & kRankPawnDoublePush[my_color];
            uint64_t double_movers_to = Forward(Forward(double_movers, my_color) & empty_bb, my_color) & empty_bb & capture_block_bb;
            
            for (uint64_t bb = double_movers_to; bb; bb &= ~LSB(bb)) {
                const Square to_sq = LSBSquare(bb);
                const uint64_t to_bb = SquareToBitboard(to_sq);
                const uint64_t from_bb = Backward(Backward(to_bb, my_color), my_color);  
                const Square from_sq = BitboardToSquare(from_bb);
                
                // Pinned
                if (from_bb & precomputed_info.pin_info.pinned_bb) {
                    const uint64_t pin_line_bb = kLineBB[own_king_sq][from_sq];
                    if (pin_line_bb & to_bb) {
                        moves.push_back(Move(from_sq, to_sq));
                        move_count++;
                    }
                } 
                // Non pinned
                else {
                    moves.push_back(Move(from_sq, to_sq));
                    move_count++;
                }
            }
            
            // ----- Non Promotion Single Pushes -----
            uint64_t single_movers_to = Forward(pawns, my_color) & empty_bb & ~kRankPromotion[my_color] & capture_block_bb;
            
            for (uint64_t bb = single_movers_to; bb; bb &= ~LSB(bb)) {
                const Square to_sq = LSBSquare(bb);
                const uint64_t to_bb = SquareToBitboard(to_sq);
                const uint64_t from_bb = Backward(to_bb, my_color);  
                const Square from_sq = BitboardToSquare(from_bb);
                
                // Pinned
                if (from_bb & precomputed_info.pin_info.pinned_bb) {
                    const uint64_t pin_line_bb = kLineBB[own_king_sq][from_sq];
                    if (pin_line_bb & to_bb) {
                        moves.push_back(Move(from_sq, to_sq));
                        move_count++;
                    }
                } 
                // Non pinned
                else {
                    moves.push_back(Move(from_sq, to_sq));
                    move_count++;
                }
            }
        }
        
        // ----- Non-Capture Promotions -----
        if (type == MoveGenType::kAll || type == MoveGenType::kTactical) {
            uint64_t dests_bb = Forward(pawns, my_color) & empty_bb & kRankPromotion[my_color] & capture_block_bb;
            
            for (uint64_t bb = dests_bb; bb; bb &= ~LSB(bb)) {
                const Square to_sq = LSBSquare(bb);
                const uint64_t to_bb = SquareToBitboard(to_sq);
                const uint64_t from_bb = Backward(to_bb, my_color);  
                const Square from_sq = BitboardToSquare(from_bb);
                
                // Pinned
                if (from_bb & precomputed_info.pin_info.pinned_bb) {
                    const uint64_t pin_line_bb = kLineBB[own_king_sq][from_sq];
                    if (pin_line_bb & to_bb) {
                        moves.push_back(Move(from_sq, to_sq, PromotionPieceType::kKnight, MoveType::kPromotion));
                        moves.push_back(Move(from_sq, to_sq, PromotionPieceType::kBishop, MoveType::kPromotion));
                        moves.push_back(Move(from_sq, to_sq, PromotionPieceType::kRook, MoveType::kPromotion));
                        moves.push_back(Move(from_sq, to_sq, PromotionPieceType::kQueen, MoveType::kPromotion));
                        move_count += 4;
                    }
                }
                // Non Pinned
                else {
                    moves.push_back(Move(from_sq, to_sq, PromotionPieceType::kKnight, MoveType::kPromotion));
                    moves.push_back(Move(from_sq, to_sq, PromotionPieceType::kBishop, MoveType::kPromotion));
                    moves.push_back(Move(from_sq, to_sq, PromotionPieceType::kRook, MoveType::kPromotion));
                    moves.push_back(Move(from_sq, to_sq, PromotionPieceType::kQueen, MoveType::kPromotion));
                    move_count += 4;                
                }
            }
        }
    
        // ----- Capture Moves -----
        if (type != MoveGenType::kQuiet) {        
            // TODO: 
            // Here we have a possible improvement: when in check, only pawn captures are of the checking piece.
            // So at most 2. If not pinned otherwise.
            
            // Captures to the east.
            uint64_t to_east_bb = East(Forward(pawns, my_color)) & board.color_bitboards[enemy_color];
            to_east_bb &= capture_block_bb;

            for (uint64_t bb = to_east_bb; bb; bb &= ~LSB(bb)) {
                const Square to_sq = LSBSquare(bb);
                const uint64_t to_bb = SquareToBitboard(to_sq);
                const uint64_t from_bb = West(Backward(to_bb, my_color));
                const Square from_sq = BitboardToSquare(from_bb);

                const bool is_promotion = to_bb & kRankPromotion[my_color];

                // Pinned
                if (from_bb & precomputed_info.pin_info.pinned_bb) {
                    const uint64_t pin_line_bb = kLineBB[own_king_sq][from_sq];
                    if (pin_line_bb & to_bb) {
                        // Promotion
                        if (is_promotion) {
                            moves.push_back(Move(from_sq, to_sq, PromotionPieceType::kKnight, MoveType::kPromotion));
                            moves.push_back(Move(from_sq, to_sq, PromotionPieceType::kBishop, MoveType::kPromotion));
                            moves.push_back(Move(from_sq, to_sq, PromotionPieceType::kRook, MoveType::kPromotion));
                            moves.push_back(Move(from_sq, to_sq, PromotionPieceType::kQueen, MoveType::kPromotion));
                            move_count += 4;
                        }
                        // Non promotion
                        else {
                            moves.push_back(Move(from_sq, to_sq));
                            move_count++;
                        }
                    }
                }
                // Non Pinned
                else {
                    // Promotion
                    if (is_promotion) {
                        moves.push_back(Move(from_sq, to_sq, PromotionPieceType::kKnight, MoveType::kPromotion));
                        moves.push_back(Move(from_sq, to_sq, PromotionPieceType::kBishop, MoveType::kPromotion));
                        moves.push_back(Move(from_sq, to_sq, PromotionPieceType::kRook, MoveType::kPromotion));
                        moves.push_back(Move(from_sq, to_sq, PromotionPieceType::kQueen, MoveType::kPromotion));
                        move_count += 4;
                    }
                    // Non promotion
                    else {
                        moves.push_back(Move(from_sq, to_sq));
                        move_count++;
                    }
                }
            }

            // Captures to the west.
            uint64_t to_west_bb = West(Forward(pawns, my_color)) & board.color_bitboards[enemy_color];
            to_west_bb &= capture_block_bb;

            for (uint64_t bb = to_west_bb; bb; bb &= ~LSB(bb)) {
                const Square to_sq = LSBSquare(bb);
                const uint64_t to_bb = SquareToBitboard(to_sq);
                const uint64_t from_bb = East(Backward(to_bb, my_color));
                const Square from_sq = BitboardToSquare(from_bb);

                const bool is_promotion = to_bb & kRankPromotion[my_color];

                // Pinned
                if (from_bb & precomputed_info.pin_info.pinned_bb) {
                    const uint64_t pin_line_bb = kLineBB[own_king_sq][from_sq];
                    if (pin_line_bb & to_bb) {
                        // Promotion
                        if (is_promotion) {
                            moves.push_back(Move(from_sq, to_sq, PromotionPieceType::kKnight, MoveType::kPromotion));
                            moves.push_back(Move(from_sq, to_sq, PromotionPieceType::kBishop, MoveType::kPromotion));
                            moves.push_back(Move(from_sq, to_sq, PromotionPieceType::kRook, MoveType::kPromotion));
                            moves.push_back(Move(from_sq, to_sq, PromotionPieceType::kQueen, MoveType::kPromotion));
                            move_count += 4;
                        }
                        // Non promotion
                        else {
                            moves.push_back(Move(from_sq, to_sq));
                            move_count++;
                        }
                    }
                }
                // Non Pinned
                else {
                    // Promotion
                    if (is_promotion) {
                        moves.push_back(Move(from_sq, to_sq, PromotionPieceType::kKnight, MoveType::kPromotion));
                        moves.push_back(Move(from_sq, to_sq, PromotionPieceType::kBishop, MoveType::kPromotion));
                        moves.push_back(Move(from_sq, to_sq, PromotionPieceType::kRook, MoveType::kPromotion));
                        moves.push_back(Move(from_sq, to_sq, PromotionPieceType::kQueen, MoveType::kPromotion));
                        move_count += 4;
                    }
                    // Non promotion
                    else {
                        moves.push_back(Move(from_sq, to_sq));
                        move_count++;
                    }
                }
            }

            // En passant
            if (board.en_passant) {
                uint64_t en_passant_takers = pawns & (Backward(West(board.en_passant), my_color) | Backward(East(board.en_passant), my_color));

                while (en_passant_takers) {
                    const uint64_t from_bb = LSB(en_passant_takers);
                    const uint64_t to_bb = board.en_passant;
                    const uint64_t captured_bb = Backward(to_bb, my_color);

                    // We check explicitly if making the move leaves the king in check, not making use of pins bb.
                    uint64_t relevant_blockers = ~board.piece_bitboards[Piece::kEmpty];
                    relevant_blockers &= ~from_bb & ~captured_bb;
                    relevant_blockers |= to_bb;

                    const uint64_t straight_attacks = RookAttackBB(own_king_sq, relevant_blockers);
                    const uint64_t diagonal_attacks = BishopAttackBB(own_king_sq, relevant_blockers);

                    const uint64_t enemy_pawns = board.piece_bitboards[Piece::kWhitePawn + 6 * enemy_color];
                    const uint64_t enemy_knights = board.piece_bitboards[Piece::kWhiteKnight + 6 * enemy_color];
                    const uint64_t enemy_rooks_queens = board.piece_bitboards[Piece::kWhiteRook + 6 * enemy_color] 
                        | board.piece_bitboards[Piece::kWhiteQueen + 6 * enemy_color];
                    const uint64_t enemy_bishops_queens = board.piece_bitboards[Piece::kWhiteBishop + 6 * enemy_color] 
                        | board.piece_bitboards[Piece::kWhiteQueen + 6 * enemy_color];

                    const bool straight_check = straight_attacks & (enemy_rooks_queens & ~captured_bb);
                    const bool diagonal_check = diagonal_attacks & (enemy_bishops_queens & ~captured_bb);
                    const bool pawn_check = PawnAttackBB(own_king_sq, my_color) & (enemy_pawns & ~captured_bb);
                    const bool knight_check = kKnightAttacksBB[own_king_sq] & (enemy_knights & ~captured_bb);
                    const bool leaves_in_check = straight_check || diagonal_check || pawn_check || knight_check;

                    if (!leaves_in_check) {
                        const Square from_sq = BitboardToSquare(from_bb);
                        const Square to_sq = BitboardToSquare(to_bb);

                        moves.push_back(Move(from_sq, to_sq, PromotionPieceType::kKnight, MoveType::kEnPassant));
                        move_count++;
                    }

                    en_passant_takers &= ~from_bb;
                }
            }
        }

        return move_count;
    }

    template <MoveGenType type>
    size_t GenerateKnightMoves(Board &board, std::vector<Move> &moves, const MoveGenInfo &precomputed_info) {
        size_t move_count = 0;

        // Helpers
        const Color my_color = board.turn;
        const Color enemy_color = OppositeColor(my_color);
        const uint64_t own_king = board.piece_bitboards[kWhiteKing + 6*my_color];
        const Square own_king_sq = BitboardToSquare(own_king);
        const size_t num_checkers = SetBitsCount(precomputed_info.checkers_bb);

        // Only king evasions are legal when in double check.
        if (num_checkers >= 2) 
            return 0;

        // Candidate destination square.
        uint64_t candidate_to_bb = 0ull;
        if constexpr (type == MoveGenType::kAll)
            candidate_to_bb = ~board.color_bitboards[my_color];
        else if constexpr (type == MoveGenType::kQuiet)
            candidate_to_bb = board.piece_bitboards[Piece::kEmpty];
        else if constexpr (type == MoveGenType::kTactical || type == MoveGenType::kCapture)
            candidate_to_bb = board.color_bitboards[enemy_color];

        // Leave as candidate destination squares only ones that get you out of check.
        uint64_t capture_block_bb = ~0ull;
        if (num_checkers == 1) {
            capture_block_bb = 0ull;
            
            // Capture the checking piece.
            if constexpr (type != MoveGenType::kQuiet)
                capture_block_bb |= precomputed_info.checkers_bb;

            // Block the check.
            if constexpr (type == MoveGenType::kAll || type == MoveGenType::kQuiet) {
                if (precomputed_info.checkers_bb &
                    ( board.piece_bitboards[kWhiteBishop + 6*enemy_color]
                    | board.piece_bitboards[kWhiteRook + 6*enemy_color]
                    | board.piece_bitboards[kWhiteQueen + 6*enemy_color])
                ) {
                    capture_block_bb |= kSegmentBB[BitboardToSquare(precomputed_info.checkers_bb)][own_king_sq];
                }
            }
        }
        candidate_to_bb &= capture_block_bb;

        // Non pinned knight moves
        for (uint64_t bb = board.piece_bitboards[Piece::kWhiteKnight + 6*my_color] & ~precomputed_info.pin_info.pinned_bb; bb; bb &= ~LSB(bb)) {\
            const Square sq = LSBSquare(bb);
            uint64_t to_bb = kKnightAttacksBB[sq] & candidate_to_bb;

            while (to_bb) {
                uint64_t to_sq_bb = LSB(to_bb);
                const Square to_sq = BitboardToSquare(to_sq_bb);

                moves.push_back(Move(sq, to_sq));
                move_count++;

                to_bb &= ~to_sq_bb;
            }
        }

        // Pinned knights cannot move, so done.
        return move_count;
    }

    template<MoveGenType type>
    size_t GenerateBishopMoves(Board &board, std::vector<Move> &moves, const MoveGenInfo &precomputed_info) {
        size_t move_count = 0;

        // Helpers
        const Color my_color = board.turn;
        const Color enemy_color = OppositeColor(my_color);
        const uint64_t own_king = board.piece_bitboards[kWhiteKing + 6*my_color];
        const Square own_king_sq = BitboardToSquare(own_king);
        const size_t num_checkers = SetBitsCount(precomputed_info.checkers_bb);

        // Only king evasions are legal when in double check.
        if (num_checkers >= 2) 
            return 0;

        // Candidate destination square.
        uint64_t candidate_to_bb = 0ull;
        if constexpr (type == MoveGenType::kAll)
            candidate_to_bb = ~board.color_bitboards[my_color];
        else if constexpr (type == MoveGenType::kQuiet)
            candidate_to_bb = board.piece_bitboards[Piece::kEmpty];
        else if constexpr (type == MoveGenType::kTactical || type == MoveGenType::kCapture)
            candidate_to_bb = board.color_bitboards[enemy_color];

        // Leave as candidate destination squares only ones that get you out of check.
        uint64_t capture_block_bb = ~0ull;
        if (num_checkers == 1) {
            capture_block_bb = 0ull;

            // Capture the checking piece.
            if constexpr (type != MoveGenType::kQuiet)
                capture_block_bb |= precomputed_info.checkers_bb;

            // Block the check.
            if constexpr (type == MoveGenType::kAll || type == MoveGenType::kQuiet) {
                if (precomputed_info.checkers_bb &
                    ( board.piece_bitboards[kWhiteBishop + 6*enemy_color]
                    | board.piece_bitboards[kWhiteRook + 6*enemy_color]
                    | board.piece_bitboards[kWhiteQueen + 6*enemy_color])
                ) {
                    capture_block_bb |= kSegmentBB[BitboardToSquare(precomputed_info.checkers_bb)][own_king_sq];
                }
            }
        }
        candidate_to_bb &= capture_block_bb;
        
        const uint64_t blockers = ~board.piece_bitboards[Piece::kEmpty];

        // Non pinned bishop moves.
        for (uint64_t bb = board.piece_bitboards[Piece::kWhiteBishop + 6*my_color] & ~precomputed_info.pin_info.pinned_bb; bb; bb &= ~LSB(bb)) {
            const Square sq = LSBSquare(bb);
            uint64_t to_bb = BishopAttackBB(sq, blockers) & candidate_to_bb;

            while (to_bb) {
                uint64_t to_sq_bb = LSB(to_bb);
                const Square to_sq = BitboardToSquare(to_sq_bb);

                moves.push_back(Move(sq, to_sq));
                move_count++;

                to_bb &= ~to_sq_bb;
            }
        }

        // Pinned bishop moves.
        // Pinned bishops may move only if their king is not in check.
        if (num_checkers == 0) {
            for (uint64_t bb = board.piece_bitboards[Piece::kWhiteBishop + 6*my_color] & precomputed_info.pin_info.pinned_bb; bb; bb &= ~LSB(bb)) {
                const Square sq = LSBSquare(bb);
                const uint64_t pin_line_bb = kLineBB[own_king_sq][sq]; 
                uint64_t to_bb = BishopAttackBB(sq, blockers) & candidate_to_bb & pin_line_bb;

                while (to_bb) {
                    uint64_t to_sq_bb = LSB(to_bb);
                    const Square to_sq = BitboardToSquare(to_sq_bb);

                    moves.push_back(Move(sq, to_sq));
                    move_count++;

                    to_bb &= ~to_sq_bb;
                }
            }
        }

        return move_count;
    }

    template <MoveGenType type>
    size_t GenerateRookMoves(Board &board, std::vector<Move> &moves, const MoveGenInfo &precomputed_info) {
        size_t move_count = 0;

        // Helpers
        const Color my_color = board.turn;
        const Color enemy_color = OppositeColor(my_color);
        const uint64_t own_king = board.piece_bitboards[kWhiteKing + 6*my_color];
        const Square own_king_sq = BitboardToSquare(own_king);
        const size_t num_checkers = SetBitsCount(precomputed_info.checkers_bb);

        // Only king evasions are legal when in double check.
        if (num_checkers >= 2) 
            return 0;

        // Candidate destination square.
        uint64_t candidate_to_bb = 0ull;
        if constexpr (type == MoveGenType::kAll)
            candidate_to_bb = ~board.color_bitboards[my_color];
        else if constexpr (type == MoveGenType::kQuiet)
            candidate_to_bb = board.piece_bitboards[Piece::kEmpty];
        else if constexpr (type == MoveGenType::kTactical || type == MoveGenType::kCapture)
            candidate_to_bb = board.color_bitboards[enemy_color];

        // Leave as candidate destination squares only ones that get you out of check.
        uint64_t capture_block_bb = ~0ull;
        if (num_checkers == 1) {
            capture_block_bb = 0ull;

            // Capture the checking piece.
            if constexpr (type != MoveGenType::kQuiet)
                capture_block_bb |= precomputed_info.checkers_bb;

            // Block the check.
            if constexpr (type == MoveGenType::kAll || type == MoveGenType::kQuiet) {
                if (precomputed_info.checkers_bb &
                    ( board.piece_bitboards[kWhiteBishop + 6*enemy_color]
                    | board.piece_bitboards[kWhiteRook + 6*enemy_color]
                    | board.piece_bitboards[kWhiteQueen + 6*enemy_color])
                ) {
                    capture_block_bb |= kSegmentBB[BitboardToSquare(precomputed_info.checkers_bb)][own_king_sq];
                }
            }
        }
        candidate_to_bb &= capture_block_bb;
        
        const uint64_t blockers = ~board.piece_bitboards[Piece::kEmpty];

        // Non pinned bishop moves.
        for (uint64_t bb = board.piece_bitboards[Piece::kWhiteRook + 6*my_color] & ~precomputed_info.pin_info.pinned_bb; bb; bb &= ~LSB(bb)) {
            const Square sq = LSBSquare(bb);
            uint64_t to_bb = RookAttackBB(sq, blockers) & candidate_to_bb;

            while (to_bb) {
                uint64_t to_sq_bb = LSB(to_bb);
                const Square to_sq = BitboardToSquare(to_sq_bb);

                moves.push_back(Move(sq, to_sq));
                move_count++;

                to_bb &= ~to_sq_bb;
            }
        }

        // Pinned bishop moves.
        // Pinned bishops may move only if their king is not in check.
        if (num_checkers == 0) {
            for (uint64_t bb = board.piece_bitboards[Piece::kWhiteRook + 6*my_color] & precomputed_info.pin_info.pinned_bb; bb; bb &= ~LSB(bb)) {
                const Square sq = LSBSquare(bb);
                const uint64_t pin_line_bb = kLineBB[own_king_sq][sq]; 
                uint64_t to_bb = RookAttackBB(sq, blockers) & candidate_to_bb & pin_line_bb;

                while (to_bb) {
                    uint64_t to_sq_bb = LSB(to_bb);
                    const Square to_sq = BitboardToSquare(to_sq_bb);

                    moves.push_back(Move(sq, to_sq));
                    move_count++;

                    to_bb &= ~to_sq_bb;
                }
            }
        }

        return move_count;
    }

    template<MoveGenType type>
    size_t GenerateQueenMoves(Board &board, std::vector<Move> &moves,const MoveGenInfo &precomputed_info) {
        size_t move_count = 0;

        // Helpers
        const Color my_color = board.turn;
        const Color enemy_color = OppositeColor(my_color);
        const uint64_t own_king = board.piece_bitboards[kWhiteKing + 6*my_color];
        const Square own_king_sq = BitboardToSquare(own_king);
        const size_t num_checkers = SetBitsCount(precomputed_info.checkers_bb);

        // Only king evasions are legal when in double check.
        if (num_checkers >= 2) 
            return 0;

        // Candidate destination square.
        uint64_t candidate_to_bb = 0ull;
        if constexpr (type == MoveGenType::kAll)
            candidate_to_bb = ~board.color_bitboards[my_color];
        else if constexpr (type == MoveGenType::kQuiet)
            candidate_to_bb = board.piece_bitboards[Piece::kEmpty];
        else if constexpr (type == MoveGenType::kTactical || type == MoveGenType::kCapture)
            candidate_to_bb = board.color_bitboards[enemy_color];

        // Leave as candidate destination squares only ones that get you out of check.
        uint64_t capture_block_bb = ~0ull;
        if (num_checkers == 1) {
            capture_block_bb = 0ull;

            // Capture the checking piece.
            if constexpr (type != MoveGenType::kQuiet)
                capture_block_bb |= precomputed_info.checkers_bb;

            // Block the check.
            if constexpr (type == MoveGenType::kAll || type == MoveGenType::kQuiet) {
                if (precomputed_info.checkers_bb &
                    ( board.piece_bitboards[kWhiteBishop + 6*enemy_color]
                    | board.piece_bitboards[kWhiteRook + 6*enemy_color]
                    | board.piece_bitboards[kWhiteQueen + 6*enemy_color])
                ) {
                    capture_block_bb |= kSegmentBB[BitboardToSquare(precomputed_info.checkers_bb)][own_king_sq];
                }
            }
        }
        candidate_to_bb &= capture_block_bb;
        
        const uint64_t blockers = ~board.piece_bitboards[Piece::kEmpty];

        // Non pinned bishop moves.
        for (uint64_t bb = board.piece_bitboards[Piece::kWhiteQueen + 6*my_color] & ~precomputed_info.pin_info.pinned_bb; bb; bb &= ~LSB(bb)) {
            const Square sq = LSBSquare(bb);
            uint64_t to_bb = QueenAttackBB(sq, blockers) & candidate_to_bb;

            while (to_bb) {
                uint64_t to_sq_bb = LSB(to_bb);
                const Square to_sq = BitboardToSquare(to_sq_bb);

                moves.push_back(Move(sq, to_sq));
                move_count++;

                to_bb &= ~to_sq_bb;
            }
        }

        // Pinned bishop moves.
        // Pinned bishops may move only if their king is not in check.
        if (num_checkers == 0) {
            for (uint64_t bb = board.piece_bitboards[Piece::kWhiteQueen + 6*my_color] & precomputed_info.pin_info.pinned_bb; bb; bb &= ~LSB(bb)) {
                const Square sq = LSBSquare(bb);
                const uint64_t pin_line_bb = kLineBB[own_king_sq][sq]; 
                uint64_t to_bb = QueenAttackBB(sq, blockers) & candidate_to_bb & pin_line_bb;

                while (to_bb) {
                    uint64_t to_sq_bb = LSB(to_bb);
                    const Square to_sq = BitboardToSquare(to_sq_bb);

                    moves.push_back(Move(sq, to_sq));
                    move_count++;

                    to_bb &= ~to_sq_bb;
                }
            }
        }

        return move_count;
    }

    template<MoveGenType type>
    size_t GenerateKingMoves(Board &board, std::vector<Move> &moves, const MoveGenInfo &precomputed_info) {
        size_t move_count = 0;

        // Helpers
        const Color my_color = board.turn;
        const Color enemy_color = OppositeColor(my_color);
        const uint64_t own_king = board.piece_bitboards[kWhiteKing + 6*my_color];
        const Square own_king_sq = BitboardToSquare(own_king);

        const uint64_t blockers = board.color_bitboards[Color::kWhite] | board.color_bitboards[Color::kBlack];
        // We must remove our king from blockers so that, among the squares defended by enemy pieces are
        // xrays through the king to avoid the king moving from check to check
        // Q . . k . -> Q . . . k should be invalid
        const uint64_t enemy_def_bb = board.DefendedBB(enemy_color, blockers & ~own_king);
        
        // Simple king moves.
        uint64_t to_bb = kKingAttacksBB[own_king_sq] & ~board.color_bitboards[my_color] & ~enemy_def_bb;
        if constexpr (type == MoveGenType::kCapture || type == MoveGenType::kTactical)
            to_bb &= board.color_bitboards[enemy_color];
        if constexpr (type == MoveGenType::kQuiet)
            to_bb &= board.piece_bitboards[Piece::kEmpty];

        while (to_bb) {
            uint64_t to_sq_bb = LSB(to_bb);
            Square to_sq = BitboardToSquare(to_sq_bb);

            moves.push_back(Move(own_king_sq, to_sq));
            move_count++;

            to_bb &= ~to_sq_bb;
        }

        // Castles
        if constexpr (type == MoveGenType::kQuiet || type == MoveGenType::kAll) {
            uint8_t relevant_castles = board.castling & kCastlesByColor[my_color];

            while (relevant_castles) {
                uint8_t current_castle = LSB(relevant_castles);
                    
                // Check the needed squares are empty.
                if (kCastleInfo[current_castle].needed_empty & blockers) {
                    relevant_castles &= ~current_castle;
                    continue;
                }

                // Check the square that need to be safe are safe.
                if (kCastleInfo[current_castle].needed_safe & enemy_def_bb) {
                    relevant_castles &= ~current_castle;
                    continue;
                }

                // We trust that the board.castles flag ensures the king / rook have not been moved (or captured).
                // As such, this castle is legal.
                Square origin_sq = BitboardToSquare(kCastleInfo[current_castle].king_origin);
                Square dest_sq = BitboardToSquare(kCastleInfo[current_castle].king_destination);
                moves.push_back(Move(origin_sq, dest_sq, PromotionPieceType::kKnight, MoveType::kCastling));
                move_count++;

                // Pop LSB
                relevant_castles &= ~current_castle;
            }    
        }

        return move_count;
    }

    template<MoveGenType type>
    size_t GenerateMoves(Board &board, std::vector<Move> &moves) {
        size_t moves_count = 0;
        const MoveGenInfo precomputed_info = GetMoveGenInfo(board);

        moves_count += GeneratePawnMoves<type>(board, moves, precomputed_info);
        moves_count += GenerateKnightMoves<type>(board, moves, precomputed_info);
        moves_count += GenerateBishopMoves<type>(board, moves, precomputed_info);
        moves_count += GenerateRookMoves<type>(board, moves, precomputed_info);
        moves_count += GenerateQueenMoves<type>(board, moves, precomputed_info);
        moves_count += GenerateKingMoves<type>(board, moves, precomputed_info);
        
        return moves_count;
    }
} // namespace lightknight::movegen

#endif // LIGHTKNIGHT_MOVEGEN_H