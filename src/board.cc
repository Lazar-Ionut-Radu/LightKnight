// board.cc
#include "board.h"
#include "exceptions.h"
#include "move_gen.h"
#include "zobrist.h"

#include <array>
#include <vector>
#include <string>
#include <cstdint>
#include <regex>
#include <iostream>
#include <unordered_map>
#include <algorithm>

namespace lightknight {
    void Board::FromFEN(const std::string& fen) {
        // Validate the format of a fen string
        std::regex fen_regex(
            R"(^(?:([PNBRQKpnbrqk1-8/]+)\s)"
            R"(([bw])\s)"
            R"((-|[KQkq]{1,4})\s)"
            R"((-|[a-h][36])\s)"
            R"((\d+)\s)"
            R"((\d+)$))"
        );

        std::smatch match;
        if (!std::regex_match(fen, match, fen_regex)) {
            throw exceptions::FENException("Invalid FEN string");
        }

        // Extract each section
        std::string placement_str  = match[1];
        std::string turn_str       = match[2];
        std::string castling_str   = match[3];
        std::string en_passant_str = match[4];
        std::string halfmove_str   = match[5];
        std::string fullmove_str   = match[6];

        // Setup the pieces on the board.
        static const std::unordered_map<char, Piece> char_to_piece = {
            {'P', Piece::kWhitePawn},   {'p', Piece::kBlackPawn},
            {'N', Piece::kWhiteKnight}, {'n', Piece::kBlackKnight},
            {'B', Piece::kWhiteBishop}, {'b', Piece::kBlackBishop},
            {'R', Piece::kWhiteRook},   {'r', Piece::kBlackRook},
            {'Q', Piece::kWhiteQueen},  {'q', Piece::kBlackQueen},
            {'K', Piece::kWhiteKing},   {'k', Piece::kBlackKing},
        };
        int file = 0, rank = 7;
        for (size_t i = 0; i < placement_str.size(); ++i) {
            // Go on the next rank
            if (placement_str[i] == '/') {
                file = 0;
                rank--;

                if (rank < 0)
                    throw exceptions::FENException("Invalid FEN string ops");
            }
            else {
                // Empty spaces
                if (placement_str[i] < 'A') {
                    for (int num = 1; num <= (int)(placement_str[i] - '0'); ++num) {
                        this->piece_bitboards[Piece::kEmpty] |= 1ULL << (8*rank + file);
                        file++;

                        if (file > 8)
                            throw exceptions::FENException("Invalid FEN string ops");
                    }
                }
                // Pieces
                else {
                    this->piece_bitboards[char_to_piece.at(placement_str[i])] |= 1ULL << (8*rank + file);
                    file++;

                    if (rank > 8)
                        throw exceptions::FENException("Invalid FEN string ops");
                }
            }
        }
        if (rank > 0)
            throw exceptions::FENException("Invalid FEN string ops");

        // Setup the turn
        if (turn_str[0] == 'w')
            this->turn = Color::kWhite;
        else
            this->turn = Color::kBlack;
        
        // Setup the castling rights
        if (castling_str[0] != '-') {
            for (char c : castling_str) {
                switch (c) {
                    case 'K':
                        this->castling |= Castle::kWhiteKingSide;
                        break;
                    case 'Q':
                        this->castling |= Castle::kWhiteQueenSide;
                        break;
                    case 'k':
                        this->castling |= Castle::kBlackKingSide;
                        break;
                    case 'q':
                        this->castling |= Castle::kBlackQueenSide;
                        break;
                }
            }
        }
        this->color_bitboards[Color::kWhite] |= piece_bitboards[0] | piece_bitboards[1] 
            | piece_bitboards[2] | piece_bitboards[3] | piece_bitboards[4] | piece_bitboards[5];

        this->color_bitboards[Color::kBlack] |= piece_bitboards[6] | piece_bitboards[7] 
            | piece_bitboards[8] | piece_bitboards[9] | piece_bitboards[10] | piece_bitboards[11];
        
        // Setup the en passant square
        if (en_passant_str[0] == '-')
            this->en_passant = 0ULL;
        else {
            int file = (int)(en_passant_str[0] - 'a');
            int rank = (int)(en_passant_str[1] - '1');
            this->en_passant = 1ULL << (8*rank+file);
        }

        // Setup the halfmove & fullmove clocks.
        this->halfmoves = this->fullmoves = 0;

        if (halfmove_str[0] != '-')
            for (size_t i = 0; i < halfmove_str.size(); ++i)
                this->halfmoves = 10 * this->halfmoves + (int)(halfmove_str[i] - '0');
                
        if (fullmove_str[0] != '-')
            for (size_t i = 0; i < fullmove_str.size(); ++i)
                this->fullmoves = 10 * this->fullmoves + (int)(fullmove_str[i] - '0');
        
        // Finally compute the zobrist hash.
        this->zobrist_hash = this->ComputeZobristHash();
        this->pawn_zobrist_hash = this->ComputePawnZobristHash();

        // Allocate mem for the position hashes history vector.
        this->hashes_history.clear();
        this->hashes_history.reserve(256);
        this->hashes_history.push_back(zobrist_hash);
    }

    Board Board::FromRaw(
        const std::array<uint64_t, kNumPieces>& piece_bitboards,
        Color turn,
        uint8_t castling,
        int en_passant_square,
        int halfmoves,
        int fullmoves
    ) {
        Board board;
        board.piece_bitboards = piece_bitboards;
        board.turn = turn;
        board.castling = castling;
        board.en_passant = (en_passant_square < 0) ? 0ULL : (1ULL << en_passant_square);
        board.halfmoves = halfmoves;
        board.fullmoves = fullmoves;

        board.color_bitboards = {0ULL};
        board.color_bitboards[Color::kWhite] |= piece_bitboards[Piece::kWhitePawn] | piece_bitboards[Piece::kWhiteKnight] | piece_bitboards[Piece::kWhiteBishop]
            | piece_bitboards[Piece::kWhiteRook] | piece_bitboards[Piece::kWhiteQueen] | piece_bitboards[Piece::kWhiteKing];
        board.color_bitboards[Color::kBlack] |= piece_bitboards[Piece::kBlackPawn] | piece_bitboards[Piece::kBlackKnight] | piece_bitboards[Piece::kBlackBishop]  
            | piece_bitboards[Piece::kBlackRook] | piece_bitboards[Piece::kBlackQueen] | piece_bitboards[Piece::kBlackKing];
        
        board.zobrist_hash = board.ComputeZobristHash();
        board.pawn_zobrist_hash = board.ComputePawnZobristHash();

        board.hashes_history.clear();
        board.hashes_history.reserve(256);
        board.hashes_history.push_back(board.zobrist_hash);

        return board;
    }

    Board::Board(const std::string& fen) : Board() {
        this->FromFEN(fen);
    }

    Board::Board() {
        this->piece_bitboards = {0ULL};
        this->color_bitboards = {0ULL};
        this->castling = 0;
        this->en_passant = 0ULL;
        this->halfmoves = 0;
        this->fullmoves = 1;
        this->turn = Color::kWhite;
        this->zobrist_hash = this->ComputeZobristHash();
        this->pawn_zobrist_hash = this->ComputePawnZobristHash();

        this->hashes_history.clear();
        this->hashes_history.reserve(256);
        this->hashes_history.push_back(this->zobrist_hash);
    }
    
    uint64_t Board::ComputeZobristHash() const {
        uint64_t hash = 0;

        // Pieces
        for (size_t piece = 0; piece < kNumPieces - 1; ++piece) {
            uint64_t pieces = this->piece_bitboards[piece];

            while (pieces != 0) {
                const unsigned square = std::countr_zero(pieces);
                hash ^= kZobrists.piece_square[piece][square];

                // Remove LSB
                pieces &= pieces - 1;
            }
        }

        // Turn is present when white is to move.
        if (turn == Color::kWhite)
            hash ^= kZobrists.turn;

        // Each combination of possible castles has its own hash.
        hash ^= kZobrists.castling[this->castling];

        // En passant
        if (en_passant != 0)
            hash ^= kZobrists.en_passant[BitboardToSquare(this->en_passant)];
    
        return hash;
    }

    uint64_t Board::ComputePawnZobristHash() const {
        uint64_t hash = 0;

        for (size_t pawn : {Piece::kWhitePawn, Piece::kBlackPawn}) {
            uint64_t pawns = this->piece_bitboards[pawn];

            while (pawns != 0) {
                const unsigned square = std::countr_zero(pawns);
                hash ^= kZobrists.piece_square[pawn][square];

                // Remove LSB
                pawns &= pawns - 1;
            }
        }
    
        // En passant
        if (en_passant != 0)
            hash ^= kZobrists.en_passant[BitboardToSquare(this->en_passant)];
    
        return hash;
    }

    uint64_t Board::DefendedBB(Color color, uint64_t blockers) const {
        const uint64_t own_pieces = color_bitboards[color];
        uint64_t defended = 0ull;

        // Pawns
        uint64_t pawns_forward = Forward(piece_bitboards[kWhitePawn + 6 * color], color); 
        uint64_t pawn_def = East(pawns_forward) | West(pawns_forward);
        defended |= pawn_def;

        // Knights
        for (uint64_t bb = piece_bitboards[Piece::kWhiteKnight + 6 * color]; bb; bb &= ~LSB(bb)) {
            const Square sq = LSBSquare(bb);
            defended |= kKnightAttacksBB[sq];
        }

        // Bishops
        for (uint64_t bb = piece_bitboards[Piece::kWhiteBishop + 6 * color]; bb; bb &= ~LSB(bb)) {
            const Square sq = LSBSquare(bb);
            defended |= BishopAttackBB(sq, blockers);
        }

        // Rooks
        for (uint64_t bb = piece_bitboards[Piece::kWhiteRook + 6 * color]; bb; bb &= ~LSB(bb)) {
            const Square sq = LSBSquare(bb);
            defended |= RookAttackBB(sq, blockers);
        }

        // Queens
        for (uint64_t bb = piece_bitboards[Piece::kWhiteQueen + 6 * color]; bb; bb &= ~LSB(bb)) {
            const Square sq = LSBSquare(bb);
            defended |= QueenAttackBB(sq, blockers);
        }

        // King
        const Square sq = BitboardToSquare(piece_bitboards[Piece::kWhiteKing + 6 * color]);
        defended |= kKingAttacksBB[sq];

        return defended;
    }

    // Returns a bitboard of squares that are defended by pieces of the given color.
    // En passant not considered.
    uint64_t Board::DefendedBB(Color color) const {
        return DefendedBB(color, color_bitboards[Color::kWhite] | color_bitboards[Color::kBlack]);
    }

    // Returns a bitboard of pieces that attack this square.
    // Does not consider en passant.
    uint64_t Board::AttackersBB(uint64_t square_bb, Color my_color) const {
        Color attacker_color = OppositeColor(my_color);
        Square sq = BitboardToSquare(square_bb);

        uint64_t attackers = 0ull;

        // Compute blockers.
        uint64_t blockers = this->color_bitboards[0] | this->color_bitboards[1];
    
        // Attacks on straights.
        uint64_t straights = RookAttackBB(sq, blockers);
        attackers |= straights & (this->piece_bitboards[Piece::kWhiteRook + 6 * attacker_color] | this->piece_bitboards[Piece::kWhiteQueen + 6 * attacker_color]);
    
        // Attacks on diagonals.
        uint64_t diagonals = BishopAttackBB(sq, blockers);
        attackers |= diagonals & (this->piece_bitboards[Piece::kWhiteBishop + 6 * attacker_color] | this->piece_bitboards[Piece::kWhiteQueen + 6 * attacker_color]);
    
        // Knight attacks.
        uint64_t knight_attacks = kKnightAttacksBB[sq];
        attackers |= knight_attacks & this->piece_bitboards[Piece::kWhiteKnight + 6 * attacker_color];
    
        // Pawn attacks.
        uint64_t pawn_attacks = PawnAttackBB(sq, my_color);
        attackers |= pawn_attacks & this->piece_bitboards[Piece::kWhitePawn + 6 * attacker_color];

        // King attacks.
        attackers |= kKingAttacksBB[sq] & this->piece_bitboards[Piece::kWhiteKing + 6 * attacker_color];

        return attackers;
    }

    // Checks if this square is attacked by a piece of the specified color.
    // Does not consider en passant.
    bool Board::IsSquareAttacked(uint64_t square_bb, Color my_color) const {
        Color attacker_color = (Color)(1 - my_color);
        Square sq = BitboardToSquare(square_bb);

        // Compute blockers.
        uint64_t blockers = this->color_bitboards[0] | this->color_bitboards[1];
       
        // Attacks on straights.
        uint64_t straights = RookAttackBB(sq, blockers);
        if (straights & (this->piece_bitboards[Piece::kWhiteRook + 6*attacker_color] | this->piece_bitboards[Piece::kWhiteQueen + 6*attacker_color]))
            return true;
        
        // Attacks on diagonals.
        uint64_t diagonals = BishopAttackBB(sq, blockers);
        if (diagonals & (this->piece_bitboards[Piece::kWhiteBishop + 6*attacker_color] | this->piece_bitboards[Piece::kWhiteQueen + 6*attacker_color]))
            return true;
        
        // Knight attacks.
        uint64_t knights_attacks = kKnightAttacksBB[sq];
        if (knights_attacks & this->piece_bitboards[Piece::kWhiteKnight + 6*attacker_color])
            return true;

        // Pawn attacks.
        uint64_t pawn_attacks = PawnAttackBB(sq, my_color);
        if ( pawn_attacks & this->piece_bitboards[Piece::kWhitePawn + 6*attacker_color])
            return true;

        // King attacks.
        uint64_t king_attacks = kKingAttacksBB[sq];
        if (king_attacks & this->piece_bitboards[Piece::kWhiteKing + 6 * attacker_color])
            return true;

        // We're safe.
        return false;
    }

    bool Board::IsInCheck(Color color) const {
        uint64_t king_bb = this->piece_bitboards[Piece::kWhiteKing + 6 * color];
        
        return this->IsSquareAttacked(king_bb, color);
    }

    bool Board::IsCheckMate(std::vector<Move> &moves) const {
        return moves.empty() && this->IsInCheck(this->turn);
    }
    
    bool Board::IsStaleMate(std::vector<Move> &moves) const{
        return moves.empty() && !this->IsInCheck(this->turn);
    }
    
    bool Board::IsRepetition(int search_ply) const {
        // Search back at most halfclock moves, i.e. up to the last move that cannot be reverted.
        const int curr_index = static_cast<int>(this->hashes_history.size()) - 1;
        const int max_distance = std::min(curr_index, static_cast<int>(this->halfmoves));

        int prev_matches = 0;

        // Search jumps by 2 bcs the same positions must have the same turn.
        // Also, a repetition cannot happen 2 ply apart (bcs the opponent must revert its move)
        // so we start with dist = 4. 
        for (int dist = 4; dist <= max_distance; dist += 2) {
            // Not a match.
            if (this->hashes_history[curr_index - dist] != this->zobrist_hash)
                continue;

            // Match
            ++prev_matches;

            // True three fold repetition.
            if (prev_matches >= 2)
                return true;

            // Repetition repeats inside the search tree.
            if (search_ply > 0 && dist <= search_ply)
                return true;
        }

        return false;
    }

    PinInfo Board::GetAbsolutePinsInfo(Color my_color) const {
        PinInfo pin_info{};
        
        const Square king_sq = BitboardToSquare(piece_bitboards[Piece::kWhiteKing + 6*my_color]);
        const uint64_t all_blockers_bb = color_bitboards[Color::kWhite] | color_bitboards[Color::kBlack];
        const Color enemy_color = OppositeColor(my_color);

        // --- Find pins and pinners for straights. ---
        // Candidate pinned pieces
        uint64_t pinned_straight_bb = RookAttackBB(king_sq, all_blockers_bb) & color_bitboards[my_color];
        // Pinners.
        uint64_t pinners_straight_bb = RookAttackBB(king_sq, all_blockers_bb & ~pinned_straight_bb) 
            & (piece_bitboards[kWhiteRook + 6*enemy_color] | piece_bitboards[kWhiteQueen + 6*enemy_color]);

        // Add the pinners.
        pin_info.pinners_bb |= pinners_straight_bb;

        // Filter out candidate pinned pieces that are not actually pinned.
        // For each pinner, get the ray it and the king and check a pinned piece is there.
        for (uint64_t pinners_bb = pinners_straight_bb; pinners_bb != 0ull; pinners_bb &= ~LSB(pinners_bb)) {
            const Square pinner_sq = LSBSquare(pinners_bb);
            pin_info.pinned_bb |= (pinned_straight_bb & kSegmentBB[king_sq][pinner_sq]);
        }

        // --- Find pins and pinners for diagonals. ---
        // Candidate pinned pieces
        uint64_t pinned_diagonal_bb = BishopAttackBB(king_sq, all_blockers_bb) & color_bitboards[my_color];
        // Pinners.
        uint64_t pinners_diagonal_bb = BishopAttackBB(king_sq, all_blockers_bb & ~pinned_diagonal_bb) 
            & (piece_bitboards[kWhiteBishop + 6*enemy_color] | piece_bitboards[kWhiteQueen + 6*enemy_color]);

        // Add the pinners.
        pin_info.pinners_bb |= pinners_diagonal_bb;

        // Filter out candidate pinned pieces that are not actually pinned.
        // For each pinner, get the ray it and the king and check a pinned piece is there.
        for (uint64_t pinners_bb = pinners_diagonal_bb; pinners_bb != 0ull; pinners_bb &= ~LSB(pinners_bb)) {
            const Square pinner_sq = LSBSquare(pinners_bb);
            pin_info.pinned_bb |= (pinned_diagonal_bb & kSegmentBB[king_sq][pinner_sq]);
        }

        return pin_info;
    }

    Piece Board::GetPiece(uint64_t square_bb) const {
        for (size_t idx = 0; idx < kNumPieces; idx++) {
            if (square_bb & this->piece_bitboards[idx])
                return (Piece)idx;
        }

        return Piece::kEmpty; // Although this should be an error.
    }

    void Board::PutPiece(Piece piece, uint64_t sq) {
        const Piece captured_piece = this->GetPiece(sq);
        const unsigned square = BitboardToSquare(sq);

        this->piece_bitboards[captured_piece] &= ~sq; // Remove piece
        if (captured_piece != Piece::kEmpty) {
            this->color_bitboards[GetPieceColor(captured_piece)] &= ~sq;
            this->zobrist_hash ^= kZobrists.piece_square[captured_piece][square];

            if (captured_piece == Piece::kWhitePawn || captured_piece == Piece::kBlackPawn)
                this->pawn_zobrist_hash ^= kZobrists.piece_square[captured_piece][square];
        }

        this->piece_bitboards[piece] |= sq; // Put piece
        this->color_bitboards[GetPieceColor(piece)] |= sq;
        this->zobrist_hash ^= kZobrists.piece_square[piece][square];

        if (piece == Piece::kWhitePawn || piece == Piece::kBlackPawn)
            this->pawn_zobrist_hash ^= kZobrists.piece_square[piece][square];
    }

    void Board::RemovePiece(uint64_t sq) {
        const Piece piece = this->GetPiece(sq);
        const unsigned square = BitboardToSquare(sq);

        if (piece == Piece::kEmpty)
            return;

        this->piece_bitboards[piece] &= ~sq;
        this->color_bitboards[GetPieceColor(piece)] &= ~sq;
        this->piece_bitboards[Piece::kEmpty] |= sq;
        
        this->zobrist_hash ^= kZobrists.piece_square[piece][square];
        if (piece == Piece::kWhitePawn || piece == Piece::kBlackPawn)
            this->pawn_zobrist_hash ^= kZobrists.piece_square[piece][square];
    }

    void Board::MovePiece(uint64_t from, uint64_t to) {
        Piece piece = this->GetPiece(from);
        
        this->RemovePiece(from);
        this->PutPiece(piece, to);
    }

    void Board::UpdateCastlingRights(uint64_t from, uint64_t to) {
        const uint64_t touched = from | to;

        constexpr uint64_t e1 = SquareToBitboard(Square::E1);
        constexpr uint64_t a1 = SquareToBitboard(Square::A1);
        constexpr uint64_t h1 = SquareToBitboard(Square::H1);

        constexpr uint64_t e8 = SquareToBitboard(Square::E8);
        constexpr uint64_t a8 = SquareToBitboard(Square::A8);
        constexpr uint64_t h8 = SquareToBitboard(Square::H8);

        // Undo this castle rights from the hash.
        this->zobrist_hash ^= kZobrists.castling[this->castling];

        if (from & e1)
            this->castling &= static_cast<uint8_t>(~(Castle::kWhiteKingSide | Castle::kWhiteQueenSide));

        if (from & e8)
            this->castling &= static_cast<uint8_t>(~(Castle::kBlackKingSide | Castle::kBlackQueenSide));

        if (touched & h1)
            this->castling &= static_cast<uint8_t>(~Castle::kWhiteKingSide);

        if (touched & a1)
            this->castling &= static_cast<uint8_t>(~Castle::kWhiteQueenSide);

        if (touched & h8)
            this->castling &= static_cast<uint8_t>(~Castle::kBlackKingSide);

        if (touched & a8)
            this->castling &= static_cast<uint8_t>(~Castle::kBlackQueenSide);
    
        // Reapply the hash from the castles.
        this->zobrist_hash ^= kZobrists.castling[this->castling];
    }

    void Board::MakeMove(Move move, UndoMoveInfo& undo) {
        const uint64_t from = move.OriginBB();
        const uint64_t to = move.DestBB();
        const MoveType move_type = move.GetMoveType();
        const Piece moving_piece = this->GetPiece(from);
        const Color turn = this->turn;

        // Save the undo context.
        undo.captured_piece = Piece::kEmpty;
        undo.castling = this->castling;
        undo.en_passant = this->en_passant;
        undo.halfmoves = this->halfmoves;
        undo.fullmoves = this->fullmoves;

        // Modify stuff not related to moving pieces.
        if (this->en_passant != 0) {
            this->zobrist_hash ^= kZobrists.en_passant[BitboardToSquare(this->en_passant)];
            this->pawn_zobrist_hash ^= kZobrists.en_passant[BitboardToSquare(this->en_passant)];
        }
        this->en_passant = 0ull;
        
        this->halfmoves++;
        if (turn == Color::kBlack) {
            this->fullmoves++;
        }

        // Reset the halfmove clock
        bool is_pawn = moving_piece == Piece::kWhitePawn || moving_piece == Piece::kBlackPawn;
        
        if (is_pawn)
            this->halfmoves = 0;

        switch (move_type) {
            case MoveType::kNormal: {
                // Move
                undo.captured_piece = this->GetPiece(to);
                this->MovePiece(from, to);

                // Halfmove clock
                if (undo.captured_piece != Piece::kEmpty)
                    this->halfmoves = 0; 
                
                // Add en passant square if double pawn move and if it actually can be taken.
                if (is_pawn) {
                    if (turn == Color::kWhite && to == (from << 16)) {
                        const uint64_t ep_bb = Backward(to, Color::kWhite);
                        const uint64_t black_pawns_bb = this->piece_bitboards[Piece::kBlackPawn];
                        const uint64_t ep_takers = black_pawns_bb & (West(to) | East(to));

                        if (ep_takers) {
                            this->en_passant = ep_bb;
                            this->zobrist_hash ^= kZobrists.en_passant[BitboardToSquare(this->en_passant)];
                            this->pawn_zobrist_hash ^= kZobrists.en_passant[BitboardToSquare(this->en_passant)];
                        }
                    }
                    else if (turn == Color::kBlack && to == (from >> 16)) {
                        const uint64_t ep_bb = Backward(to, Color::kBlack);
                        const uint64_t white_pawns_bb = this->piece_bitboards[Piece::kWhitePawn];
                        const uint64_t ep_takers = white_pawns_bb & (West(to) | East(to));

                        if (ep_takers) {
                            this->en_passant = ep_bb;
                            this->zobrist_hash ^= kZobrists.en_passant[BitboardToSquare(this->en_passant)];
                            this->pawn_zobrist_hash ^= kZobrists.en_passant[BitboardToSquare(this->en_passant)];
                        }
                    }
                }

                break;
            }
            case MoveType::kPromotion: {
                // Move
                undo.captured_piece = this->GetPiece(to);
                this->RemovePiece(from);
                this->PutPiece(move.PromPiece(turn), to);

                this->halfmoves = 0;
                break;
            }
            case MoveType::kEnPassant: {
                const uint64_t captured_bb = turn == Color::kWhite ? to >> 8 : to << 8;

                undo.captured_piece = this->GetPiece(captured_bb);

                this->RemovePiece(captured_bb);
                this->MovePiece(from, to);

                this->halfmoves = 0;
                break;
            }
            case MoveType::kCastling: {
                MovePiece(from, to);

                const bool king_side = to > from;
                const uint64_t rook_from = king_side ? from << 3 : from >> 4;
                const uint64_t rook_to = king_side ? from << 1 : from >> 1;
                
                MovePiece(rook_from, rook_to);
                break;
            }
        }
        
        this->UpdateCastlingRights(from, to);
        this->turn = OppositeColor(turn);
        this->zobrist_hash ^= kZobrists.turn;

        this->hashes_history.push_back(this->zobrist_hash);
    }

    void Board::UnmakeMove(Move move, const UndoMoveInfo& undo) {
        // Get rid of that position from the history of hashes.
        this->hashes_history.pop_back();

        const uint64_t from = move.OriginBB();
        const uint64_t to = move.DestBB();

        const MoveType move_type = move.GetMoveType();

        // Return to the side that originally made the move.
        this->turn = OppositeColor(this->turn);
        this->zobrist_hash ^= kZobrists.turn;

        const Color moving_color = turn;

        switch (move_type) {
            case MoveType::kNormal: {
                MovePiece(to, from);
                if (undo.captured_piece != Piece::kEmpty)
                    PutPiece(undo.captured_piece, to);

                break;
            }

            case MoveType::kPromotion: {
                RemovePiece(to);
                const Piece pawn = moving_color == Color::kWhite ? Piece::kWhitePawn : Piece::kBlackPawn;
                PutPiece(pawn, from);

                if (undo.captured_piece != Piece::kEmpty) {
                    PutPiece(undo.captured_piece, to);
                }

                break;
            }

            case MoveType::kEnPassant: {
                MovePiece(to, from);
                const uint64_t captured_bb = moving_color == Color::kWhite ? to >> 8 : to << 8;
                PutPiece(undo.captured_piece, captured_bb);
                
                break;
            }

            case MoveType::kCastling: {
                MovePiece(to, from);

                const bool king_side = to > from;
                const uint64_t rook_from = king_side ? from << 3 : from >> 4;
                const uint64_t rook_to = king_side ? from << 1 : from >> 1;

                MovePiece(rook_to, rook_from);
                break;
            }
        }

        this->zobrist_hash ^= kZobrists.castling[this->castling];
        this->zobrist_hash ^= kZobrists.castling[undo.castling];
        this->castling = undo.castling;
        
        if (this->en_passant != 0) {
            this->zobrist_hash ^= kZobrists.en_passant[BitboardToSquare(this->en_passant)];
            this->pawn_zobrist_hash ^= kZobrists.en_passant[BitboardToSquare(this->en_passant)];
        }
        if (undo.en_passant != 0) {
            this->zobrist_hash ^= kZobrists.en_passant[BitboardToSquare(undo.en_passant)];
            this->pawn_zobrist_hash ^= kZobrists.en_passant[BitboardToSquare(undo.en_passant)];
        }
        this->en_passant = undo.en_passant;
        
        this->halfmoves = undo.halfmoves;
        this->fullmoves = undo.fullmoves;
    }
    
    bool Board::IsCapture(Move move) const {
        return move.GetMoveType() == MoveType::kEnPassant ||
            this->GetPiece(move.DestBB()) != Piece::kEmpty;
    }

    Piece Board::GetCapturedPiece(Move move) const {
        if (move.GetMoveType() == MoveType::kEnPassant) {
            return turn == Color::kWhite
                ? Piece::kBlackPawn
                : Piece::kWhitePawn;
        }

        return this->GetPiece(move.DestBB());
    }

    Piece Board::GetMovedPiece(Move move) const {
        return GetPiece(move.OriginBB());
    }
    
} // namespace lightknight
