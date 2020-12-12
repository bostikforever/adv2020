#include <array>
#include <cassert>
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <experimental/iterator>

namespace {

class Solution {

    static constexpr std::size_t N = 20;

    std::string solution;
    int calls = 0;

    template <typename CALLABLE>
    bool solve(CALLABLE&& loop)
    {
        ++calls;

        std::unordered_map<std::string, int> foodCount;
        std::map<std::string, std::set<std::string>> allergenSet;

        const auto ret = loop([&](auto&& innerFoods,
                                  auto&& innerAllergens) {
            std::set<std::string> foodSet;
            innerFoods([&](const std::string& food) {
                // std::cout << food << ": ";
                ++foodCount[food];
                foodSet.insert(food);
            });
            innerAllergens([&](const std::string& allergen) {
                // std::cout << allergen << ": ";
                const auto it = allergenSet.emplace(allergen, foodSet).first;
                std::set<std::string> intersect;
                std::set_intersection(it->second.begin(),
                                      it->second.end(),
                                      foodSet.begin(),
                                      foodSet.end(),
                                      std::inserter(intersect,
                                                    intersect.begin()));
                it->second.swap(intersect);
            });
            // std::cout << '\n';
            return true;
        });

        std::vector<std::pair<std::string, std::string>> foodWithAllergens;
        for (int i = 0; i < allergenSet.size(); ++i) {
            auto it = std::find_if(allergenSet.begin(),
                                   allergenSet.end(),
                                   [](auto value) {
                                    return value.second.size() == 1;
                                   });
            auto food = *it->second.begin();
            foodWithAllergens.emplace_back(it->first, food);
            for (auto& value : allergenSet) {
                value.second.erase(food);
            }
        }
        std::sort(foodWithAllergens.begin(), foodWithAllergens.end());
        std::vector<std::string> canonical;
        std::transform(foodWithAllergens.begin(),
                       foodWithAllergens.end(),
                       std::back_inserter(canonical),
                       [](auto val) { return val.second; });
        std::ostringstream oss;
        std::copy(canonical.begin(),
                  canonical.end(),
                  std::experimental::make_ostream_joiner(oss, ","));
        solution = oss.str();
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
      while(getline(inputFile, line)) {
        std::istringstream lineStream(line);
        res = outer(
          [&](auto&& foods) {
            while (lineStream >> line && line != "(contains") {
              foods(line);
            }
          },
          [&](auto&& allergens) {
            while (lineStream >> line) {
                allergens(line.substr(0, line.size() - 1));
            }
          });
      }
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
