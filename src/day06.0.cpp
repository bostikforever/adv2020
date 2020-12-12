#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>

namespace {

class Solution {

    int solution = 0;
    int calls = 0;

    template <typename CALLABLE>
    bool solve(CALLABLE&& loop)
    {
        ++calls;
        std::string seen;
        const auto ret = loop([&](const std::string& line) {
            seen.reserve(seen.size() + line.size());
            seen += line;
            return true;
        });
        std::sort(seen.begin(), seen.end());
        const auto uniqueEnd = std::unique(seen.begin(), seen.end());
        solution += uniqueEnd - seen.begin();
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
