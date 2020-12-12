#include <cassert>
#include <fstream>
#include <iostream>
#include <unordered_set>

namespace {

class Solution {

    static constexpr int target = 2020;

    std::unordered_set<int> seen;

    int solution = 0;

    bool solve(int num)
    {
        if (seen.count(target - num)) {
            solution = num;
            return true;
        }
        seen.emplace(num);
        return true;
    }

  public:
    bool operator()(int num)
    {
        return solve(num);
    }

    int output() const
    {
        return (target - solution) * solution;
    }
};

template <typename CALLABLE>
void readLine(const std::string& filename, CALLABLE&& callable)
{
    std::ifstream inputFile(filename);
    int num;
    while (inputFile >> num && callable(num))
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
