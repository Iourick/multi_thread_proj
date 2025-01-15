#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <stdexcept>

void generate_random_file(const std::string& filename, size_t numLines, size_t nBytes)
{
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to create file");
    }

    std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<int> distribution(32, 126); // Printable ASCII characters

    size_t written = 0;
    size_t bytes_per_line = nBytes / numLines;

    for (size_t line = 0; line < numLines; ++line) {
        size_t line_bytes = (line == numLines - 1) ? (nBytes - written) : bytes_per_line;

        for (size_t i = 0; i < line_bytes; ++i) {
            file.put(static_cast<char>(distribution(generator)));
        }

        if (line < numLines - 1) {
            file.put('\n');
            ++written; // Account for newline character
        }

        written += line_bytes;
        std::cout << "written:  " << written << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: program <filename> <numLines> <nBytes>\n";
        return 1;
    }

    std::string filename = argv[1];
    size_t numLines = std::stoull(argv[2]);
    size_t nBytes = std::stoull(argv[3]);

    try {
        generate_random_file(filename, numLines, nBytes);
        std::cout << "File generated successfully.\n";
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
