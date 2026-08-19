#ifndef LIGHTKNIGHT_BOARD_H
#define LIGHTKNIGHT_BOARD_H

#include <cstdint>
#include <array>
#include <string>
#include <vector>
#include "types.h"

namespace lightknight {
    class Board {
    public:
        // Array of bitboards, one for each piece type, specifying their positions on the table.
        std::array<uint64_t, kNumPieces> piece_bitboards;
        std::array<uint64_t, kNumColors> color_bitboards;

        uint8_t castling;
        lightknight::Color turn;
        // Specifies the target square (if any) an en passant capture can be done.
        uint64_t en_passant;
        // Counts the number of half moves (one side) since the last capture or pawn advancement.
        // Used for the 50 rule move.
        int halfmoves;
        // Counts the number of moves played in the game. Starts at 1 and is incremented after
        // black's move.
        int fullmoves;

        // Hoshes for TT and Pawn Hash.
        uint64_t zobrist_hash = 0ULL;
        uint64_t pawn_zobrist_hash = 0ULL;
        
        // Pinned pieces bb.
        std::array<uint64_t, kNumColors> pins_diagonal_bitboard;
        std::array<uint64_t, kNumColors> pins_straight_bitboard;

        // History of positions (their hashes) for finding repetitions inside the search function.
        std::vector<uint64_t> hashes_history;

        // Constructors
        Board();
        explicit Board(const std::string& fen);
        ~Board() = default;

        // Only checks if the fen string is valid, not if the resulting position is.
        void FromFEN(const std::string& fen);

        // Does not check the validity of the data, only for testing purposes.
        static Board FromRaw(
            const std::array<uint64_t, kNumPieces>& piece_bitboards,
            Color turn,
            uint8_t castling,
            int en_passant_square,
            int halfmoves,
            int fullmoves
        );

        uint64_t ComputeZobristHash() const;
        uint64_t ComputePawnZobristHash() const;
        
        void ComputePinBitboards();

        bool IsSquareAttacked(uint64_t square_bb, Color my_color) const;
        bool IsInCheck(Color color) const;
        bool IsCheckMate(std::vector<lightknight::Move> &moves) const;
        bool IsStaleMate(std::vector<lightknight::Move> &moves) const;

        // Considers the last 'search_ply' positions to be from a search tree rather than played in
        // the game. If the current position occured twice within the search tree or three times
        // within the whole history the function returns true. To check true 3 fold repetition keep
        // 'search_ply' equal to 0.
        bool IsRepetition(int search_ply = 0) const;

        lightknight::Piece GetPiece(uint64_t square_bb) const;
        void MovePiece(uint64_t from, uint64_t to);
        void PutPiece(lightknight::Piece piece, uint64_t sq);
        void RemovePiece(uint64_t sq);
        void UpdateCastlingRights(uint64_t from, uint64_t to);
        
        void MakeMove(lightknight::Move move, lightknight::UndoMoveInfo& undo);
        void UnmakeMove(lightknight::Move move, const lightknight::UndoMoveInfo& undo);
    
        bool IsCapture(Move move) const;
        Piece GetCapturedPiece(Move move) const;
        Piece GetMovedPiece(Move move) const;
    };
} // namespace lightknight

#endif // LIGHTKNIGHT_BOARD_H
