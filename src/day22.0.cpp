#include <cassert>
#include <cstdint>
#include <deque>
#include <fstream>
#include <iostream>
#include <numeric>
#include <string>

namespace {

class Solution {

    uint64_t solution = 0;
    int calls = 0;

    template <typename CALLABLE>
    bool solve(CALLABLE&& loop)
    {
        ++calls;
        using NumType = uint16_t;
        using Deck = std::deque<NumType>;
        Deck deck1;
        Deck deck2;

        const auto ret = loop([&](auto&& player1,
                                  auto&& player2) {
            player1([&](NumType num) {
              // std::cout << num << '\n';
              deck1.push_back(num);
            });
            // std::cout << "break" << '\n';
            player2([&](NumType num) {
              // std::cout << num << '\n';
              deck2.push_back(num);
            });
            // std::cout << '\n';
            return true;
        });
        while (!deck1.empty() && !deck2.empty()) {
          Deck *bigger, *smaller;
          if (deck1.front() > deck2.front()) {
            bigger = &deck1;
            smaller = &deck2;
          }
          else {
            bigger = &deck2;
            smaller = &deck1;
          }
          bigger->push_back(bigger->front());
          bigger->pop_front();
          bigger->push_back(smaller->front());
          smaller->pop_front();
        }

        const Deck& winner = deck1.empty() ? deck2 : deck1;
        assert (!winner.empty());
        for (int i = 0; i < winner.size(); ++i) {
          solution += winner[i] * (winner.size() - i);
        }
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

    while (inputFile && callable([&](auto&& outer) {
      bool res = true;
      // std::cout << line << '\n';
      res = outer(
        [&](auto&& player1) {
          getline(inputFile, line);
          while (getline(inputFile, line) && !line.empty()) {
            player1(std::stoi(line));
          }
        },
        [&](auto&& player2) {
          getline(inputFile, line);
          while (getline(inputFile, line) && !line.empty()) {
            player2(std::stoi(line));
          }
        });
      return res;
    }));
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
