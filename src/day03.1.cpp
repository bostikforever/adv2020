#include <algorithm>
#include <array>
#include <fstream>
#include <functional>
#include <iostream>
#include <numeric>
#include <string>
#include <utility>

namespace {

class Solution {

    static constexpr int N = 5;
    static constexpr std::array<int, N> x_delta {1, 3, 5, 7, 1};
    static constexpr std::array<int, N> y_delta {1, 1, 1, 1, 2};

    std::array<int, N> solution {0, 0, 0, 0, 0};
    std::array<int, N> x_pos {0, 0, 0, 0, 0};
    int y_pos = 0;

    bool solve(const std::string& line)
    {
        for (int i = 0; i < N; ++i) {
            if (y_pos % y_delta[i]) {
                continue;
            }
            int& pos = x_pos[i];
            bool valid = line[pos] == '#';
            solution[i] += valid;

            pos += x_delta[i];
            pos %= line.size();
        }
        y_pos++;
        return true;
    }

  public:
    template <typename ...Args>
    bool operator()(Args... args)
    {
        return solve(args...);
    }

    auto output() const
    {
        return std::accumulate(solution.begin(),
                               solution.end(),
                               1UL,
                               std::multiplies());
    }
};

template <typename CALLABLE>
void readLine(const std::string& filename, CALLABLE&& callable)
{
    std::ifstream inputFile(filename);
    std::string line;
    while (inputFile >> line && callable(line))
        ;
}

}  // close unnamed namespace

int main(int argc, const char* argv[])
{
    assert(argc > 1);
    std::string filename = argv[1];
    Solution solution;
    readLine(filename, solution);
    std::cout << solution.output() << '\n';
    return 0;
}
