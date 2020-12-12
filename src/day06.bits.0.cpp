#include <algorithm>
#include <bitset>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <numeric>
#include <string>

namespace {

class Solution {

    static constexpr uint32_t all_ones_26 = 0x3ffffff;
    static constexpr int N = 26;
    static_assert(uint32_t(~0) >> (32 - N) == all_ones_26);

    int solution = 0;
    int calls = 0;

    template <typename CALLABLE>
    bool solve(CALLABLE&& loop)
    {
        ++calls;
        uint32_t seen = 0;
        const auto ret = loop([&](const std::string& line) {
            seen = std::accumulate(line.begin(),
                                   line.end(),
                                   seen,
                                   [](auto acc, uint8_t ch) {
                const uint8_t ord = ch - 'a';
                assert(0 <= ord < N);
                return acc | (1 << ord);

            });
            // optimisation: bail out early from group if we've seen everything
            return seen != all_ones_26;
        });
        solution += std::bitset<N>(seen).count();
        return ret;
    }

  public:
    template <typename ...Args>
    bool operator()(Args... args)
    {
        return solve(args...);
    }

    auto output() const
    {
        std::cout << "calls: " << calls << '\n';
        return solution;
    }
};

template <typename CALLABLE>
void readBatches(const std::string& filename, CALLABLE&& callable)
{
    std::ifstream inputFile(filename);
    std::string line;

    while (inputFile) {
        callable([&](auto&& body) {
            bool res;
            while (getline(inputFile, line) &&
                   !line.empty() &&
                   (res = body(line)))
                ;

            // exhaust remaining group input
            while (!line.empty() && getline(inputFile, line))
               ;
            return res;
        });
    }
}

}  // close unnamed namespace

int main(int argc, const char* argv[])
{
    assert(argc > 1);
    std::string filename = argv[1];
    Solution solution;
    readBatches(filename, solution);
    std::cout << solution.output() << '\n';
    return 0;
}
