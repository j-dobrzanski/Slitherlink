#include "../api/slitherlink_template.hpp"
#include "../../../utilities/api/trace_lib.hpp"
#include <string>
#include <fstream>

template<typename T_edge, typename T_face>
SlitherlinkTemplate<T_edge, T_face>::SlitherlinkTemplate(){
    V = 0;
    E = 0;
    F = 0;
    edges = {edge(INVALID_INDEX,INVALID_INDEX)};
    faces = {face(INVALID_INDEX, {})};
}

template<typename T_edge, typename T_face>
SlitherlinkTemplate<T_edge, T_face>::SlitherlinkTemplate(std::size_t V,
                                         std::size_t E,
                                         std::vector<T_edge> edges,
                                         std::size_t F,
                                         std::vector<T_face> faces)
    : V{ V },
      E{ E },
      edges{ edges },
      F{ F },
      faces{ faces }{}
