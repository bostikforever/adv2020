#include <algorithm>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace {

class Solution {

    int solution = 0;
    int calls = 0;

    template <typename CALLABLE>
    bool solve(CALLABLE&& loop)
    {
        ++calls;
        std::optional<std::vector<char>> intersect;
        const auto ret = loop([&intersect](std::string line) {
            std::sort(line.begin(), line.end());
            if(!intersect) {
                intersect.emplace(line.begin(), line.end());
                return true;
            }
            std::vector<char> intersect_temp;
            std::set_intersection(intersect->begin(),
                                  intersect->end(),
                                  line.begin(),
                                  line.end(),
                                  std::back_inserter(intersect_temp));
            (*intersect) = std::move(intersect_temp);
            return !intersect->empty();
        });
        assert(bool(intersect));
        solution += intersect->size();
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

            // exhaust remaining group input
            while (!line.empty() && getline(inputFile, line))
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
