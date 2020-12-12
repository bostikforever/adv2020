#include <cassert>
#include <fstream>   // for char_traits, operator>>
#include <iostream>

namespace {

class Solution {

    int solution = 0;

    bool solve(int lo, int hi, char ch, const std::string& pass)
    {
        bool valid = (pass[lo - 1] == ch) ^ (pass[hi - 1] == ch);
        solution += valid;
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
    int lo, hi;
    char ch, blank;
    std::string password;
    while (inputFile >> lo >> blank >> hi >> ch
                     >> blank >> password
           && callable(lo, hi, ch, password))
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
