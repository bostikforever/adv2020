#include <cassert>
#include <cstdint>
#include <deque>
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <utility>

namespace {

class Solution {

    uint64_t solution = 0;
    int calls = 0;

    using NumType = uint8_t;
    using Deck = std::deque<NumType>;
    enum Winner {e_LEFT, e_RIGHT};

    template <typename DECK>
    std::pair<Winner, Deck *> game(DECK&& deck1, DECK&& deck2)
    {
      std::map<Deck, uint32_t> seen1;
      std::map<Deck, uint32_t> seen2;
      int round = 0;
      while (!deck1.empty() && !deck2.empty()) {
        ++round;
        Deck *winner, *loser;
        auto& count1 = seen1[deck1];
        auto& count2 = seen2[deck2];

        const auto old1 = count1;
        const auto old2 = count2;

        count1 = round;
        count2 = round;

        if (0 < old1 && old1 < round || 0 < old2 && old2 < round) {
          return std::make_pair(e_LEFT, &deck1);
        }
        else if (deck1.front() < deck1.size() &&
                 deck2.front() < deck2.size() ) {
          // std::cout << deck1.front() << ", "  << deck2.front() << '\n';
          const auto res = game(Deck{ deck1.begin() + 1,
                                      deck1.begin() + deck1.front() + 1},
                                Deck{ deck2.begin() + 1,
                                      deck2.begin() + deck2.front() + 1});
          switch(res.first) {
            case e_LEFT:
              winner = &deck1;
              loser = &deck2;
              break;
            case e_RIGHT:
              winner = &deck2;
              loser = &deck1;
          }
        }
        else {
          if (deck1.front() > deck2.front()) {
            winner = &deck1;
            loser = &deck2;
          }
          else {
            winner = &deck2;
            loser = &deck1;
          }
        }
        winner->push_back(winner->front());
        winner->pop_front();
        winner->push_back(loser->front());
        loser->pop_front();
      }

      if (deck2.empty()) {
        return std::make_pair(e_LEFT, &deck1);
      }
      return std::make_pair(e_RIGHT, &deck2);
    }

    template <typename CALLABLE>
    bool solve(CALLABLE&& loop)
    {
        ++calls;
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

        const Deck& winner = *(game(deck1, deck2).second);
        assert (!winner.empty());
        for (int i = 0; i < winner.size(); ++i) {
          std::cout << int(winner[i]) << ", ";
          solution += winner[i] * (winner.size() - i);
        }
        std::cout << '\n';
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
