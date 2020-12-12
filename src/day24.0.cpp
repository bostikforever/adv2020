#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <set>

namespace {

class Solution {

    int solution = 0;
    int calls = 0;

    enum Color { e_WHITE, e_BLACK };
    enum Dir { e_SW, e_SE, e_NW, e_NE, e_W, e_E };

    using Pos = std::pair<int, int>;

    Pos add(Pos lhs, Pos rhs) {
        return Pos {lhs.first + rhs.first, lhs.second + rhs.second};
    }

    Pos addToPos(Pos pos, Dir dir)
    {
        switch (dir) {
          case e_W:
            return add(pos, Pos(-2, 0));
          case e_E:
            return add(pos, Pos(2, 0));
          case e_NW:
            return add(pos, Pos(-1, 1));
          case e_NE:
            return add(pos, Pos(1, 1));
          case e_SW:
            return add(pos, Pos(-1, -1));
          case e_SE:
            return add(pos, Pos(1, -1));
        }
    }

    template <typename CALLABLE>
    bool solve(CALLABLE&& loop)
    {
        ++calls;
        std::set<Pos> blacks;
        const auto ret = loop([&](const std::string& line) {
            auto it = line.begin();
            Dir dir;
            Pos pos(0, 0); //start from reference;
            while (it != line.end()) {
                if (*it == 's' || *it == 'n') {
                    // std::cout << *it << *std::next(it) << ' ';
                    switch (*it) {
                      case 's':
                        switch((std::advance(it, 1), *it)) {
                          case 'e':
                            dir = e_SE;
                            break;
                          case 'w':
                            dir = e_SW;
                            break;
                        }
                        break;
                      case 'n':
                        switch((std::advance(it, 1), *it)) {
                          case 'e':
                            dir = e_NE;
                            break;
                          case 'w':
                            dir = e_NW;
                            break;
                        }
                        break;
                    }
                }
                else {
                    // std::cout << *it << ' ';
                    switch(*it) {
                      case 'e':
                        dir = e_E;
                        break;
                      case 'w':
                        dir = e_W;
                        break;
                    }
                }
                ++it;
                // std::cout << '\n';
                pos = addToPos(pos, dir);
            }
            // std::cout << pos.first <<", " << pos.second << '\n';
            const auto colorIt = blacks.find(pos);
            if (colorIt != blacks.end()) {
                blacks.erase(colorIt);
            }
            else {
                blacks.emplace(pos);
            }
            return true;
        });
        solution = blacks.size();
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
