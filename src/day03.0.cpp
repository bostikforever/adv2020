#include <cassert>
#include <fstream>   // for string, char_traits, operator<<, operator>>
#include <iostream>

namespace {

class Solution {

    static constexpr int delta = 3;

    int solution = 0;
    int pos = 0;

    bool solve(const std::string& line)
    {

        bool valid = line[pos] == '#';
        solution += valid;

        pos += delta;
        pos %= line.size();
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
