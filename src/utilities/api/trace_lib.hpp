#ifndef TRACE_LIB_H
#define TRACE_LIB_H

template<typename ...Args>
void ERROR(Args&&... args){
    std::cerr << "ERROR! File:" << __FILE__ << " Function: " << __func__ << " Line: " << __LINE__ << " - ";
    line(args...);
    std::cerr << std::endl;
}

#endif