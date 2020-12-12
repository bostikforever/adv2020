#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>

namespace {

class Solution {

    static constexpr char occupied = '#';
    static constexpr char floor = '.';
    static constexpr char empty = 'L';
    static constexpr int8_t dirs[] = {-1, 0, 1};

    uint64_t solution = 0;
    std::size_t calls = 0;

    bool checkOccupy(std::string& tmp,
                     std::string& values,
                     std::size_t  rowSize,
                     std::size_t  colSize) const
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
        for (std::size_t i = 0; i < values.size(); ++i) {
            if (tmp[i] == floor) {
                continue;
            }

            const std::size_t col = i % rowSize;
            std::size_t occupiedCount = 0;
            for (auto dir_x : dirs) {
                if (col == 0 && dir_x == -1 ||
                    col == rowSize - 1 && dir_x == 1) {
                    continue;
                }
                const std::size_t row = i / rowSize;
                for (auto dir_y : dirs) {
                    if (row == 0 && dir_y == -1 ||
                        row == colSize - 1 && dir_y == 1) {
                        continue;
                    }
                    if (dir_y == dir_x && dir_x == 0) {
                        continue;
                    }
                    occupiedCount += tmp[i + dir_x + dir_y * rowSize]
                                                                   == occupied;
                }
            }
            // if (i % rowSize == 0) {
            //     std::cout << '\n';
            // }
            // std::cout << occupiedCount;
            if (tmp[i] == occupied && occupiedCount >= 4) {
                values[i] = empty;
                changed = true;
                continue;
            }
            if (tmp[i] == empty && occupiedCount == 0) {
                values[i] = occupied;
                changed = true;
                continue;
            }
            // unchanged
            values[i] = tmp[i];
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
        std::string valuesTemp(values);
        while (checkOccupy(valuesTemp, values, values.size()/cols, cols));
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
