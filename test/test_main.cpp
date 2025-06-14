/*
 * test_main.cpp
 * Unified test suite with CLI options
 */

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <algorithm>

#include "test.hpp"

using namespace std;

// Declarations for test runners from other files
void run_p2_tests(const std::string& file = "assets/mpi_office.ppm");
void run_parallel_tests(int argc = 0, char* argv[] = nullptr);
void run_cornell_box_test();
void run_bmp_tests();
void run_geometry_tests();
void run_intersect_tests();
// Add more test group declarations as needed

void print_usage(const char* prog) {
    std::cout << "Usage: " << prog << " [all|p2|parallel|cornell_box|bmp|geometry|intersect|...]\n";
    std::cout << "Available tests:\n";
    std::cout << "  all          - Run all tests\n";
    std::cout << "  p2           - Run P2 (image/tone mapping) tests\n";
    std::cout << "  parallel     - Run parallel rendering tests\n";
    std::cout << "  cornell_box  - Run Cornell Box scene test\n";
    std::cout << "  bmp          - Run BMP read/write tests\n";
    std::cout << "  geometry     - Run geometry tests\n";
    std::cout << "  intersect    - Run intersection tests\n";
    // List more test groups here
}

int main(int argc, char* argv[]) {
    std::vector<std::string> args(argv + 1, argv + argc);

    bool ran = false;
    bool run_all = false;
    if (std::find(args.begin(), args.end(), "all") != args.end()) {
        run_all = true;
    }

    for (const auto& arg : args) {
        if (arg == "all") continue; // Handled by run_all flag

        if (run_all || arg == "p2") {
            std::cout << "Running P2 tests...\n";
            run_p2_tests();
            ran = true;
            if (!run_all && arg != "all") { if (!run_all) break; }
        }
        if (run_all || arg == "parallel") {
            std::cout << "Running parallel tests...\n";
            run_parallel_tests(); 
            ran = true;
            if (!run_all && arg != "all") { if (!run_all) break; }
        }
        if (run_all || arg == "cornell_box") {
            std::cout << "Running Cornell Box test...\n";
            run_cornell_box_test();
            ran = true;
            if (!run_all && arg != "all") { if (!run_all) break; }
        }
        if (run_all || arg == "bmp") {
            std::cout << "Running BMP tests...\n";
            run_bmp_tests();
            ran = true;
            if (!run_all && arg != "all") { if (!run_all) break; }
        }
        if (run_all || arg == "geometry") {
            std::cout << "Running geometry tests...\n";
            run_geometry_tests();
            ran = true;
            if (!run_all && arg != "all") { if (!run_all) break; }
        }
        if (run_all || arg == "intersect") {
            std::cout << "Running intersect tests...\n";
            run_intersect_tests();
            ran = true;
            if (!run_all && arg != "all") { if (!run_all) break; }
        }
        // Add more test group checks here
    }

    if (run_all && !ran) { // If 'all' was specified but no specific tests ran (e.g. only 'all' was arg)
        std::cout << "Running P2 tests...\n";
        run_p2_tests();
        std::cout << "Running parallel tests...\n";
        run_parallel_tests();
        std::cout << "Running Cornell Box test...\n";
        run_cornell_box_test();
        std::cout << "Running BMP tests...\n";
        run_bmp_tests();
        std::cout << "Running geometry tests...\n";
        run_geometry_tests();
        std::cout << "Running intersect tests...\n";
        run_intersect_tests();
        ran = true;
    }

    if (!ran && !args.empty() && args[0] != "all") {
        bool known_arg = false;
        for (const auto& arg : args) {
            if (arg == "p2" || arg == "parallel" || arg == "cornell_box" || arg == "bmp" || arg == "geometry" || arg == "intersect") {
                known_arg = true;
                break;
            }
        }
        if (!known_arg) {
             std::cerr << "❌ Unknown test category specified: " << args[0] << "\n\n";
             print_usage(argv[0]);
             return 1;
        }
    } else if (!ran) {
        std::cerr << "❌ No valid test categories specified. Use 'all' or specific test names.\n\n";
        print_usage(argv[0]);
        return 1;
    }

    return 0;
}
