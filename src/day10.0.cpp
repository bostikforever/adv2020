#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <numeric>
#include <vector>

namespace {

class Solution {

    uint64_t solution = 0;
    std::size_t calls = 0;

    template <typename CALLABLE>
    bool solve(CALLABLE&& loop)
    {
        ++calls;
        std::vector<uint64_t> values;
        const auto ret = loop([&](uint64_t arg) {
            values.emplace_back(arg);
            return true;
        });
        std::sort(values.begin(), values.end());
        std::adjacent_difference(values.begin(),
                                 values.end(),
                                 values.begin());
        int count_1 = std::count(values.begin(), values.end(), 1);
        int count_3 = 1 + std::count(values.begin(), values.end(), 3);
        solution = count_1 * count_3;
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
    uint64_t arg;

    const auto ret = callable([&](auto&& body) {
        bool res;
        while (inputFile >> arg && (res = body(arg)))
            ;
        return res;
    });
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
