#include "movegen.h"
#include "types.h"
#include <iostream>
#include <vector>


namespace lightknight::movegen {
    uint64_t PawnAttackBitboard(Square sq, Color color) {
        return kPawnAttacks[color * kNumSquares + sq];
    }

    uint64_t BishopAttackBitboard(Square sq, uint64_t blockers_bitboard) {
        uint64_t relevant_blockers = blockers_bitboard & kBishopRelevantOccupancy[sq];
        uint64_t magics_square_hash = (relevant_blockers * kBishopMagics[sq]) >> kBishopMagicShift;
        uint64_t magics_hash = sq * (1ULL << (64 - kBishopMagicShift)) + magics_square_hash;
        
        return kBishopAttacks[magics_hash];
    }

    uint64_t RookAttackBitboard(Square sq, uint64_t blockers_bitboard) {
        uint64_t relevant_blockers = blockers_bitboard & kRookRelevantOccupancy[sq];
        uint64_t magics_square_hash = (relevant_blockers * kRookMagics[sq]) >> kRookMagicShift;
        uint64_t magics_hash = sq * (1ULL << (64 - kRookMagicShift)) + magics_square_hash;
        
        return kRookAttacks[magics_hash];
    }

    uint64_t QueenAttackBitboard(Square sq, uint64_t blockers_bitboard) {
        return BishopAttackBitboard(sq, blockers_bitboard) | RookAttackBitboard(sq, blockers_bitboard);
    }

    size_t GeneratePawnMoves(Board &board, std::vector<Move> &moves) {
        Color my_color = board.turn;
        Color opposite_color = (Color)(1 - my_color);
        
        uint64_t pawns_bb = board.piece_bitboards[Piece::kWhitePawn + 6 * my_color];

        // For checking if a move leaves you in check
        uint64_t blockers = (board.color_bitboards[0] | board.color_bitboards[1]);
        
        uint64_t king_bb = board.piece_bitboards[Piece::kWhiteKing + 6 * my_color];
        Square king_sq = BitboardToSquare(king_bb); 
        
        // Itterate through the pawns.
        size_t new_moves_count = 0;
        while (pawns_bb) {
            uint64_t pawn_bb = LSB(pawns_bb);

            // Push forward
            uint64_t dest_bb = Forward(pawn_bb, my_color);
            if (board.piece_bitboards[Piece::kEmpty] & dest_bb) {
                uint64_t relevant_blockers = (blockers & ~pawn_bb) | dest_bb;
                uint64_t straights = RookAttackBitboard(king_sq, relevant_blockers) & ~dest_bb;
                uint64_t diagonals = BishopAttackBitboard(king_sq, relevant_blockers) & ~dest_bb;
                
                bool check_on_straights = straights & (board.piece_bitboards[Piece::kWhiteRook + 6*opposite_color] | board.piece_bitboards[Piece::kWhiteQueen + 6*opposite_color]);
                bool check_on_diagonals = diagonals & (board.piece_bitboards[Piece::kWhiteBishop + 6*opposite_color] | board.piece_bitboards[Piece::kWhiteQueen + 6*opposite_color]);
                bool pawn_check = (PawnAttackBitboard(BitboardToSquare(king_bb), my_color) & ~dest_bb) & board.piece_bitboards[Piece::kWhitePawn + 6*opposite_color];
                bool knight_check = (kKnightAttacks[BitboardToSquare(king_bb)] & ~dest_bb) & board.piece_bitboards[Piece::kWhiteKnight + 6*opposite_color];

                // Legal move.
                if (!(check_on_diagonals || check_on_straights || pawn_check || knight_check)) {
                    if (!(check_on_straights || check_on_diagonals)) {
                        // Promotion.
                        if (dest_bb & kRankPromotion[my_color]) {
                            moves.push_back(Move(BitboardToSquare(pawn_bb), BitboardToSquare(dest_bb), PromotionPieceType::kQueen, MoveType::kPromotion));
                            moves.push_back(Move(BitboardToSquare(pawn_bb), BitboardToSquare(dest_bb), PromotionPieceType::kRook, MoveType::kPromotion));
                            moves.push_back(Move(BitboardToSquare(pawn_bb), BitboardToSquare(dest_bb), PromotionPieceType::kBishop, MoveType::kPromotion));
                            moves.push_back(Move(BitboardToSquare(pawn_bb), BitboardToSquare(dest_bb), PromotionPieceType::kKnight, MoveType::kPromotion));
                            new_moves_count += 4;
                        }
                        // Normal
                        else {
                            moves.push_back(Move(BitboardToSquare(pawn_bb), BitboardToSquare(dest_bb)));
                            new_moves_count++;
                        }
                    }
                }
            }

            // Double Pushes
            dest_bb = Forward(Forward(pawn_bb, my_color), my_color);
            if ((pawn_bb & kRankPawnDoublePush[my_color]) && 
                (board.piece_bitboards[Piece::kEmpty] & Forward(pawn_bb, my_color)) &&
                (board.piece_bitboards[Piece::kEmpty] & dest_bb)) 
            {
                // Check if the move leaves you in check.
                uint64_t relevant_blockers = (blockers & ~pawn_bb) | dest_bb;
                uint64_t straights = RookAttackBitboard(king_sq, relevant_blockers) & ~dest_bb;
                uint64_t diagonals = BishopAttackBitboard(king_sq, relevant_blockers) & ~dest_bb;
                
                bool check_on_straights = straights & (board.piece_bitboards[Piece::kWhiteRook + 6*opposite_color] | board.piece_bitboards[Piece::kWhiteQueen + 6*opposite_color]);
                bool check_on_diagonals = diagonals & (board.piece_bitboards[Piece::kWhiteBishop + 6*opposite_color] | board.piece_bitboards[Piece::kWhiteQueen + 6*opposite_color]);
                bool pawn_check = (PawnAttackBitboard(BitboardToSquare(king_bb), my_color) & ~dest_bb) & board.piece_bitboards[Piece::kWhitePawn + 6*opposite_color];
                bool knight_check = (kKnightAttacks[BitboardToSquare(king_bb)] & ~dest_bb) & board.piece_bitboards[Piece::kWhiteKnight + 6*opposite_color];

                // Legal move.
                if (!(check_on_straights || check_on_diagonals || pawn_check || knight_check)) {
                    moves.push_back(Move(BitboardToSquare(pawn_bb), BitboardToSquare(dest_bb)));
                    new_moves_count++;
                }
            }

            // Captures
            uint64_t pawn_attacks = PawnAttackBitboard(BitboardToSquare(pawn_bb), my_color) & board.color_bitboards[opposite_color];
            // Itterate through the (at most 2) possible captures
            while (pawn_attacks) {
                uint64_t dest_bb = LSB(pawn_attacks);

                // Check if the move leaves you in check.
                uint64_t relevant_blockers = (blockers & ~pawn_bb) | dest_bb;
                uint64_t straights = RookAttackBitboard(king_sq, relevant_blockers) & ~dest_bb;
                uint64_t diagonals = BishopAttackBitboard(king_sq, relevant_blockers) & ~dest_bb;
                
                bool check_on_straights = straights & (board.piece_bitboards[Piece::kWhiteRook + 6*opposite_color] | board.piece_bitboards[Piece::kWhiteQueen + 6*opposite_color]);
                bool check_on_diagonals = diagonals & (board.piece_bitboards[Piece::kWhiteBishop + 6*opposite_color] | board.piece_bitboards[Piece::kWhiteQueen + 6*opposite_color]);
                bool pawn_check = (PawnAttackBitboard(BitboardToSquare(king_bb), my_color) & ~dest_bb) & board.piece_bitboards[Piece::kWhitePawn + 6*opposite_color];
                bool knight_check = (kKnightAttacks[BitboardToSquare(king_bb)] & ~dest_bb) & board.piece_bitboards[Piece::kWhiteKnight + 6*opposite_color];

                if (check_on_diagonals || check_on_straights || pawn_check || knight_check) {
                    pawn_attacks &= ~dest_bb;
                    continue;
                }

                // Legal move.
                if (!(check_on_straights || check_on_diagonals)) {
                    // Promotion.
                    if (dest_bb & kRankPromotion[my_color]) {
                        moves.push_back(Move(BitboardToSquare(pawn_bb), BitboardToSquare(dest_bb), PromotionPieceType::kQueen, MoveType::kPromotion));
                        moves.push_back(Move(BitboardToSquare(pawn_bb), BitboardToSquare(dest_bb), PromotionPieceType::kRook, MoveType::kPromotion));
                        moves.push_back(Move(BitboardToSquare(pawn_bb), BitboardToSquare(dest_bb), PromotionPieceType::kBishop, MoveType::kPromotion));
                        moves.push_back(Move(BitboardToSquare(pawn_bb), BitboardToSquare(dest_bb), PromotionPieceType::kKnight, MoveType::kPromotion));
                        new_moves_count += 4;
                    }
                    // Normal.
                    else {
                        moves.push_back(Move(BitboardToSquare(pawn_bb), BitboardToSquare(dest_bb)));
                        new_moves_count++;
                    }
                }

                // Pop LSB
                pawn_attacks &= ~dest_bb;
            }

            // Pop LSB
            pawns_bb &= ~pawn_bb;
        }

        // En Passant
        if (board.en_passant) {
            // Pawns that can take en passant.
            uint64_t en_passant_takers_bb = board.piece_bitboards[Piece::kWhitePawn + 6 * my_color] &
                (Backward(West(board.en_passant), my_color) | (Backward(East(board.en_passant), my_color)));
                                           
            // Itterate through them.
            while (en_passant_takers_bb) {
                uint64_t origin_bb = LSB(en_passant_takers_bb);
                uint64_t dest_bb = board.en_passant;
                uint64_t taken_bb = Backward(dest_bb, my_color);

                // Check if this move leaves you in check.
                uint64_t relevant_blockers = (blockers & ~origin_bb & ~taken_bb) | dest_bb;
                uint64_t straights = RookAttackBitboard(king_sq, relevant_blockers);
                uint64_t diagonals = BishopAttackBitboard(king_sq, relevant_blockers);
                
                bool check_on_straights = straights & (board.piece_bitboards[Piece::kWhiteRook + 6*opposite_color] | board.piece_bitboards[Piece::kWhiteQueen + 6*opposite_color]);
                bool check_on_diagonals = diagonals & (board.piece_bitboards[Piece::kWhiteBishop + 6*opposite_color] | board.piece_bitboards[Piece::kWhiteQueen + 6*opposite_color]);
                bool pawn_check = (PawnAttackBitboard(king_sq, my_color) & ~taken_bb) & board.piece_bitboards[Piece::kWhitePawn + 6*opposite_color];
                bool knight_check = (kKnightAttacks[king_sq] & ~taken_bb) & board.piece_bitboards[Piece::kWhiteKnight + 6*opposite_color];

                // It's a legal move.
                if (!(check_on_straights || check_on_diagonals || pawn_check || knight_check)) {
                    moves.push_back(Move(BitboardToSquare(origin_bb), BitboardToSquare(dest_bb), PromotionPieceType::kKnight, MoveType::kEnPassant));
                    new_moves_count++;
                }

                // Pop LSB
                en_passant_takers_bb &= ~origin_bb;
            }
        }

        return new_moves_count;
    }

    size_t GenerateKnightMoves(Board &board, std::vector<Move> &moves) {
        Color my_color = board.turn;
        Color opposite_color = (Color)(1 - my_color);

        // For generating candidate moves for a knight.
        uint64_t knights_bb = board.piece_bitboards[Piece::kWhiteKnight + 6 * my_color];
        
        // For checking later if a move leaves you in check
        uint64_t blockers = (board.color_bitboards[0] | board.color_bitboards[1]);
        uint64_t king_bb = board.piece_bitboards[Piece::kWhiteKing + 6 * my_color];
        Square king_sq = BitboardToSquare(king_bb); 
        
        size_t new_moves_count = 0;

        // Itterate through all knight of that color.
        while (knights_bb) {
            uint64_t origin_bb = LSB(knights_bb);

            // Get the moves that don't capture one of your pieces.
            uint64_t attacks_bb = kKnightAttacks[BitboardToSquare(origin_bb)] & (board.color_bitboards[opposite_color] | board.piece_bitboards[Piece::kEmpty]);
        
            // Itterate through them
            while (attacks_bb) {
                uint64_t dest_bb = LSB(attacks_bb);

                uint64_t relevant_blockers = (blockers | dest_bb) & ~origin_bb;
                uint64_t straights = RookAttackBitboard(king_sq, relevant_blockers) & ~dest_bb;
                uint64_t diagonals = BishopAttackBitboard(king_sq, relevant_blockers) & ~dest_bb;

                bool check_on_straights = straights & (board.piece_bitboards[Piece::kWhiteRook + 6*opposite_color] | board.piece_bitboards[Piece::kWhiteQueen + 6*opposite_color]);
                bool check_on_diagonals = diagonals & (board.piece_bitboards[Piece::kWhiteBishop + 6*opposite_color] | board.piece_bitboards[Piece::kWhiteQueen + 6*opposite_color]);
                bool pawn_check = (PawnAttackBitboard(BitboardToSquare(king_bb), my_color) & ~dest_bb) & board.piece_bitboards[Piece::kWhitePawn + 6*opposite_color];
                bool knight_check = (kKnightAttacks[BitboardToSquare(king_bb)] & ~dest_bb) & board.piece_bitboards[Piece::kWhiteKnight + 6*opposite_color];

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

    size_t GenerateBishopMoves(Board &board, std::vector<Move> &moves) {
        Color my_color = board.turn;
        Color opposite_color = (Color)(1 - my_color);

        // For generating candidate moves for a bishop
        uint64_t bishops_bb = board.piece_bitboards[Piece::kWhiteBishop + 6 * my_color];
        uint64_t good_dests_bb = board.color_bitboards[opposite_color] | board.piece_bitboards[Piece::kEmpty];
        
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
            uint64_t attacks_bb = BishopAttackBitboard(BitboardToSquare(origin_bb), blockers);
            attacks_bb &= good_dests_bb;

            // Itterate through them
            while (attacks_bb) {
                uint64_t dest_bb = LSB(attacks_bb);

                // Make sure moving this bishop does not leave your king in check.
                uint64_t relevant_blockers = (blockers | dest_bb) & ~origin_bb;
                uint64_t straights = RookAttackBitboard(king_sq, relevant_blockers) & ~dest_bb;
                uint64_t diagonals = BishopAttackBitboard(king_sq, relevant_blockers) & ~dest_bb;
                
                bool check_on_straights = straights & (board.piece_bitboards[Piece::kWhiteRook + 6*opposite_color] | board.piece_bitboards[Piece::kWhiteQueen + 6*opposite_color]);
                bool check_on_diagonals = diagonals & (board.piece_bitboards[Piece::kWhiteBishop + 6*opposite_color] | board.piece_bitboards[Piece::kWhiteQueen + 6*opposite_color]);
                bool pawn_check = (PawnAttackBitboard(BitboardToSquare(king_bb), my_color) & ~dest_bb) & board.piece_bitboards[Piece::kWhitePawn + 6*opposite_color];
                bool knight_check = (kKnightAttacks[BitboardToSquare(king_bb)] & ~dest_bb) & board.piece_bitboards[Piece::kWhiteKnight + 6*opposite_color];

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

    size_t GenerateRookMoves(Board &board, std::vector<Move> &moves) {
        Color my_color = board.turn;
        Color opposite_color = (Color)(1 - my_color);

        // For generating candidate moves for a rook
        uint64_t rooks_bb = board.piece_bitboards[Piece::kWhiteRook + 6 * my_color];
        uint64_t good_dests_bb = board.color_bitboards[opposite_color] | board.piece_bitboards[Piece::kEmpty];
        
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
            uint64_t attacks_bb = RookAttackBitboard(BitboardToSquare(origin_bb), blockers);
            attacks_bb &= good_dests_bb;

            // Itterate through them
            while (attacks_bb) {
                uint64_t dest_bb = LSB(attacks_bb);

                // Make sure moving this bishop does not leave your king in check.
                uint64_t relevant_blockers = (blockers | dest_bb) & ~origin_bb;
                uint64_t straights = RookAttackBitboard(king_sq, relevant_blockers) & ~dest_bb;
                uint64_t diagonals = BishopAttackBitboard(king_sq, relevant_blockers) & ~dest_bb;
                
                bool check_on_straights = straights & (board.piece_bitboards[Piece::kWhiteRook + 6*opposite_color] | board.piece_bitboards[Piece::kWhiteQueen + 6*opposite_color]);
                bool check_on_diagonals = diagonals & (board.piece_bitboards[Piece::kWhiteBishop + 6*opposite_color] | board.piece_bitboards[Piece::kWhiteQueen + 6*opposite_color]);
                bool pawn_check = (PawnAttackBitboard(BitboardToSquare(king_bb), my_color) & ~dest_bb) & board.piece_bitboards[Piece::kWhitePawn + 6*opposite_color];
                bool knight_check = (kKnightAttacks[BitboardToSquare(king_bb)] & ~dest_bb) & board.piece_bitboards[Piece::kWhiteKnight + 6*opposite_color];

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

    size_t GenerateQueenMoves(Board &board, std::vector<Move> &moves) {
        Color my_color = board.turn;
        Color opposite_color = (Color)(1 - my_color);

        // For generating candidate moves for a queen.
        uint64_t queens_bb = board.piece_bitboards[Piece::kWhiteQueen + 6 * my_color];
        uint64_t good_dests_bb = board.color_bitboards[opposite_color] | board.piece_bitboards[Piece::kEmpty];
        
        // For checking later if a move leaves you in check.
        Square king_sq = BitboardToSquare(board.piece_bitboards[Piece::kWhiteKing + 6 * my_color]); 
        uint64_t blockers = (board.color_bitboards[0] | board.color_bitboards[1]);
                
        // Count number of new moves added.
        size_t new_moves_count = 0;

        // Itterate through all the bishops.
        while (queens_bb) {
            uint64_t origin_bb = LSB(queens_bb);

            // Get the possible destination squares for this move, that don't capture one of your pieces.
            uint64_t attacks_bb = QueenAttackBitboard(BitboardToSquare(origin_bb), blockers);
            attacks_bb &= good_dests_bb;

            // Itterate through them
            while (attacks_bb) {
                uint64_t dest_bb = LSB(attacks_bb);

                // Make sure moving this bishop does not leave your king in check.
                uint64_t relevant_blockers = (blockers | dest_bb) & ~origin_bb;
                uint64_t straights = RookAttackBitboard(king_sq, relevant_blockers) & ~dest_bb;
                uint64_t diagonals = BishopAttackBitboard(king_sq, relevant_blockers) & ~dest_bb;
                
                bool check_on_straights = straights & (board.piece_bitboards[Piece::kWhiteRook + 6*opposite_color] | board.piece_bitboards[Piece::kWhiteQueen + 6*opposite_color]);
                bool check_on_diagonals = diagonals & (board.piece_bitboards[Piece::kWhiteBishop + 6*opposite_color] | board.piece_bitboards[Piece::kWhiteQueen + 6*opposite_color]);
                bool pawn_check = (PawnAttackBitboard(king_sq, my_color) & ~dest_bb) & board.piece_bitboards[Piece::kWhitePawn + 6*opposite_color];
                bool knight_check = (kKnightAttacks[king_sq] & ~dest_bb) & board.piece_bitboards[Piece::kWhiteKnight + 6*opposite_color];

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

    size_t GenerateKingMoves(Board &board, std::vector<Move> &moves) {
        Color my_color = board.turn;
        Color opposite_color = (Color)(1 - my_color);
        Square king_sq = BitboardToSquare(board.piece_bitboards[Piece::kWhiteKing + 6 * my_color]);
        Square enemy_king_sq = BitboardToSquare(board.piece_bitboards[Piece::kWhiteKing + 6 * opposite_color]);
        uint64_t blockers = (board.color_bitboards[0] | board.color_bitboards[1]);
        
        // Get king destination squares that don't put the king near the enemy king or take its own pieces.
        uint64_t king_attacks_bb = kKingAttacks[king_sq] & ~kKingAttacks[enemy_king_sq];
        king_attacks_bb &= ~board.color_bitboards[my_color];

        // Count the numver of new moves.
        size_t new_moves_count = 0;

        // Itterate through them.
        while (king_attacks_bb) {
            uint64_t dest_bb = LSB(king_attacks_bb);

            // Check if this move leaves the king in check.
            uint64_t relevant_blockers = (blockers & ~SquareToBitboard(king_sq)) & ~dest_bb;
            uint64_t straights = RookAttackBitboard(BitboardToSquare(dest_bb), relevant_blockers);
            uint64_t diagonals = BishopAttackBitboard(BitboardToSquare(dest_bb), relevant_blockers); 

            bool check_on_staights = straights & (board.piece_bitboards[Piece::kWhiteRook + 6*opposite_color] | board.piece_bitboards[Piece::kWhiteQueen + 6*opposite_color]);
            bool check_on_diagonals = diagonals & (board.piece_bitboards[Piece::kWhiteBishop + 6*opposite_color] | board.piece_bitboards[Piece::kWhiteQueen + 6*opposite_color]);
            bool pawn_check = PawnAttackBitboard(BitboardToSquare(dest_bb), my_color) & board.piece_bitboards[Piece::kWhitePawn + 6*opposite_color];
            bool knight_check = kKnightAttacks[BitboardToSquare(dest_bb)] & board.piece_bitboards[Piece::kWhiteKnight + 6*opposite_color];

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
    
        return new_moves_count;
    }

    size_t GenerateMoves(Board &board, std::vector<Move> &moves) {
        size_t moves_count = 0;

        moves_count += GeneratePawnMoves(board, moves);
        moves_count += GenerateKnightMoves(board, moves);
        moves_count += GenerateBishopMoves(board, moves);
        moves_count += GenerateRookMoves(board, moves);
        moves_count += GenerateQueenMoves(board, moves);
        moves_count += GenerateKingMoves(board, moves);
        
        return moves_count;
    }
} // namespace lightknight::movegen