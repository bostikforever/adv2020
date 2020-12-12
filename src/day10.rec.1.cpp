#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <tuple>
#include <fstream>
#include <iostream>
#include <utility>
#include <vector>

namespace {

class Solution {

    uint64_t solution = 0;
    std::size_t calls = 0;

    template <typename CALLABLE>
    bool solve(CALLABLE&& loop)
    {
        ++calls;
        std::vector<bool> values(1, true);
        const auto ret = loop([&](std::size_t arg) {
            values.resize(std::max(values.size(), arg + 1));
            values[arg] = true;
            return true;
        });

        std::tuple<uint64_t, uint64_t, uint64_t> prevs = {
                                         values[2] * ( values[1] + values[0] ),
                                         values[1] * values[0],
                                         values[0] };

        for (int i = 3; i < values.size(); ++i) {
            prevs = { values[i] * ( std::get<0>(prevs) +
                                    std::get<1>(prevs) +
                                    std::get<2>(prevs) ),
                      std::get<0>(prevs),
                      std::get<1>(prevs) };
        }

        solution = std::get<0>(prevs);
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
    std::size_t arg;

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
