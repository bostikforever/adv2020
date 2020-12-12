#include <cassert>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <regex>
#include <stack>
#include <string>
#include <string_view>
#include <unordered_map>

namespace hash_tuple { // http://coliru.stacked-crooked.com/a/fcc23b6af1db5a99
template <typename TT>
struct hash {
  size_t operator()(TT const& tt) const {
      return std::hash<TT>()(tt);
  }
};

template <class T>
inline void hash_combine(std::size_t& seed, T const& v) {
  seed ^= hash_tuple::hash<T>()(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

namespace details {
  // Recursive template code derived from Matthieu M.
  template <class Tuple, size_t Index = std::tuple_size<Tuple>::value - 1>
  struct HashValueImpl {
    void operator()(size_t& seed, Tuple const& tuple)const{
      HashValueImpl<Tuple, Index-1>{}(seed, tuple);
      hash_combine(seed, std::get<Index>(tuple));
    }
  };
  template <class Tuple>
  struct HashValueImpl<Tuple,0> {
    void operator()(size_t& seed, Tuple const& tuple)const{
      hash_combine(seed, std::get<0>(tuple));
    }
  };
}

template <typename ... TT>
struct hash<std::tuple<TT...>> {
  size_t operator()(std::tuple<TT...> const& tt) const {
    size_t seed = 0;
    details::HashValueImpl<std::tuple<TT...> >{}(seed, tt);
    return seed;
  }
};
}

namespace {

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

template <typename T>
struct KeyType {
  using type = T;
};

template <>
struct KeyType<std::string_view> {
  using type = std::string;
};

template <typename T>
using key_type_t = typename KeyType<T>::type;

template <typename R, typename... Args>
struct WithCache {
  using ArgTup = std::tuple<key_type_t<std::decay_t<Args>>...>;
  using Cache = std::unordered_map<ArgTup, R, hash_tuple::hash<ArgTup>>;

  std::function<R(Args...)> func;
  Cache cache;

  template<typename... CallArgs>
  auto operator()(CallArgs&& ...callArgs) {
    auto args = std::make_tuple(key_type_t<CallArgs>(
                                         std::forward<CallArgs>(callArgs))...);
    auto it  = cache.find(args);
    if (it != cache.end()) {
      return it->second;
    }
    it = cache.emplace(std::move(args), func(callArgs...)).first;
    return it->second;
  }
};

template <typename R, typename... Args>
WithCache<R, Args...> withCache(std::function<R(Args...)>&& func)
{
  return WithCache<R, Args...> { std::move(func) };
}

const auto withCaching = [](auto&& outerFunc) {
  return [outerFunc](auto&& ...outerArg) {
    return withCache(std::function { outerFunc(outerArg...) });
  };
};

const auto constant = [](const std::string& constantToken)
{
  return [constantToken](std::string_view token)
  {
    // std::cout << "const: " << token << " vs. " << constantToken << '\n';
    return token.size() == constantToken.size() && token == constantToken;
  };
};

const auto alternative = withCaching([](auto& lhs, auto& rhs)
{
  return [lhs, rhs](std::string_view token)
  {
    return lhs(token) || rhs(token);
  };
});

const auto concat = withCaching([](auto& lhs, auto& rhs)
{
  return [lhs, rhs](std::string_view token)
  {
    const auto begin = std::begin(token);
    const auto end = std::end(token);
    auto it = begin;
    while(it != end) {
      if (lhs(std::string_view {&*begin, std::size_t(it - begin)}) &&
          rhs(std::string_view {&*it, std::size_t(end - it)})) {
        // std::cout << "concat matched: " << token << '\n';
        return true;
      }
      ++it;
    }
    return lhs(token) && rhs(std::string_view {});
  };
});

using Rule = std::pair<int, std::vector<std::string>>;
using Rules = std::stack<Rule,
                         std::vector<Rule>>;
using Parser = std::function<bool(std::string_view)>;
using RuleParsers = std::unordered_map<int, Parser>;

auto buildParser(const std::vector<std::string>&    rule,
                 const std::shared_ptr<RuleParsers>& ruleParsers)
{
  enum Op { e_CONCAT, e_ALT };
  std::stack<Op> opStack;
  std::stack<Parser> argStack;

  const auto parserAt = [ruleParsers](int ruleNum) {
    return Parser([ruleParsers, ruleNum](std::string_view token) {
      return ruleParsers->at(ruleNum)(token);
    });
  };

  const auto apply = [](Op op, auto lhs, auto rhs) {
    switch (op) {
      case e_ALT:
        return Parser(alternative(lhs, rhs));
      case e_CONCAT:
        return Parser(concat(lhs, rhs));
    }
  };

  const auto consume = [&apply](auto& argStack, auto& opStack) {
    auto accum = argStack.top();
    argStack.pop();
    while (!argStack.empty()) {
      assert(!opStack.empty());
      assert(opStack.top() == e_ALT);
      accum = apply(opStack.top(), argStack.top(), accum);
      opStack.pop();
      argStack.pop();
    }
    return accum;
  };

  for (const auto& token: rule) {
    Parser arg;
    if (token.size() == 1) {
      switch(token[0]) {
        case ' ':
          opStack.push(e_CONCAT);
          assert(!argStack.empty());
          continue;
        case '|':
          opStack.push(e_ALT);
          assert(!argStack.empty());
          continue;
        default: // number
          auto ruleNum = std::stoi(token);
          arg = parserAt(ruleNum);
      }
    }
    else {
      if (token[0] == '"') { // string literal
        assert(token[token.length() - 1] == '"');
        arg = constant(token.substr(1, token.length() - 2));
      }
      else { // number
        auto ruleNum = std::stoi(token);
        // std::cout << "ruleNum: " << ruleNum << '\n';
        arg = parserAt(ruleNum);
      }
    }

    if (!opStack.empty() && opStack.top() == e_CONCAT) { // can combine
      assert(!argStack.empty());
      const Op op = opStack.top();
      arg = apply(op, argStack.top(), arg);
      opStack.pop();
      argStack.pop();
    }
    argStack.emplace(arg);
  }
  const auto res = consume(argStack, opStack);
  assert(argStack.empty());
  assert(opStack.empty());
  return res;
}

auto buildParser(Rules rules)
{
  auto ruleParsers = std::make_shared<RuleParsers>();
  while (!rules.empty()) {
    auto [idx, rule] = rules.top();
    rules.pop();
    (*ruleParsers)[idx] = buildParser(rule, ruleParsers);
  }
  return ruleParsers->at(0);
}

class Solution {

    static constexpr std::size_t N = 20;

    uint64_t solution = 0;
    int calls = 0;

    std::vector<Rules::value_type> rules;

    template <typename CALLABLE>
    bool solve(CALLABLE&& loop)
    {
        ++calls;
        const auto ret = loop(overloaded {
          // build parser
          [&](int ruleNum, auto&& inner) {
            std::cout << ruleNum << " ";
            auto& rule = rules.emplace_back();
            rule.first = ruleNum;
            inner([&](const std::string& token) {
              rule.second.emplace_back(token);
              std::cout << token << " ";// << token.size();
            });
            std::cout << '\n';
            return true;
          },
          // parse input
          [&](auto&& inner) {
            std::cout << "====== break ======" << '\n';
            std::sort(rules.begin(), rules.end());
            const auto parser = buildParser(Rules { std::move(rules) });
            inner([&](const std::string& line) {
              // FAIL: should be able to construct the parser outside the loop
              // and just somehow reset the cache.
              const auto valid = parser(line);
              std::cout << line << " " << valid << '\n';
              solution += valid;
            });
            return true;
          }
          });
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
    std::vector<uint16_t> lineNumbers;

    while (inputFile && callable([&](auto&& outer) {
      bool res = true;
      while (getline(inputFile, line) && !line.empty() && res) {
        const std::regex
              token_re(R"<kk>(((\d+): )|((\d+)|( (\|) )|( )|(\"[ab]\")))<kk>");
        std::sregex_token_iterator it(line.begin(),
                                      line.end(),
                                      token_re,
                                      {2, 4, 6, 7, 8});
        res = outer(
          std::stoi(*(it++)),
          [&](auto&& inner) {
            while (it != std::sregex_token_iterator()) {
              if (it->length() == 0) {
                ++it;
                continue;
              }
              inner(*(it++));
            }
        });
      }
      res = outer([&](auto&& inner) {
        while (getline(inputFile, line) && !line.empty() && res) {
          inner(line);
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
