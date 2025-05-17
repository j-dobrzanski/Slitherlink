#include "../api/solver.hpp"

// Local function definitions
bool isOuter(slitherlink_edge* edge_p) {
    return (edge_p->face_ids[0] == -1 ||
            edge_p->face_ids[1] == -1);
}

void Solver::generateAndSaveRules(std::string file_name,
                                  std::ptrdiff_t base_size) {
    (void) file_name;
    generateBase(base_size);

}

void Solver::generateBase(std::ptrdiff_t base_size) {
    if (base_size == 2) {

        slitherlink_vertex* vertex = new slitherlink_vertex();
        vertex->id = 0;
        vertex->no_of_edges = 3;



        slitherlink_face* face = new slitherlink_face();
        face->id = 0;
        face->value = 0;
        face->no_of_edges = 6;
    }
}
