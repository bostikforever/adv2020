#include <algorithm>
#include <bitset>
#include <fstream>
#include <iostream>
#include <numeric>
#include <string>

namespace {

class Solution {

    static constexpr char occupied = '#';
    static constexpr char floor = '.';
    static constexpr char empty = 'L';
    static constexpr int8_t directions[] = {-1, 0, 1};

    uint64_t solution = 0;
    std::size_t calls = 0;

    uint8_t toBitPos(int8_t dir_x, int8_t dir_y) const
    {
        constexpr size_t N = 3;
        static_assert(N == std::size(directions));
        dir_x = (N + dir_x) % N;
        dir_y = (N + dir_y) % N;
        return N * N - (N * dir_x + dir_y) - 1;
    }

    std::size_t popCount(uint8_t val) const
    {
        return std::bitset<8>(val).count();
    }

    bool checkOccupy(std::string&      tmp,
                     std::string&      values,
                     const std::size_t rowSize,
                     const std::size_t colSize) const
    {

        // for (auto i = 0; i < values.size(); ++i) {
        //     if (i % rowSize == 0) {
        //         std::cout << '\n';
        //     }
        //     std::cout << values[i];
        // }
        // std::cout << '\n';
        tmp.swap(values);
        bool changed = false;
        // std::fill(values.begin(), values.end(), 0);
            // fill done individually manaully below, instead.

        for (std::size_t i = 0; i < values.size(); ++i) {
            if (tmp[i] != occupied) {
                continue;
            }

            for (auto dirx : directions) {
            for (auto diry : directions) {
                if (diry == dirx && dirx == 0) {
                    continue;
                }
                std::size_t row = i / rowSize;
                std::size_t col = i % rowSize;
                while (0 <= row + diry && row + diry < colSize &&
                       0 <= col + dirx && col + dirx < rowSize) {
                    col += dirx;
                    row += diry;
                    const std::size_t linIdx = row * rowSize + col;

                    const auto bitPos = toBitPos(dirx, diry);

                    // optmisation (shaves off like 5ms on input, lol)
                    const bool alreadySet = values[linIdx] & 1 << bitPos;
                    if (alreadySet) {
                        break;
                    }
                    // end optimisation

                    values[linIdx] |= 1 << bitPos;
                    if(tmp[linIdx] != floor) {
                        break;
                    }
                }
            }}
        }

        for (std::size_t i = 0; i < values.size(); ++i) {
            const std::size_t occlusionCount = popCount(values[i]);
            // if (i % rowSize == 0) {
            //     std::cout << '\n';
            // }
            // std::cout << int(occlusionCount) << ' ';

            if (tmp[i] == occupied && occlusionCount >= 5) {
                values[i] = empty;
                changed = true;
                tmp[i] = 0;
                continue;
            }
            if (tmp[i] == empty && occlusionCount == 0) {
                values[i] = occupied;
                changed = true;
                tmp[i] = 0;
                continue;
            }
            // unchanged
            values[i] = tmp[i];
            tmp[i] = 0;
        }
        return changed;
    }

    std::size_t countOccupied(const std::string& values) const
    {
       return std::count(values.begin(), values.end(), occupied);
    }

    template <typename CALLABLE>
    bool solve(CALLABLE&& loop)
    {
        ++calls;
        std::string values;
        int cols = 0;
        const auto ret = loop([&](const std::string& line) {
            ++cols;
            values += line;
            return true;
        });
        std::string valuesTemp(values.size(), 0);
        while (checkOccupy(valuesTemp, values, values.size()/cols, cols))
            ;
        solution = countOccupied(values);
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

    const auto ret = callable([&](auto&& body) {
        bool res;
        while (inputFile >> line && (res = body(line)))
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
