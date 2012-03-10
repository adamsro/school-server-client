#include <cstdlib>
#include <iostream>

#include <cerrno>
#include <fstream>
#include <vector>
#include <jansson.h>

/*
 * For each number in range test if perfect number
 */
std::vector<unsigned long> brute_perfect(unsigned long start, unsigned long end) {
    std::vector<unsigned long> perfect_nums;

    for (unsigned long testnum = start; testnum < end; ++testnum) {
        unsigned long factor_sum = 0;
        for (unsigned long k = 1; k < testnum; ++k) {
            if (testnum % k == 0) { // is k a factor of testnum?
                factor_sum += k;
            }
        }
        // does testnum meet the definition of perf. num?
        if (factor_sum == testnum) {
            perfect_nums.push_back(testnum);
        }
    }
    return perfect_nums;
}

void print_vector(std::vector<unsigned long> temp) {
    std::cout << "arr: ";
    for (int i = 0; i < (int) temp.size(); ++i) {
        std::cout << temp.at(i) << " ";
    }
    std::cout << std::endl;
}

int main(int argc, char* argv[]) {
    print_vector(brute_perfect(1, 9589));
}

