#include <algorithm>
#include <bitset>
#include <fstream>
#include <iostream>
#include <numeric>
#include <string>
#include <tuple>
#include <set>
#include <map>

namespace {

class Solution {

    static constexpr char active = '#';
    static constexpr char inactive = '.';
    static constexpr int8_t directions[] = {-1, 0, 1};

    uint64_t solution = 0;
    int calls = 0;

    template <typename CALLABLE>
    bool solve(CALLABLE&& loop)
    {
        ++calls;

        using Coord = std::tuple<int, int, int>;

        std::set<Coord> actives;
        int rows = 0;
        const auto ret = loop([&](const std::string& line) {
            for (int i = 0; i < line.size(); ++i) {
                switch (line[i]) {
                  case active:
                    actives.emplace(rows, i, 0);
                    break;
                  case inactive:
                    break;
                  default:
                    assert(false);
                }
            }
            ++rows;
            return true;
        });
        std::set<Coord> next_actives;
        std::map<Coord, int> inactive_active_count;
        for (int i = 0; i < 6; ++i) {
            for (const auto& coord: actives) {
                int active_neighs = 0;
                for (const auto x: directions) {
                for (const auto y: directions) {
                for (const auto z: directions) {
                    if (x == y & y == z && z == 0) {
                        continue;
                    }
                    const auto neigh = std::make_tuple(std::get<0>(coord) + x,
                                                       std::get<1>(coord) + y,
                                                       std::get<2>(coord) + z);
                    if (actives.count(neigh)) {
                        active_neighs++;
                    }
                    else {
                        inactive_active_count[neigh]++;
                    }
                } } }
                switch (active_neighs) {
                  case 2:
                  case 3:
                     next_actives.emplace(coord);
                     break;
                }
            }
            for (const auto& coordCount: inactive_active_count) {
                auto& coord = coordCount.first;
                if (coordCount.second != 3) {
                    continue;
                }
                switch (coordCount.second) {
                  case 3:
                    next_actives.emplace(coordCount.first);
                    break;
                }
            }
            actives.swap(next_actives);
            next_actives.clear();
            inactive_active_count.clear();
        }
        // for (const auto& coord: actives) {
        //     std::cout << std::get<0>(coord) <<','
        //               << std::get<1>(coord) <<','
        //               << std::get<2>(coord) << '\n';
        // }
        solution = actives.size();
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
