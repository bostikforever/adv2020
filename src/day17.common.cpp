#include <algorithm>
#include <bitset>
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <set>
#include <string>
#include <tuple>
#include <type_traits>

namespace {

class Solution {

    static constexpr char active = '#';
    static constexpr char inactive = '.';
    static constexpr int8_t directions[] = { -1, 0, 1 };
    static constexpr int N = 3;

    template <typename T, std::size_t dummy>
    using Repeat = T;

    template <template <class...> typename C, typename T, typename Seq>
    struct Expand { };

    template <template <class...> typename C, typename T, std::size_t... I>
    struct Expand<C, T, std::index_sequence<I...>> {
        using type = C<Repeat<T, I>...>;
    };

    template <typename T, std::size_t N>
    using CoordND =
                typename Expand<std::tuple,
                                T,
                                std::make_index_sequence<N>>::type;

    uint64_t solution = 0;
    int calls = 0;

    template <typename... T, std::size_t... I>
    std::ostream& printCoordHelper(std::ostream&           os,
                                   const std::tuple<T...>& coord,
                                   std::index_sequence<I...>)
    {
        (void) std::initializer_list<std::ostream *> {
                                    &( os << std::get<I>(coord) << ", " )... };
        return os;
    }

    template <typename... T>
    std::ostream& printCoord(std::ostream& os, const std::tuple<T...>& coord)
    {
        constexpr auto N = sizeof...(T);
        os << '(';
        printCoordHelper(os, coord, std::make_index_sequence<N - 1>());
        os << std::get<N - 1>(coord) << ')';
        return os;
    }

    template <typename FUNC,
              typename... T,
              typename... S>
    void forEachImpl(const std::tuple<T...>& inCoord,
                     const std::tuple<S...>& outCoord,
                     FUNC&& func)
    {
        if constexpr(sizeof...(T) != sizeof...(S)) {
        constexpr auto I = sizeof...(S);
        for (auto w: directions) {
            forEachImpl(inCoord,
                        std::tuple_cat(
                                   outCoord,
                                   std::make_tuple(std::get<I>(inCoord) + w )),
                        func);
        }
        } else {
        if (inCoord != outCoord) {
            func(outCoord);
        }
        return;
        }
    }

    template <typename FUNC, typename... T>
    void forEach(const std::tuple<T...>& coord, FUNC&& func)
    {
        forEachImpl(coord,
                    std::tuple<>{},
                    //std::make_tuple(std::get<0>(coord)),
                    func);
    }

    template <typename CALLABLE>
    bool solve(CALLABLE&& loop)
    {
        ++calls;

        using Coord = CoordND<int, N>;

        std::set<Coord> actives;
        int rows = 0;
        const auto ret = loop([&](const std::string& line) {
            for (int i = 0; i < line.size(); ++i) {
                switch (line[i]) {
                  case active:
                    actives.emplace(rows, i);
                                           // implicit zero for the other dims
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
                forEach(coord, [&](auto&& neigh) {
                    if (actives.count(neigh)) {
                        active_neighs++;
                    }
                    else {
                        inactive_active_count[neigh]++;
                    }
                });
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
        //     printCoord(std::cout, coord);
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
