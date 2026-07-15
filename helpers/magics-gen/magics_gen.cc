#include "magics_gen.h"
#include "movegen.h"
#include <iostream>
#include <cstdint>
#include <random>
#include <unordered_set>
#include <vector>
#include <array>
#include <string>

const uint64_t kSeed = 42;

uint64_t RandU64()
{
    static std::mt19937_64 rng(kSeed);
    return rng();
}

int GetOccupancyHash(uint64_t occupancy, uint64_t magic, int shift)
{
    return (occupancy * magic) >> shift;
}

std::vector<uint64_t> GenerateBlockerBoards(uint64_t relevant_occupancy)
{
    std::vector<int> bit_indices;
    for (int sq = 0; sq < 64; sq++) {
        if (relevant_occupancy & (1ULL << sq))
        bit_indices.push_back(sq);
    }

    std::vector<uint64_t> bitboards;
    uint64_t num_bitboards = 1ULL << bit_indices.size();
    
    for (uint64_t i = 0; i < num_bitboards; i++) {
        uint64_t bitboard = 0ULL;
        for (size_t j = 0; j < bit_indices.size(); j++)
            if (i & (1ULL << j))
                bitboard |= (1ULL << bit_indices[j]);
        
        bitboards.push_back(bitboard);
    }

    return bitboards;
}

uint64_t FindRookMagic(int square, int shift, int n_attempts) {
    // Check if there are enough bits for the hash.
    if (kRookBits[square] > 64 - shift) {
        std::cerr << "Shift too big for " << kRookBits[square] << " bits" << std::endl;
        return 0ULL;
    }
    
    std::vector<uint64_t> blockers_bitboards = GenerateBlockerBoards(lightknight::movegen::kRookRelevantOccupancy[square]);
    std::unordered_set<uint64_t> hashes;    

    // For a number of attempts check a randomly generated magic to see if it produces a valid hash.
    for (int i = 0; i < n_attempts; i++) {
        uint64_t candidate_magic = RandU64() & RandU64() & RandU64();
        hashes.clear();
        
        for (uint64_t occupancy : blockers_bitboards) {
            uint64_t new_hash = GetOccupancyHash(occupancy, candidate_magic, shift);
            if (hashes.contains(new_hash))
                break;
            
            hashes.insert(new_hash);
        }

        // Found.
        if (hashes.size() == blockers_bitboards.size())
            return candidate_magic;
    }

    // Not found within the allocated number of attempts.
    std::cerr << "Unable to find rook magic for square " << square << " within " << n_attempts << " attempts" << std::endl;
    return 0ULL;
}

uint64_t FindBishopMagic(int square, int shift, int n_attempts) {
    // Check if there are enough bits for the hash.
    if (kBishopBits[square] > 64 - shift) {
        std::cerr << "Shift too big for " << kBishopBits[square] << " bits" << std::endl;
        return 0ULL;
    }
    
    std::vector<uint64_t> blockers_bitboards = GenerateBlockerBoards(lightknight::movegen::kBishopRelevantOccupancy[square]);
    std::unordered_set<uint64_t> hashes;    

    // For a number of attempts check a randomly generated magic to see if it produces a valid hash.
    for (int i = 0; i < n_attempts; i++) {
        uint64_t candidate_magic = RandU64() & RandU64() & RandU64();
        hashes.clear();
        
        for (uint64_t occupancy : blockers_bitboards) {
            uint64_t new_hash = GetOccupancyHash(occupancy, candidate_magic, shift);

            if (hashes.contains(new_hash))
                break;

            hashes.insert(new_hash);
        }

        // Found.
        if (hashes.size() == blockers_bitboards.size())
            return candidate_magic;
    }

    // Not found within the allocated number of attempts.
    std::cerr << "Unable to find bishop magic for square " << square << " within " << n_attempts << " attempts" << std::endl;
    return 0ULL;
}

void PrintMagicsArray(std::array<uint64_t, 64> &magics, const std::string var_name)
{
    std::cout << "inline constexpr std::array<uint64_t, kNumSquares> " << var_name << " = {" << std::endl;
    for (int rank = 0; rank < 8; rank++) {
        std::cout << "\t"; 
        for (int file = 0; file < 8; file++) {
            std::cout << "0x" << std::hex << magics[rank * 8 + file] << std::dec << "ULL, ";
        }
        std::cout << std::endl;
    }
    std::cout << "};" << std::endl;
}       

bool GenerateRookFixedShiftMagics(int shift, int n_attempts)
{
    std::array<uint64_t, 64> magics = {0ULL};
    for (int i = 0; i < 64; i++) {
        magics[i] = FindRookMagic(i, shift, n_attempts);
             
        if (magics[i] == 0) 
            return false;
    }

    PrintMagicsArray(magics, "kRookMagics");
    return true;
}

bool GenerateBishopFixedShiftMagics(int shift, int n_attempts)
{
    std::array<uint64_t, 64> magics = {0ULL};
    for (int i = 0; i < 64; i++) {
        magics[i] = FindBishopMagic(i, shift, n_attempts);

        if (magics[i] == 0) 
            return false;
    }

    PrintMagicsArray(magics, "kBishopMagics");
    return true;
}

int main() {
    GenerateRookFixedShiftMagics(64 - 12, 1'000'000);
    //GenerateBishopFixedShiftMagics(64 - 9, 1'000'000);
    return 0;
}