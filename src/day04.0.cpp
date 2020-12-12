#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <iterator>
#include <set>
#include <string>
#include <vector>

namespace {

class Solution {

    static const std::set<std::string> expected;

    static const std::string exempted;

    int solution = 0;
    int calls = 0;

    bool solve(const std::set<std::string>& keys)
    {
        ++calls;
        std::vector<std::string> missing;
        std::set_difference(
                            expected.begin(),
                            expected.end(),
                            keys.begin(),
                            keys.end(),
                            std::back_inserter(missing));
        const int num_missing = missing.size();
        const bool exempted_missing = std::count(missing.begin(),
                                                      missing.end(),
                                                      exempted);
        // std:: cout << "num: " << num_missing
        //            << " exm:" << exempted_missing << '\n';
        solution += not bool (num_missing - exempted_missing);
        return true;
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

const std::string Solution::exempted = "cid";
const std::set<std::string> Solution::expected { "byr",
                                                 "iyr",
                                                 "eyr",
                                                 "hgt",
                                                 "hcl",
                                                 "ecl",
                                                 "pid",
                                                 "cid"  };

bool readKeys(std::set<std::string> *keys, const std::string& line)
{
    auto begin = line.cbegin();
    while (begin != line.cend()) {
        const auto end = std::find(begin, line.cend(), ':');
        const auto token = keys->emplace(begin, end);
        // assert(token.first->size() == 3)
        // std::cout << *token.first << ' ';
        // discard value associated with key
        begin = std::find(end, line.cend(), ' ');
        if (begin != line.cend()) {
            begin++;
        }
    }
    // std::cout << '\n';
    return true; // always a success?
}

template <typename CALLABLE>
void readLine(const std::string& filename, CALLABLE&& callable)
{
    std::ifstream inputFile(filename);
    std::string line;
    std::set<std::string> keys;
    while (getline(inputFile, line)) {
        if (line.empty()) {
            callable(keys);
            keys.clear();
            continue;
        }
        readKeys(&keys, line);
    }
    if(!keys.empty()) {
        callable(keys);
    }
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
