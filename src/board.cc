#include "board.h"
#include "exceptions.h"
#include "movegen.h"

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
            throw lightknight::exceptions::FENException("Invalid FEN string");
        }

        // Extract each section
        std::string placement_str  = match[1];
        std::string turn_str       = match[2];
        std::string castling_str   = match[3];
        std::string en_passant_str = match[4];
        std::string halfmove_str   = match[5];
        std::string fullmove_str   = match[6];

        // Setup the pieces on the board.
        static const std::unordered_map<char, lightknight::Piece> char_to_piece = {
            {'P', lightknight::Piece::kWhitePawn}, {'p', lightknight::Piece::kBlackPawn},
            {'N', lightknight::Piece::kWhiteKnight}, {'n', lightknight::Piece::kBlackKnight},
            {'B', lightknight::Piece::kWhiteBishop}, {'b', lightknight::Piece::kBlackBishop},
            {'R', lightknight::Piece::kWhiteRook}, {'r', lightknight::Piece::kBlackRook},
            {'Q', lightknight::Piece::kWhiteQueen}, {'q', lightknight::Piece::kBlackQueen},
            {'K', lightknight::Piece::kWhiteKing}, {'k', lightknight::Piece::kBlackKing},
        };
        int file = 0, rank = 7;
        for (size_t i = 0; i < placement_str.size(); ++i) {
            // Go on the next rank
            if (placement_str[i] == '/') {
                file = 0;
                rank--;

                if (rank < 0)
                    throw lightknight::exceptions::FENException("Invalid FEN string ops");
            }
            else {
                // Empty spaces
                if (placement_str[i] < 'A') {
                    for (int num = 1; num <= (int)(placement_str[i] - '0'); ++num) {
                        this->piece_bitboards[lightknight::Piece::kEmpty] |= 1ULL << (8*rank + file);
                        file++;

                        if (file > 8)
                            throw lightknight::exceptions::FENException("Invalid FEN string ops");
                    }
                }
                // Pieces
                else {
                    this->piece_bitboards[char_to_piece.at(placement_str[i])] |= 1ULL << (8*rank + file);
                    file++;

                    if (rank > 8)
                        throw lightknight::exceptions::FENException("Invalid FEN string ops");
                }
            }
        }
        if (rank > 0)
            throw lightknight::exceptions::FENException("Invalid FEN string ops");

        // Setup the turn
        if (turn_str[0] == 'w')
            this->turn = lightknight::Color::kWhite;
        else
            this->turn = lightknight::Color::kBlack;
        
        // Setup the castling rights
        if (castling_str[0] != '-') {
            for (char c : castling_str) {
                switch (c) {
                    case 'K':
                        this->castling |= lightknight::Castle::kWhiteKingSide;
                        break;
                    case 'Q':
                        this->castling |= lightknight::Castle::kWhiteQueenSide;
                        break;
                    case 'k':
                        this->castling |= lightknight::Castle::kBlackKingSide;
                        break;
                    case 'q':
                        this->castling |= lightknight::Castle::kBlackQueenSide;
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
        this->turn = lightknight::Color::kWhite;
    }
    
    // Checks if this square is attacked by a piece of the specified color.
    bool Board::IsSquareAttacked(uint64_t square_bb, Color my_color) const {
        Color attacker_color = (Color)(1 - my_color);
        Square sq = BitboardToSquare(square_bb);

        // Compute diagonal and straight attacks from the king's POV to find possible attackers.
        uint64_t blockers = this->color_bitboards[0] | this->color_bitboards[1];
        uint64_t straights = lightknight::movegen::RookAttackBitboard(sq, blockers);
        uint64_t diagonals = lightknight::movegen::BishopAttackBitboard(sq, blockers);

        bool check_on_diagonals = diagonals &
            (this->piece_bitboards[Piece::kWhiteBishop + 6*attacker_color] | this->piece_bitboards[Piece::kWhiteQueen + 6*attacker_color]);
        bool check_on_straights = straights &
            (this->piece_bitboards[Piece::kWhiteRook + 6*attacker_color] | this->piece_bitboards[Piece::kWhiteQueen + 6*attacker_color]);
    
        // Compute knight attacks from the king's POV to find checks
        uint64_t knights_attacks = lightknight::movegen::kKnightAttacks[sq];
        bool check_by_knight = knights_attacks & this->piece_bitboards[Piece::kWhiteKnight + 6*attacker_color];

        // Compute pawn attacks from the king's POV to find checks
        uint64_t pawn_attacks = lightknight::movegen::PawnAttackBitboard(sq, my_color);
        bool check_by_pawn = pawn_attacks & this->piece_bitboards[Piece::kWhitePawn + 6*attacker_color];

        // Verdict.
        if (check_on_diagonals || check_on_straights || check_by_knight || check_by_pawn) {
            return true;
        }
        return false;
    }

    bool Board::IsInCheck(Color color) const {
        uint64_t king_bb = this->piece_bitboards[lightknight::Piece::kWhiteKing + 6 * color];
        
        return this->IsSquareAttacked(king_bb, color);
    }

    bool Board::IsCheckMate(std::vector<lightknight::Move> &moves) const {
        return moves.empty() && this->IsInCheck(this->turn);
    }
    
    bool Board::IsStaleMate(std::vector<lightknight::Move> &moves) const{
        return moves.empty() && !this->IsInCheck(this->turn);
    }
    
    lightknight::Piece Board::GetPiece(uint64_t square_bb) {
        for (size_t idx = 0; idx < lightknight::kNumPieces; idx++) {
            if (square_bb & this->piece_bitboards[idx])
                return (lightknight::Piece)idx;
        }

        return lightknight::Piece::kEmpty; // Although this should be an error.
    }

    void Board::PutPiece(lightknight::Piece piece, uint64_t sq) {
        lightknight::Piece captured_piece = this->GetPiece(sq);
        
        this->piece_bitboards[captured_piece] &= ~sq; // Remove piece
        if (captured_piece != lightknight::Piece::kEmpty)
            this->color_bitboards[GetPieceColor(captured_piece)] &= ~sq;
        
        this->piece_bitboards[piece] |= sq; // Put piece
        this->color_bitboards[GetPieceColor(piece)] |= sq;
    }

    void Board::RemovePiece(uint64_t sq) {
        lightknight::Piece piece = this->GetPiece(sq);

        if (piece == lightknight::Piece::kEmpty)
            return;

        this->piece_bitboards[piece] &= ~sq;
        this->color_bitboards[GetPieceColor(piece)] &= ~sq;
        this->piece_bitboards[lightknight::Piece::kEmpty] |= sq;
    }

    void Board::MovePiece(uint64_t from, uint64_t to) {
        lightknight::Piece piece = this->GetPiece(from);
        
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

        if (from & e1) {
            this->castling &= static_cast<uint8_t>(
                ~(kWhiteKingSide | kWhiteQueenSide)
            );
        }

        if (from & e8) {
            castling &= static_cast<uint8_t>(
                ~(kBlackKingSide | kBlackQueenSide)
            );
        }

        if (touched & h1)
            this->castling &= static_cast<uint8_t>(~kWhiteKingSide);

        if (touched & a1)
            this->castling &= static_cast<uint8_t>(~kWhiteQueenSide);

        if (touched & h8)
            this->castling &= static_cast<uint8_t>(~kBlackKingSide);

        if (touched & a8)
            this->castling &= static_cast<uint8_t>(~kBlackQueenSide);
    }

    void Board::MakeMove(lightknight::Move move, lightknight::UndoMoveInfo& undo) {
        const uint64_t from = move.GetOriginBitboard();
        const uint64_t to = move.GetDestionationBitboard();
        const lightknight::MoveType move_type = move.GetMoveType();
        const lightknight::Piece moving_piece = this->GetPiece(from);
        const lightknight::Color turn = this->turn;

        // Save the undo context.
        undo.captured_piece = lightknight::Piece::kEmpty;
        undo.castling = this->castling;
        undo.en_passant = this->en_passant;
        undo.halfmoves = this->halfmoves;
        undo.fullmoves = this->fullmoves;

        // Modify stuff not related to moving pieces.
        this->en_passant = 0ull;
        this->halfmoves++;
        if (turn == lightknight::Color::kBlack) {
            this->fullmoves++;
        }

        // Reset the halfmove clock
        bool is_pawn = moving_piece == lightknight::Piece::kWhitePawn || 
                       moving_piece == lightknight::Piece::kBlackPawn;
        
        if (is_pawn)
            this->halfmoves = 0;

        switch (move_type) {
            case lightknight::MoveType::kNormal: {
                // Move
                undo.captured_piece = this->GetPiece(to);
                this->MovePiece(from, to);

                // Halfmove clock
                if (undo.captured_piece != lightknight::Piece::kEmpty)
                    this->halfmoves = 0; 
                
                // Add en passant square if double pawn move.
                if (is_pawn) {
                    if (turn == lightknight::Color::kWhite && to == (from << 16))
                        this->en_passant = from << 8;
                    else if (turn == lightknight::Color::kBlack && to == (from >> 16))
                        this->en_passant = from >> 8;
                }

                break;
            }
            case lightknight::MoveType::kPromotion: {
                // Move
                undo.captured_piece = this->GetPiece(to);
                this->RemovePiece(from);
                this->PutPiece(lightknight::GetPiece(turn, move.GetPromotionPieceType()), to);

                this->halfmoves = 0;
                break;
            }
            case lightknight::MoveType::kEnPassant: {
                const uint64_t captured_bb = turn == lightknight::Color::kWhite
                    ? to >> 8
                    : to << 8;

                undo.captured_piece = this->GetPiece(captured_bb);

                this->RemovePiece(captured_bb);
                this->MovePiece(from, to);

                this->halfmoves = 0;
                break;
            }
            case lightknight::MoveType::kCastling: {
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
    }

    void Board::UnmakeMove(lightknight::Move move, const lightknight::UndoMoveInfo& undo) {
        const uint64_t from = move.GetOriginBitboard();
        const uint64_t to = move.GetDestionationBitboard();

        const lightknight::MoveType move_type =
            move.GetMoveType();

        // Return to the side that originally made the move.
        this->turn = OppositeColor(this->turn);

        const lightknight::Color moving_color = turn;

        switch (move_type) {
            case lightknight::MoveType::kNormal: {
                MovePiece(to, from);

                if (undo.captured_piece !=
                    lightknight::Piece::kEmpty) {
                    PutPiece(undo.captured_piece, to);
                }

                break;
            }

            case lightknight::MoveType::kPromotion: {
                RemovePiece(to);

                const lightknight::Piece pawn =
                    moving_color == lightknight::Color::kWhite
                        ? lightknight::Piece::kWhitePawn
                        : lightknight::Piece::kBlackPawn;

                PutPiece(pawn, from);

                if (undo.captured_piece !=
                    lightknight::Piece::kEmpty) {
                    PutPiece(undo.captured_piece, to);
                }

                break;
            }

            case lightknight::MoveType::kEnPassant: {
                MovePiece(to, from);

                const uint64_t captured_bb =
                    moving_color == lightknight::Color::kWhite
                        ? to >> 8
                        : to << 8;

                PutPiece(undo.captured_piece, captured_bb);
                break;
            }

            case lightknight::MoveType::kCastling: {
                MovePiece(to, from);

                const bool king_side = to > from;

                const uint64_t rook_from =
                    king_side ? from << 3 : from >> 4;

                const uint64_t rook_to =
                    king_side ? from << 1 : from >> 1;

                MovePiece(rook_to, rook_from);
                break;
            }
        }

        this->castling = undo.castling;
        this->en_passant = undo.en_passant;
        this->halfmoves = undo.halfmoves;
        this->fullmoves = undo.fullmoves;
    }
    
} // namespace lightknight
