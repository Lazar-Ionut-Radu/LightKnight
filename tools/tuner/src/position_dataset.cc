// tools/tuner/src/position_dataset.cc
#include "position_dataset.h"
#include "board.h"

#include <algorithm>
#include <istream>
#include <utility>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <iterator>

#define PERCENTAGE_STEP 10
#define N_LOADING_BAR 20
#define CHAR_FULL '#'
#define CHAR_EMPTY '_'

namespace lightknight::tuner {
    size_t CountLines(const std::string& path) {
        std::ifstream file(path);
        if (!file) 
            throw std::runtime_error("Could not open file: " + path);

        size_t count = std::count_if(std::istreambuf_iterator<char>{file}, {}, [](char c) { return c == '\n'; });
        return count;
    }

    PositionDataset LoadPositionDataset(const std::string& path) {
        std::ifstream file(path);
        if (!file) 
            throw std::runtime_error("Could not open position dataset: " + path);

        const size_t n_lines = CountLines(path) - 1;
        size_t n_processed_lines = 0;
        size_t next_print_percentage = PERCENTAGE_STEP;

        // Print stuff.
        std::cout << "Loading dataset: ";
        for (int i = 0; i < N_LOADING_BAR; i++) {
            std::cout << CHAR_EMPTY;
        }
        std::cout << ": 0%, 0/" << n_lines << " positions \n";

        PositionDataset dataset;
        std::string line;

        // Skip the csv header
        std::getline(file, line);

        while (std::getline(file, line)) {
            if (line.empty())
                continue;

            std::stringstream stream(line);
            std::string fen;
            std::string win_str;
            std::string draws_str;
            std::string losses_str;

            std::getline(stream, fen, ',');
            std::getline(stream, win_str, ',');
            std::getline(stream, draws_str, ',');
            std::getline(stream, losses_str, ',');

            if (fen.empty() || win_str.empty() || draws_str.empty() || losses_str.empty())
                continue;

            Board board;
            board.FromFEN(fen);

            dataset.push_back({std::move(board), std::stoi(win_str), std::stoi(draws_str), std::stoi(losses_str)});
        
            // Print stuff.
            n_processed_lines++;
            size_t percentage = n_processed_lines * 100 / n_lines;
            
            if (percentage != 100 && percentage >= next_print_percentage) {
                std::cout << "Loading dataset: ";
                for (int i = 0; i < N_LOADING_BAR; i++) {
                    if (i * 100 / N_LOADING_BAR < percentage)
                        std::cout << CHAR_FULL;
                    else
                        std::cout << CHAR_EMPTY;
                }
                std::cout << ": " << percentage << "%, " << n_processed_lines << "/"<< n_lines << " positions \n";
                
                next_print_percentage += PERCENTAGE_STEP;
            }
        }
        std::cout << "Loading dataset: ";
        for (int i = 0; i < N_LOADING_BAR; i++) {
            std::cout << CHAR_FULL;
        }
        std::cout << ": 100%, " << n_lines << "/" << n_lines << " positions \n";
        std::cout << "Loaded " << dataset.size() << " positions.\n";
        return dataset;
    }
} // namespace lightknight::tuner