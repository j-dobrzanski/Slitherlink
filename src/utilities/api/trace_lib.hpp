#include <iostream>

#ifndef TRACE_LIB_H
#define TRACE_LIB_H

void line();

template<typename First, typename ...Rest>
void line(First && first, Rest && ...rest){
    std::cout << std::forward<First>(first);
    line(std::forward<Rest>(rest)...);
}

template<typename ...Args>
void ERROR(Args&&... args){
    std::cerr << "ERROR! File:" << __FILE__ << " Function: " << __func__ << " Line: " << __LINE__ << " - ";
    line(args...);
    std::cerr << std::endl;
}

#endif