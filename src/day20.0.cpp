#include <algorithm>
#include <array>
#include <bitset>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <numeric>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace {

class Solution {
    using NumType = uint16_t;

    struct Tile {
        NumType top;
        NumType right;
        NumType bottom;
        NumType left;
    };

    static constexpr auto N = 10;

    int calls = 0;
    int sideSize = 0;

    using Cell = std::pair<int, int>; //id, orientation
    using TopLeft = std::pair<int, int>; // top, left
    std::map<Cell, Tile> edgesMap;
    std::unordered_multimap<int , Cell> leftToCell;
    std::unordered_multimap<int , Cell> topToCell;
    std::map<std::pair<int, int>, Cell> topLeftToCell;

    NumType edgeToNum(const std::string& bits) {
        return std::bitset<N>(bits, 0, bits.length(), '.', '#').to_ullong();
    }

    template <typename FUNC>
    void forEachRotation(std::string top,
                         std::string right,
                         std::string bottom,
                         std::string left,
                         FUNC&& func)
    {
        int i = 0;
        do {
            Tile tile { edgeToNum(top),
                        edgeToNum(right),
                        edgeToNum(bottom),
                        edgeToNum(left) };

            func(tile);
            using std::swap;
            // rotate
            swap(left, bottom);
            swap(bottom, right);
            swap(right, top);
            std::reverse(top.begin(), top.end());
            std::reverse(bottom.begin(), bottom.end());
            ++i;
        } while (i != 4);

    }

    void fill(int id, int orientation, Tile tile) {
        // std::cout << id << ", " << orientation << ", "
        //           << "{ " << tile.top
        //           << ", " << tile.right
        //           << ", " << tile.bottom
        //           << ", " << tile.left << " }\n";
        Cell cell{id, orientation};
        edgesMap.emplace(cell, tile);
        leftToCell.emplace(tile.left, cell);
        topToCell.emplace(tile.top, cell);
        TopLeft topLeft {tile.top, tile.left};
        topLeftToCell.emplace(topLeft, cell);

    }

    void fillAllOrientations(int         id,
                             std::string top,
                             std::string right,
                             std::string bottom,
                             std::string left)
    {
        // do normal orientation
        int orient = 0;
        forEachRotation(top,
                        right,
                        bottom,
                        left,
                        [&orient, id, this](Tile tile) {
            fill(id, orient, tile);
            ++orient;
        });

        // flip
        std::reverse(top.begin(), top.end());
        std::reverse(bottom.begin(), bottom.end());
        swap(left, right);
        forEachRotation(top,
                        right,
                        bottom,
                        left,
                        [&orient, id, this](Tile tile) {
            fill(id, orient, tile);
            ++orient;
        });
    }

    template <typename CALLABLE>
    bool solve(int id, CALLABLE&& loop)
    {
        ++calls;
        std::string top;
        std::string right;
        std::string bottom;
        std::string left;
        int i = 0;
        const auto ret = loop([&](const std::string& line) {
            if (i == 0) {
                top = line;
            }
            if (i == N-1) {
                bottom = line;
            }
            left += line[0];
            right += line[N-1];
            ++i;
            return true;
        });
        fillAllOrientations(id, top, right, bottom, left);
        return ret;
    }

    std::pair<int, int> toRowCol(int idx, int cols) const
    {
        return {idx / cols, idx % cols};
    }

    int toIndex(int row, int col, int cols) const
    {
        return row * cols + col;
    }

    bool search(std::deque<Cell>&       solution,
                std::unordered_set<int>& usedIds) const
    {

        // for (const auto cell : solution) {
        //     std::cout << "sideSize: " << sideSize;
        //     std::cout << " id: " << cell.first
        //        << " orientation: " << cell.second
        //        << '\n';
        // }

        const auto index = solution.size();

        if (index == calls) {
            return true;
        }

        auto& currCell = solution.emplace_back();
        if (index == 0) {
            bool found = false;
            for(const auto& val : edgesMap) {
                const auto cell = val.first;
                currCell = cell;
                const auto it = usedIds.emplace(cell.first);
                assert(it.second);
                found = search(solution, usedIds);
                if (found) break;
                usedIds.erase(it.first);
            }
            if (!found) {
                solution.pop_back();
            }
            return found;
        }
        const auto searchRange = [&](auto range) {
            bool found = false;
            for (auto it = range.first; it != range.second; ++it) {
                const Cell cell = it->second;
                auto usedIt = usedIds.find(cell.first);
                if (usedIt != usedIds.end()) {
                    continue; // already used
                }
                currCell = cell;
                usedIt = usedIds.emplace(cell.first).first;
                found = search(solution, usedIds);
                if (found) break;
                usedIds.erase(usedIt);
            }
            if (!found) {
                solution.pop_back();
            }
            return found;
        };

        const auto [row, col] = toRowCol(index, sideSize);
        if (row == 0) {
            Cell leftCell = solution[toIndex(row, col - 1, sideSize)];
            const auto it = edgesMap.find(leftCell);
            NumType left = it->second.right;
            const auto range = leftToCell.equal_range(left);
            return searchRange(range);
        }
        if (col == 0) {
            Cell topCell = solution[toIndex(row - 1, col, sideSize)];
            const auto it = edgesMap.find(topCell);
            NumType top = it->second.bottom;
            const auto range = topToCell.equal_range(top);
            return searchRange(range);
        }
        // interior tile
        // get required top and required left
        Cell leftCell = solution[toIndex(row, col - 1, sideSize)];
        const auto leftIt = edgesMap.find(leftCell);
        NumType left = leftIt->second.right;
        Cell topCell = solution[toIndex(row - 1, col, sideSize)];
        const auto rightIt = edgesMap.find(topCell);
        NumType top = rightIt->second.bottom;
        TopLeft topLeft {top, left};
        const auto range = topLeftToCell.equal_range(topLeft);
        return searchRange(range);
    }

  public:
    template <typename ...Args>
    bool operator()(Args... args)
    {
        return solve(args...);
    }

    void output(std::ostream& os) // breaks with other solutions by doing
                                  // actual computation here
    {
        sideSize = sqrt(calls);
        os << "calls: " << calls << '\n';
        std::deque<Cell> solution;
        std::unordered_set<int> usedIds;
        bool found = search(solution, usedIds);
        os << "found: " << found << '\n';
        if (found) {
            for (const auto cell : solution) {
                os << "id: " << cell.first
                   << " orientation: " << cell.second
                   << '\n';
            }
            std::array<Cell, 4> corners {
                                  solution[toIndex(0, 0, sideSize)],
                                  solution[toIndex(0, sideSize - 1, sideSize)],
                                  solution[toIndex(sideSize - 1, 0, sideSize)],
                                  solution[toIndex(sideSize - 1,
                                                    sideSize - 1, sideSize)] };
            auto part1 = std::accumulate(corners.begin(),
                                         corners.end(),
                                         1ULL,
                                         [](auto lhs, auto rhs){
                                            return lhs * rhs.first;
                                         });
            os << "part1: " << part1 << '\n';
        }
    }
};

template <typename CALLABLE>
void readBatches(const std::string& filename, CALLABLE&& callable)
{
    std::ifstream inputFile(filename);
    std::string line;
    int id;
    while (inputFile >> line >> id) {
        inputFile.clear();
        inputFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        callable(id, [&](auto&& body) {
            assert(id != 0);
            bool res;
            while (getline(inputFile, line) &&
                   !line.empty() &&
                   (res = body(line)))
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
    solution.output(std::cout);
    return 0;
}
