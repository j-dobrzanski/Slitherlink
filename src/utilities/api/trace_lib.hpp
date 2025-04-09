#include <iostream>
#include <assert.h>

#ifndef TRACE_LIB_H
#define TRACE_LIB_H

#define ERROR(...); line_wrapper_error(__FILE__, __func__, __LINE__, __VA_ARGS__);

// #define LOG(...); line_wrapper_log(__FILE__, __func__, __LINE__, __VA_ARGS__);
#define LOG(...); 

#define LOG_DEBUG(...); line_wrapper_log(__FILE__, __func__, __LINE__, __VA_ARGS__);

void line();

template<typename First, typename ...Rest>
void line(First && first, Rest && ...rest){
    std::cout << std::forward<First>(first) << " ";
    line(std::forward<Rest>(rest)...);
}

template<typename ...Args>
void line_wrapper_error(Args&&... args){
    std::cerr << "ERROR!" << std::endl;
    line(args...);
    std::cerr << std::endl;
}

template<typename ...Args>
void line_wrapper_log(Args&&... args){
    std::cerr << "LOGGING" << std::endl;
    line(args...);
    std::cerr << std::endl;
}

#endif