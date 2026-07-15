#ifndef LIGHTKNIGHT_MAGICS_GEN_H
#define LIGHTKNIGHT_MAGICS_GEN_H

#include <cstdint>
#include <vector>
#include <string>

extern const uint64_t kSeed;

const int kRookBits[64] = {
  12, 11, 11, 11, 11, 11, 11, 12,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  12, 11, 11, 11, 11, 11, 11, 12
};

const int kBishopBits[64] = {
  6, 5, 5, 5, 5, 5, 5, 6,
  5, 5, 5, 5, 5, 5, 5, 5,
  5, 5, 7, 7, 7, 7, 5, 5,
  5, 5, 7, 9, 9, 7, 5, 5,
  5, 5, 7, 9, 9, 7, 5, 5,
  5, 5, 7, 7, 7, 7, 5, 5,
  5, 5, 5, 5, 5, 5, 5, 5,
  6, 5, 5, 5, 5, 5, 5, 6
};

uint64_t RandU64();
int GetOccupancyHash(uint64_t occupancy, uint64_t magic, int shift);
std::vector<uint64_t> GenerateBlockerBoards(uint64_t relevant_occupancy);
uint64_t FindRookMagic(int square, int shift, int n_attempts);
uint64_t FindBishopMagic(int square, int shift, int n_attempts);
void PrintMagicsArray(std::array<uint64_t, 64> &magics, const std::string var_name);
bool GenerateRookFixedShiftMagics(int shift, int n_attempts);
bool GenerateBishopFixedShiftMagics(int shift, int n_attempts);

#endif // LIGHTKNIGHT_MAGICS_GEN_H