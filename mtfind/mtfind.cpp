#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <regex>
#include <stdexcept>
#include <functional>

std::mutex results_mutex;
std::vector<std::tuple<int, int, std::string>> results;

void search_in_chunk(const std::vector<std::string>& lines, const std::string& mask, int start_line) {
    try {
        std::string regex_pattern = std::regex_replace(mask, std::regex("\\?"), ".");
        std::regex mask_regex(regex_pattern);

        for (size_t i = 0; i < lines.size(); ++i) {
            const auto& line = lines[i];
            std::smatch match;
            std::string::const_iterator search_start = line.begin();

            while (std::regex_search(search_start, line.end(), match, mask_regex)) {
                size_t pos = match.position();
                {
                    std::lock_guard<std::mutex> lock(results_mutex);
                    results.emplace_back(start_line + i + 1, pos + 1, match.str());
                }
                search_start = match.suffix().first;
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error in thread starting at line " << start_line << ": " << e.what() << "\n";
    }
}

void process_file(const std::string& filename, const std::string& mask) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file");
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }

    //size_t num_threads = 5; // Adjust threads
    size_t num_threads = std::thread::hardware_concurrency();
    size_t chunk_size = (lines.size() + num_threads - 1) / num_threads;
    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i) {
        size_t start = i * chunk_size;
        size_t end = std::min(start + chunk_size, lines.size());
        if (start < lines.size()) {
            std::vector<std::string> chunk(lines.begin() + start, lines.begin() + end);
            std::cout << "Creating thread for lines " << start << " to " << end << "\n";
            threads.emplace_back([=]() { search_in_chunk(chunk, mask, start); });
        }
    }

    try {
        for (size_t i = 0; i < threads.size(); ++i) {
            threads[i].join();
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Exception during thread join: " << e.what() << "\n";
        std::terminate();
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: mtfind <filename> <mask>\n";
        return 1;
    }

    std::string filename = argv[1];
    std::string mask = argv[2];

    if (mask.length() > 1000) {
        std::cerr << "Error: Mask length exceeds 1000 characters.\n";
        return 1;
    }

    try {
        process_file(filename, mask);

        std::cout << results.size() << " matches found:\n";
        for (size_t i = 0; i < results.size(); ++i) {
            std::cout << std::get<0>(results[i]) << " " << std::get<1>(results[i]) << " " << std::get<2>(results[i]) << "\n";
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}