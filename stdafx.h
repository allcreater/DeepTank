#include <glm/glm.hpp>
#include <glm/gtx/vec_swizzle.hpp>

#include <algorithm>
#include <cassert>
#include <chrono>
#include <deque>
#include <functional>
#include <future>
#include <list>
#include <mutex>
#include <optional>
#include <random>
#include <ranges>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

// overloaded
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

using namespace std::literals;