#include "../api/trace_lib.hpp"

static void line(){}

template<typename First, typename ...Rest>
void line(First && first, Rest && ...rest){
    std::cout << std::forward<First>(first);
    line(std::forward<Rest>(rest)...);
}

void ERROR(){}
