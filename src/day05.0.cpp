#include <cassert>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string_view>

namespace {

class Solution {

    static constexpr int row_length = 7;
    static constexpr int col_length = 3;
    static constexpr char row_hi = 'B';
    static constexpr char row_lo = 'F';
    static constexpr char col_hi = 'R';
    static constexpr char col_lo = 'L';


    int solution = 0;

    int decode(const std::string_view str, char lo, char hi)
    {
        int ret = 0;
        for (auto ch: str) {
            ret <<= 1;
            const bool bit = [&] {
                if (ch == lo) {
                    return 0;
                }
                else {
                    assert(ch == hi);
                    return 1;
                }
            }();
            ret += bit;
        }

        return ret;
    }

    int computeID(int row, int col)
    {
        return row * 8 + col;
    }

    bool solve(const std::string& line)
    {

        const auto col_begin = &*line.cbegin() + row_length;
        const int row = decode({&*line.cbegin(), row_length}, row_lo, row_hi);
        const int col = decode({col_begin, col_length}, col_lo, col_hi);

        // std::cout << "row: " << row << " col: " << col << '\n';

        const int ID = computeID(row, col);
        solution = std::max(solution, ID);

        return true;
    }

  public:
    template <typename ...Args>
    bool operator()(Args... args)
    {
        return solve(args...);
    }

    int output() const
    {
        return solution;
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
