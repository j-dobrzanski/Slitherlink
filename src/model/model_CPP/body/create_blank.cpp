#include <cassert>

#include "../api/slitherlink.hpp"
#include "../api/parser.hpp"

inline void verticeLayer(std::ptrdiff_t layer_no,
                         std::vector<slitherlink_vertex*>& vertices,
                         std::ptrdiff_t* last_id,
                         std::ptrdiff_t size);

inline void edgeLayer(std::ptrdiff_t layer_no,
                      std::vector<slitherlink_edge*>& edges,
                      std::ptrdiff_t* last_id,
                      std::ptrdiff_t size);

inline void edgeLayerInner(std::ptrdiff_t layer_no,
                           std::vector<slitherlink_edge*>& edges,
                           std::ptrdiff_t* last_id,
                           std::ptrdiff_t size);

inline void edgeLayerOuter(std::ptrdiff_t layer_no,
                           std::vector<slitherlink_edge*>& edges,
                           std::ptrdiff_t* last_id,
                           std::ptrdiff_t size);

inline void faceLayer(std::ptrdiff_t layer_no,
                      std::vector<slitherlink_face*>& faces,
                      std::ptrdiff_t* last_id,
                      std::ptrdiff_t size);

Slitherlink::Slitherlink(std::ptrdiff_t size){
    std::size_t new_params_bitmap = 0;
    std::ptrdiff_t new_no_of_vertices = 6 * size * size;
    new_params_bitmap += V_PRESENT;
    std::ptrdiff_t new_no_of_edges = 9 * size * size - 3 * size;
    new_params_bitmap += E_PRESENT;
    std::ptrdiff_t new_no_of_faces = 3 * size * size - 3 * size + 2; // additional 1 for outer face
    new_params_bitmap += F_PRESENT;
    std::vector<slitherlink_vertex*> new_vertices(new_no_of_vertices);
    std::vector<slitherlink_edge*> new_edges(new_no_of_edges);
    std::vector<slitherlink_face*> new_faces(new_no_of_faces);

    // initialize vertices
    LOG_DEBUG("Generating vertices");
    std::ptrdiff_t current_vertice_id = 0;
    for (std::ptrdiff_t layer = 0; layer < size; ++layer) {
        LOG_DEBUG("   Vertice layer: ", layer);
        verticeLayer(layer, new_vertices, &current_vertice_id, size);
    }
    assert(current_vertice_id == new_no_of_vertices);
    new_params_bitmap += LIST_OF_VERTICES_PRESENT;
    LOG_DEBUG("Vertices generated: ", current_vertice_id);

    // initialize edges
    LOG_DEBUG("Generating edges");
    std::ptrdiff_t current_edge_id = 0;
    for (std::ptrdiff_t layer = 0; layer < size; ++layer) {
        LOG_DEBUG("   Edge layer: ", layer);
        edgeLayer(layer, new_edges, &current_edge_id, size);
    }
    assert(current_edge_id == new_no_of_edges);
    new_params_bitmap += LIST_OF_EDGES_PRESENT;
    LOG_DEBUG("Edges generated: ", current_edge_id);

    // initialize faces
    LOG_DEBUG("Generating faces");
    std::ptrdiff_t current_face_id = 0;
    for (std::ptrdiff_t layer = 0; layer < size; ++layer) {
        LOG_DEBUG("   Face layer: ", layer);
        faceLayer(layer, new_faces, &current_face_id, size);
    }
    // add outer face
    slitherlink_face* outer_face = new slitherlink_face{
        .id = OUTER_FACE,
        .value = 0,
        .no_of_edges = 0,
        .edge_ids = {},
        .face_ids = {},
        .edge_refs = {},
        .face_refs = {}
    };
    new_faces[current_face_id] = outer_face;
    current_face_id++;
    new_params_bitmap += LIST_OF_FACES_PRESENT;
    LOG_DEBUG("Faces generated: ", current_face_id);
    assert(current_face_id == new_no_of_faces);
    // // iterate over all faces
    // for (std::ptrdiff_t i = 0; i < current_face_id; ++i) {
    //     // iterate over all edgs in face
    //     for (std::ptrdiff_t j = 0; j < faces[i]->no_of_edges; ++j) {
    //         // check if edge is in face
    //         std::ptrdiff_t edge_id = faces[i]->edge_ids[j];
    //         std::ptrdiff_t face_id = edges[edge_id]->face_ids[0] == i ?
    //                                   edges[edge_id]->face_ids[1] :
    //                                   edges[edge_id]->face_ids[0];
    //         bool found = false;
    //         for (std::ptrdiff_t k = 0; k < faces[face_id]->no_of_edges; ++k) {
    //             if (faces[face_id]->edge_ids[k] == edge_id) {
    //                 found = true;
    //                 break;
    //             }
    //         }
    //         if (!found) {
    //             LOG("Edge ", edge_id, " not found in face ", face_id);
    //         }
    //     }
    // }

    // initialize slitherlink

    this->params_bitmap = new_params_bitmap;
    this->no_of_vertices = new_no_of_vertices;
    this->no_of_edges = new_no_of_edges;
    this->no_of_faces = new_no_of_faces;
    this->vertices = new_vertices;
    this->edges = new_edges;
    this->faces = new_faces;
    assert(this->evaluateReferences() == 0);
    assert(this->checkCorrectness() == 0);
}


inline void verticeLayer(std::ptrdiff_t layer_no,
                         std::vector<slitherlink_vertex*>& vertices,
                         std::ptrdiff_t* last_id,
                         std::ptrdiff_t size) {
    std::ptrdiff_t id = *last_id;
    if (size == 1) { // special case if only one layer exists
        vertices[0] = new slitherlink_vertex{
            .id = 0,
            .no_of_edges = 2,
            .edge_ids = {5, 1},
            .edge_refs = {}
        };
        for (std::ptrdiff_t i = 1; i < 5; ++i) {
            vertices[i] = new slitherlink_vertex{
                .id = i,
                .no_of_edges = 2,
                .edge_ids = {i - 1, i + 1},
                .edge_refs = {}
            };
        }
        vertices[5] = new slitherlink_vertex{
            .id = 5,
            .no_of_edges = 2,
            .edge_ids = {4, 0},
            .edge_refs = {}
        };
        id = 6;
    }
    else if (layer_no == 0) { // special case for first layer
        vertices[0] = new slitherlink_vertex{
            .id = 0,
            .no_of_edges = 3,
            .edge_ids = {5, 0, 6},
            .edge_refs = {}
        };
        for (std::ptrdiff_t i = 1; i < 6; ++i) {
            vertices[i] = new slitherlink_vertex{
                .id = i,
                .no_of_edges = 3,
                .edge_ids = {i - 1, i, 6 + i},
                .edge_refs = {}
            };
        }
        id = 6;
    }
    else if (layer_no == size - 1) { // special case for last layer
        // first half of upper side
        std::ptrdiff_t first_id =  9 * (layer_no + 1) * (layer_no + 1) - 3 * (layer_no + 1) - 1;
        std::ptrdiff_t second_id = 9 * layer_no * layer_no + 3 * layer_no;
        std::ptrdiff_t third_id = layer_no % 2 == 1 ?
                                  9 * layer_no * layer_no - 3 * layer_no :
                                  -1;
        if (third_id != -1) {
            vertices[id] = new slitherlink_vertex{
                .id = id,
                .no_of_edges = 3,
                .edge_ids = {first_id, second_id, third_id},
                .edge_refs = {}
            };
        }
        else {
            vertices[id] = new slitherlink_vertex{
                .id = id,
                .no_of_edges = 2,
                .edge_ids = {first_id, second_id},
                .edge_refs = {}
            };
        }
        id++;
        for (std::ptrdiff_t i = 1; i < layer_no + 1; ++i) {
            std::ptrdiff_t first_id =  9 * layer_no * layer_no + 3 * layer_no + i - 1;
            std::ptrdiff_t second_id = 9 * layer_no * layer_no + 3 * layer_no + i;
            std::ptrdiff_t third_id = (layer_no % 2 + i % 2) % 2 == 1 ?
                                      9 * layer_no * layer_no - 3 * layer_no + i / 2:
                                      -1;
            if (third_id != -1) {
                vertices[id] = new slitherlink_vertex{
                    .id = id,
                    .no_of_edges = 3,
                    .edge_ids = {first_id, second_id, third_id},
                    .edge_refs = {}
                };
            }
            else {
                vertices[id] = new slitherlink_vertex{
                    .id = id,
                    .no_of_edges = 2,
                    .edge_ids = {first_id, second_id},
                    .edge_refs = {}
                };
            }
            id++;
        }

        // five sides
        for (std::ptrdiff_t side = 0; side < 5; ++side) {
            for (std::ptrdiff_t i = 0; i < 2 * layer_no + 1; ++i) {
                std::ptrdiff_t first_id =  9 * layer_no * layer_no + 3 * layer_no + layer_no * (2 * side + 1) + side + i;
                std::ptrdiff_t second_id = 9 * layer_no * layer_no + 3 * layer_no + layer_no * (2 * side + 1) + side + i + 1;
                std::ptrdiff_t third_id = i % 2 == 1 ?
                                          9 * layer_no * layer_no - 3 * layer_no + (2 * side * layer_no + layer_no + i) / 2:
                                          -1;
                if (third_id != -1) {
                    vertices[id] = new slitherlink_vertex{
                        .id = id,
                        .no_of_edges = 3,
                        .edge_ids = {first_id, second_id, third_id},
                        .edge_refs = {}
                    };
                }
                else {
                    vertices[id] = new slitherlink_vertex{
                        .id = id,
                        .no_of_edges = 2,
                        .edge_ids = {first_id, second_id},
                        .edge_refs = {}
                    };
                }
                id++;
            }
        }

        // second half of upper side 
        for (std::ptrdiff_t i = 0; i < layer_no - 1; ++i) {
            std::ptrdiff_t first_id =  9 * layer_no * layer_no + 3 * layer_no + 11 * layer_no + 5 + i;
            std::ptrdiff_t second_id = 9 * layer_no * layer_no + 3 * layer_no + 11 * layer_no + 5 + i + 1;
            std::ptrdiff_t third_id = i % 2 == 1 ?
                                      9 * layer_no * layer_no - 3 * layer_no + (11 * layer_no + i) / 2 :
                                      -1;
            if (third_id != -1) {
                vertices[id] = new slitherlink_vertex{
                    .id = id,
                    .no_of_edges = 3,
                    .edge_ids = {first_id, second_id, third_id},
                    .edge_refs = {}
                };
            }
            else {
                vertices[id] = new slitherlink_vertex{
                    .id = id,
                    .no_of_edges = 2,
                    .edge_ids = {first_id, second_id},
                    .edge_refs = {}
                };
            }
            id++;
        }
        first_id = 9 * (layer_no + 1) * (layer_no + 1) - 3 * (layer_no + 1) - 2;
        second_id = 9 * (layer_no + 1) * (layer_no + 1) - 3 * (layer_no + 1) - 1;
        third_id = layer_no % 2 == 0 ?
                    9 * layer_no * layer_no - 3 * layer_no + (12 * layer_no - 1) / 2 :
                    -1;
        if (third_id != -1) {
            vertices[id] = new slitherlink_vertex{
                .id = id,
                .no_of_edges = 3,
                .edge_ids = {first_id, second_id, third_id},
                .edge_refs = {}
            };
        }
        else {
            vertices[id] = new slitherlink_vertex{
                .id = id,
                .no_of_edges = 2,
                .edge_ids = {first_id, second_id},
                .edge_refs = {}
            };
        }
        id++;

    }
    else { // standard case
        // first half of upper side
        std::ptrdiff_t first_id =  9 * (layer_no + 1) * (layer_no + 1) - 3 * (layer_no + 1) - 1;
        std::ptrdiff_t second_id = 9 * layer_no * layer_no + 3 * layer_no;
        std::ptrdiff_t third_id = layer_no % 2 == 1 ?
                                  9 * layer_no * layer_no - 3 * layer_no :
                                  9 * (layer_no + 1) * (layer_no + 1) - 3 * (layer_no + 1);
        vertices[id] = new slitherlink_vertex{
            .id = id,
            .no_of_edges = 3,
            .edge_ids = {first_id, second_id, third_id},
            .edge_refs = {}
        };
        id++;
        for (std::ptrdiff_t i = 1; i < layer_no + 1; ++i) {
            std::ptrdiff_t first_id =  9 * layer_no * layer_no + 3 * layer_no + i - 1;
            std::ptrdiff_t second_id = 9 * layer_no * layer_no + 3 * layer_no + i;
            std::ptrdiff_t third_id = (layer_no % 2 + i % 2) % 2 == 1 ?
                                      9 * layer_no * layer_no - 3 * layer_no + i / 2:
                                      9 * (layer_no + 1) * (layer_no + 1) - 3 * (layer_no + 1) + i / 2;
            vertices[id] = new slitherlink_vertex{
                .id = id,
                .no_of_edges = 3,
                .edge_ids = {first_id, second_id, third_id},
                .edge_refs = {}
            };
            id++;
        }

        // five sides
        for (std::ptrdiff_t side = 0; side < 5; ++side) {
            for (std::ptrdiff_t i = 0; i < 2 * layer_no + 1; ++i) {
                std::ptrdiff_t first_id =  9 * layer_no * layer_no + 3 * layer_no + layer_no * (2 * side + 1) + side + i;
                std::ptrdiff_t second_id = 9 * layer_no * layer_no + 3 * layer_no + layer_no * (2 * side + 1) + side + i + 1;
                std::ptrdiff_t third_id = i % 2 == 1 ?
                                          9 * layer_no * layer_no - 3 * layer_no + (2 * side * layer_no + layer_no + i) / 2:
                                          9 * (layer_no + 1) * (layer_no + 1) - 3 * (layer_no + 1) + (2 * side * layer_no + layer_no + 2 * side + i) / 2 + 1;
                vertices[id] = new slitherlink_vertex{
                    .id = id,
                    .no_of_edges = 3,
                    .edge_ids = {first_id, second_id, third_id},
                    .edge_refs = {}
                };
                id++;
            }
        }

        // second half of upper side 
        for (std::ptrdiff_t i = 0; i < layer_no - 1; ++i) {
            std::ptrdiff_t first_id =  9 * layer_no * layer_no + 3 * layer_no + 11 * layer_no + 5 + i;
            std::ptrdiff_t second_id = 9 * layer_no * layer_no + 3 * layer_no + 11 * layer_no + 5 + i + 1;
            std::ptrdiff_t third_id = i % 2 == 1 ?
                                      9 * layer_no * layer_no - 3 * layer_no + (11 * layer_no + i) / 2 :
                                      9 * (layer_no + 1) * (layer_no + 1) - 3 * (layer_no + 1) + (11 * layer_no + i + 10) / 2 + 1;
            vertices[id] = new slitherlink_vertex{
                .id = id,
                .no_of_edges = 3,
                .edge_ids = {first_id, second_id, third_id},
                .edge_refs = {}
            };
            id++;
        }
        first_id = 9 * (layer_no + 1) * (layer_no + 1) - 3 * (layer_no + 1) - 2;
        second_id = 9 * (layer_no + 1) * (layer_no + 1) - 3 * (layer_no + 1) - 1;
        third_id = layer_no % 2 == 0 ?
                    9 * layer_no * layer_no - 3 * layer_no + (12 * layer_no - 1) / 2 :
                    9 * (layer_no + 1) * (layer_no + 1) - 3 * (layer_no + 1) + (12 * layer_no + 8) / 2 + 1;
        vertices[id] = new slitherlink_vertex{
            .id = id,
            .no_of_edges = 3,
            .edge_ids = {first_id, second_id, third_id},
            .edge_refs = {}
        };
        id++;

    }
    *last_id = id;
}

inline void edgeLayer(std::ptrdiff_t layer_no,
                      std::vector<slitherlink_edge*>& edges,
                      std::ptrdiff_t* last_id,
                      std::ptrdiff_t size) {
    edgeLayerInner(layer_no, edges, last_id, size);
    edgeLayerOuter(layer_no, edges, last_id, size);
}

inline void edgeLayerInner(std::ptrdiff_t layer_no,
                           std::vector<slitherlink_edge*>& edges,
                           std::ptrdiff_t* last_id,
                           std::ptrdiff_t size) {
    std::ptrdiff_t id = *last_id;
    if (size == 1) { // special case if only one layer exists
        // no inner layer in case of size 1
    }
    else if (layer_no == 0) { // special case for first layer
        // no inner layer in case of first layer
    }
    else { // standard case
        // first half of upper side
        std::ptrdiff_t vertice_first_id = 6 * (layer_no - 1) * (layer_no - 1) + (layer_no + 1) % 2;
        std::ptrdiff_t vertice_second_id = 6 * layer_no * layer_no + (layer_no + 1) % 2;
        std::ptrdiff_t face_first_id = layer_no % 2 == 0 ?
                                        3 * layer_no * layer_no - 3 * layer_no + 1 :
                                        3 * (layer_no + 1) * (layer_no + 1) - 3 * (layer_no + 1);
        std::ptrdiff_t face_second_id = 3 * layer_no * layer_no - 3 * layer_no + 1 + (layer_no + 1) % 2;
        edges[id] = new slitherlink_edge{
            .id = id,
            .vertices = {vertice_first_id, vertice_second_id},
            .face_ids = {face_first_id, face_second_id},
            .vertex_refs = {},
            .face_refs = {},
            .solution = EDGE_UNKNOWN
        };
        id++;
        for (std::ptrdiff_t i = 1; i < (layer_no + 1) / 2; ++i) {
            std::ptrdiff_t vertice_first_id =  6 * (layer_no - 1) * (layer_no - 1) + 2 * i + (layer_no + 1) % 2;
            std::ptrdiff_t vertice_second_id = 6 * layer_no * layer_no + 2 * i + (layer_no + 1) % 2;
            std::ptrdiff_t face_first_id = 3 * layer_no * layer_no - 3 * layer_no + (layer_no + 1) % 2 + i;
            std::ptrdiff_t face_second_id = 3 * layer_no * layer_no - 3 * layer_no + (layer_no + 1) % 2 + 1 + i;
            edges[id] = new slitherlink_edge{
                .id = id,
                .vertices = {vertice_first_id, vertice_second_id},
                .face_ids = {face_first_id, face_second_id},
                .vertex_refs = {},
                .face_refs = {},
                .solution = EDGE_UNKNOWN
            };
            id++;
        }
        // five sides
        for (std::ptrdiff_t side = 0; side < 5; ++side) {
            for (std::ptrdiff_t i = 0; i < layer_no; ++i) {
                std::ptrdiff_t vertice_first_id =  6 * (layer_no - 1) * (layer_no - 1) + layer_no + side * (2 * layer_no - 1) + 2 * i;
                std::ptrdiff_t vertice_second_id = 6 * layer_no * layer_no + layer_no + side * (2 * layer_no + 1) + 2 * i + 2;
                std::ptrdiff_t face_first_id = 3 * layer_no * layer_no - 3 * layer_no + layer_no / 2 + side * layer_no + 1 + i;
                std::ptrdiff_t face_second_id = 3 * layer_no * layer_no - 3 * layer_no + layer_no / 2 + side * layer_no + 2 + i;
                edges[id] = new slitherlink_edge{
                    .id = id,
                    .vertices = {vertice_first_id, vertice_second_id},
                    .face_ids = {face_first_id, face_second_id},
                    .vertex_refs = {},
                    .face_refs = {},
                    .solution = EDGE_UNKNOWN
                };
                id++;
            }
        }
        // second half of upper side
        for (std::ptrdiff_t i = 0; i < layer_no / 2 - 1; i++) {
            std::ptrdiff_t vertice_first_id =  6 * (layer_no - 1) * (layer_no - 1) + 11 * layer_no - 5 + 2 * i;
            std::ptrdiff_t vertice_second_id = 6 * layer_no * layer_no + 11 * layer_no + 7 + 2 * i;
            std::ptrdiff_t face_first_id =  3 * layer_no * layer_no - 3 * layer_no + layer_no / 2 + 5 * layer_no + i + 1;
            std::ptrdiff_t face_second_id =  3 * layer_no * layer_no - 3 * layer_no + layer_no / 2 + 5 * layer_no + i + 2;
            edges[id] = new slitherlink_edge{
                .id = id,
                .vertices = {vertice_first_id, vertice_second_id},
                .face_ids = {face_first_id, face_second_id},
                .vertex_refs = {},
                .face_refs = {},
                .solution = EDGE_UNKNOWN
            };
            if (id == 154) {
                LOG_DEBUG("id: ", id);
                LOG_DEBUG(*edges[id]);
            }
            id++;
        }
        if (layer_no > 1) {
            vertice_first_id = layer_no % 2 == 0 ?
                                6 * layer_no * layer_no - 1 :
                                6 * layer_no * layer_no - 2;
            vertice_second_id = layer_no % 2 == 0 ?
                                6 * (layer_no + 1) * (layer_no + 1) - 1 :
                                6 * (layer_no + 1) * (layer_no + 1) - 2;
            face_first_id = layer_no % 2 == 0 ?
                                3 * (layer_no + 1) * (layer_no + 1) - 3 * (layer_no + 1):
                                3 * (layer_no + 1) * (layer_no + 1) - 3 * (layer_no + 1) - 1;
            face_second_id = layer_no % 2 == 0 ?
                                3 * layer_no * layer_no - 3 * layer_no + 1 :
                                3 * (layer_no + 1) * (layer_no + 1) - 3 * (layer_no + 1);
            edges[id] = new slitherlink_edge{
                .id = id,
                .vertices = {vertice_first_id, vertice_second_id},
                .face_ids = {face_first_id, face_second_id},
                .vertex_refs = {},
                .face_refs = {},
                .solution = EDGE_UNKNOWN
            };
            id++;
        }
    }
    *last_id = id;
}

inline void edgeLayerOuter(std::ptrdiff_t layer_no,
                           std::vector<slitherlink_edge*>& edges,
                           std::ptrdiff_t* last_id,
                           std::ptrdiff_t size) {
    std::ptrdiff_t id = *last_id;
    if (size == 1) { // special case if only one layer exists
        edges[0] = new slitherlink_edge{
            .id = 0,
            .vertices = {0, 1},
            .face_ids = {0, OUTER_FACE},
            .vertex_refs = {},
            .face_refs = {},
            .solution = EDGE_UNKNOWN
        };
        edges[1] = new slitherlink_edge{
            .id = 1,
            .vertices = {1, 2},
            .face_ids = {0, OUTER_FACE},
            .vertex_refs = {},
            .face_refs = {},
            .solution = EDGE_UNKNOWN
        };
        edges[2] = new slitherlink_edge{
            .id = 2,
            .vertices = {2, 3},
            .face_ids = {0, OUTER_FACE},
            .vertex_refs = {},
            .face_refs = {},
            .solution = EDGE_UNKNOWN
        };
        edges[3] = new slitherlink_edge{
            .id = 3,
            .vertices = {3, 4},
            .face_ids = {0, OUTER_FACE},
            .vertex_refs = {},
            .face_refs = {},
            .solution = EDGE_UNKNOWN
        };
        edges[4] = new slitherlink_edge{
            .id = 4,
            .vertices = {4, 5},
            .face_ids = {0, OUTER_FACE},
            .vertex_refs = {},
            .face_refs = {},
            .solution = EDGE_UNKNOWN
        };
        edges[5] = new slitherlink_edge{
            .id = 5,
            .vertices = {5, 0},
            .face_ids = {0, OUTER_FACE},
            .vertex_refs = {},
            .face_refs = {},
            .solution = EDGE_UNKNOWN
        };
        id = 6;
    }
    else if (layer_no == 0) { // special case for first layer
        edges[0] = new slitherlink_edge{
            .id = 0,
            .vertices = {0, 1},
            .face_ids = {0, 1},
            .vertex_refs = {},
            .face_refs = {},
            .solution = EDGE_UNKNOWN
        };
        edges[1] = new slitherlink_edge{
            .id = 1,
            .vertices = {1, 2},
            .face_ids = {0, 2},
            .vertex_refs = {},
            .face_refs = {},
            .solution = EDGE_UNKNOWN
        };
        edges[2] = new slitherlink_edge{
            .id = 2,
            .vertices = {2, 3},
            .face_ids = {0, 3},
            .vertex_refs = {},
            .face_refs = {},
            .solution = EDGE_UNKNOWN
        };
        edges[3] = new slitherlink_edge{
            .id = 3,
            .vertices = {3, 4},
            .face_ids = {0, 4},
            .vertex_refs = {},
            .face_refs = {},
            .solution = EDGE_UNKNOWN
        };
        edges[4] = new slitherlink_edge{
            .id = 4,
            .vertices = {4, 5},
            .face_ids = {0, 5},
            .vertex_refs = {},
            .face_refs = {},
            .solution = EDGE_UNKNOWN
        };
        edges[5] = new slitherlink_edge{
            .id = 5,
            .vertices = {5, 0},
            .face_ids = {0, 6},
            .vertex_refs = {},
            .face_refs = {},
            .solution = EDGE_UNKNOWN
        };
        id = 6;
    }
    else if (layer_no == size - 1){ // special case for last layer
        // first half of upper side
        for (std::ptrdiff_t i = 0; i < layer_no; i++){
            std::ptrdiff_t vertice_first_id =  6 * layer_no * layer_no + i;
            std::ptrdiff_t vertice_second_id = 6 * layer_no * layer_no + i + 1;
            std::ptrdiff_t face_first_id = 3 * layer_no * layer_no - 3 * layer_no + 1 + (i + (layer_no - 1) % 2)/2;
            edges[id] = new slitherlink_edge{
                .id = id,
                .vertices = {vertice_first_id, vertice_second_id},
                .face_ids = {face_first_id, OUTER_FACE},
                .vertex_refs = {},
                .face_refs = {},
                .solution = EDGE_UNKNOWN
            };
            id++;            
        }
        std::ptrdiff_t vertice_first_id =  6 * layer_no * layer_no + layer_no;
        std::ptrdiff_t vertice_second_id = 6 * layer_no * layer_no + layer_no + 1;
        std::ptrdiff_t face_first_id = 3 * layer_no * layer_no - 3 * layer_no + 1 + (layer_no - 1 + (layer_no - 1) % 2)/2;
        edges[id] = new slitherlink_edge{
            .id = id,
            .vertices = {vertice_first_id, vertice_second_id},
            .face_ids = {face_first_id, OUTER_FACE},
            .vertex_refs = {},
            .face_refs = {},
            .solution = EDGE_UNKNOWN
        };
        id++;  
        // five sides
        for (std::ptrdiff_t side = 0; side < 5; side++) {
            for (std::ptrdiff_t i = 0; i < 2 * layer_no; i++) {
                std::ptrdiff_t vertice_first_id =  6 * layer_no * layer_no + layer_no + side * (2 * layer_no + 1) + i + 1;
                std::ptrdiff_t vertice_second_id = 6 * layer_no * layer_no + layer_no + side * (2 * layer_no + 1) + i + 2;
                std::ptrdiff_t face_first_id = 3 * layer_no * layer_no - 3 * layer_no + 1 + layer_no / 2 + side * layer_no + (i + 1)/2;
                edges[id] = new slitherlink_edge{
                    .id = id,
                    .vertices = {vertice_first_id, vertice_second_id},
                    .face_ids = {face_first_id, OUTER_FACE},
                    .vertex_refs = {},
                    .face_refs = {},
                    .solution = EDGE_UNKNOWN
                };
                id++;
            }
            std::ptrdiff_t vertice_first_id =  6 * layer_no * layer_no + 3 * layer_no + 1 + side * (2 * layer_no + 1);
            std::ptrdiff_t vertice_second_id = 6 * layer_no * layer_no + 3 * layer_no + 2 + side * (2 * layer_no + 1);
            std::ptrdiff_t face_first_id = 3 * layer_no * layer_no - 3 * layer_no + 1 + layer_no / 2 + side * layer_no + layer_no;
            edges[id] = new slitherlink_edge{
                .id = id,
                .vertices = {vertice_first_id, vertice_second_id},
                .face_ids = {face_first_id, OUTER_FACE},
                .vertex_refs = {},
                .face_refs = {},
                .solution = EDGE_UNKNOWN
            };
            id++;
        }
        // second half of upper side
        for (std::ptrdiff_t i = 0; i < layer_no - 1; i++){
            std::ptrdiff_t vertice_first_id =  6 * layer_no * layer_no + 11 * layer_no + 6 + i;
            std::ptrdiff_t vertice_second_id = 6 * layer_no * layer_no + 11 * layer_no + 6 + i + 1;
            std::ptrdiff_t face_first_id = 3 * layer_no * layer_no - 3 * layer_no + 1 + (layer_no / 2) + 5 * layer_no + (i + 1)/2;
            edges[id] = new slitherlink_edge{
                .id = id,
                .vertices = {vertice_first_id, vertice_second_id},
                .face_ids = {face_first_id, OUTER_FACE},
                .vertex_refs = {},
                .face_refs = {},
                .solution = EDGE_UNKNOWN
            };
            id++;
        }
        vertice_first_id = 6 * (layer_no + 1) * (layer_no + 1) - 1;
        vertice_second_id = 6 * layer_no * layer_no;
        face_first_id = layer_no % 2 == 0 ?
                            3 * layer_no * layer_no - 3 * layer_no + 1 :
                            3 * (layer_no + 1) * (layer_no + 1) - 3 * (layer_no + 1);
        edges[id] = new slitherlink_edge{
            .id = id,
            .vertices = {vertice_first_id, vertice_second_id},
            .face_ids = {face_first_id, OUTER_FACE},
            .vertex_refs = {},
            .face_refs = {},
            .solution = EDGE_UNKNOWN
        };
        id++;
    }
    else { // standard case
        // first half of upper side
        for (std::ptrdiff_t i = 0; i < layer_no; i++){
            std::ptrdiff_t vertice_first_id =  6 * layer_no * layer_no + i;
            std::ptrdiff_t vertice_second_id = 6 * layer_no * layer_no + i + 1;
            std::ptrdiff_t face_first_id = 3 * layer_no * layer_no - 3 * layer_no + 1 + (i + (layer_no - 1) % 2)/2;
            std::ptrdiff_t face_second_id = 3 * (layer_no + 1) * (layer_no + 1) - 3 * (layer_no + 1) + 1 + (i + layer_no % 2)/2;
            edges[id] = new slitherlink_edge{
                .id = id,
                .vertices = {vertice_first_id, vertice_second_id},
                .face_ids = {face_first_id, face_second_id},
                .vertex_refs = {},
                .face_refs = {},
                .solution = EDGE_UNKNOWN
            };
            id++;
        }
        std::ptrdiff_t vertice_first_id =  6 * layer_no * layer_no + layer_no;
        std::ptrdiff_t vertice_second_id = 6 * layer_no * layer_no + layer_no + 1;
        std::ptrdiff_t face_first_id = 3 * layer_no * layer_no - 3 * layer_no + 1 + (layer_no - 1 + (layer_no - 1) % 2)/2;
        std::ptrdiff_t face_second_id = 3 * (layer_no + 1) * (layer_no + 1) - 3 * (layer_no + 1) + 1 + (layer_no + 1) / 2;
        edges[id] = new slitherlink_edge{
            .id = id,
            .vertices = {vertice_first_id, vertice_second_id},
            .face_ids = {face_first_id, face_second_id},
            .vertex_refs = {},
            .face_refs = {},
            .solution = EDGE_UNKNOWN
        };
        id++;  
        // five sides
        for (std::ptrdiff_t side = 0; side < 5; side++) {
            for (std::ptrdiff_t i = 0; i < 2 * layer_no; i++) {
                std::ptrdiff_t vertice_first_id =  6 * layer_no * layer_no + layer_no + side * (2 * layer_no + 1) + i + 1;
                std::ptrdiff_t vertice_second_id = 6 * layer_no * layer_no + layer_no + side * (2 * layer_no + 1) + i + 2;
                std::ptrdiff_t face_first_id = 3 * layer_no * layer_no - 3 * layer_no + 1 + layer_no / 2 + side * layer_no + (i + 1)/2;
                std::ptrdiff_t face_second_id = 3 * (layer_no + 1) * (layer_no + 1) - 3 * (layer_no + 1) + 2 + (layer_no + 1) / 2 + side * (layer_no + 1) + i / 2;
                edges[id] = new slitherlink_edge{
                    .id = id,
                    .vertices = {vertice_first_id, vertice_second_id},
                    .face_ids = {face_first_id, face_second_id},
                    .vertex_refs = {},
                    .face_refs = {},
                    .solution = EDGE_UNKNOWN
                };
                id++;
            }
            std::ptrdiff_t vertice_first_id =  6 * layer_no * layer_no + 3 * layer_no + 1 + side * (2 * layer_no + 1);
            std::ptrdiff_t vertice_second_id = 6 * layer_no * layer_no + 3 * layer_no + 2 + side * (2 * layer_no + 1);
            std::ptrdiff_t face_first_id = 3 * layer_no * layer_no - 3 * layer_no + 1 + layer_no / 2 + side * layer_no + layer_no;
            std::ptrdiff_t face_second_id = 3 * (layer_no + 1) * (layer_no + 1) - 3 * (layer_no + 1) + 2 + (layer_no + 1) / 2 + side * (layer_no + 1) + layer_no;
            edges[id] = new slitherlink_edge{
                .id = id,
                .vertices = {vertice_first_id, vertice_second_id},
                .face_ids = {face_first_id, face_second_id},
                .vertex_refs = {},
                .face_refs = {},
                .solution = EDGE_UNKNOWN
            };
            id++;
        }
        // second half of upper side
        for (std::ptrdiff_t i = 0; i < layer_no - 1; i++){
            std::ptrdiff_t vertice_first_id =  6 * layer_no * layer_no + 11 * layer_no + 6 + i;
            std::ptrdiff_t vertice_second_id = 6 * layer_no * layer_no + 11 * layer_no + 6 + i + 1;
            std::ptrdiff_t face_first_id = 3 * layer_no * layer_no - 3 * layer_no + 1 + (layer_no / 2) + 5 * layer_no + (i + 1)/2;
            std::ptrdiff_t face_second_id = 3 * (layer_no + 1) * (layer_no + 1) - 3 * (layer_no + 1) + 2 + (layer_no + 1) / 2 + 5 * (layer_no + 1) + i / 2;
            edges[id] = new slitherlink_edge{
                .id = id,
                .vertices = {vertice_first_id, vertice_second_id},
                .face_ids = {face_first_id, face_second_id},
                .vertex_refs = {},
                .face_refs = {},
                .solution = EDGE_UNKNOWN
            };
            id++;
        }
        vertice_first_id = 6 * (layer_no + 1) * (layer_no + 1) - 1;
        vertice_second_id = 6 * layer_no * layer_no;
        face_first_id = layer_no % 2 == 0 ?
                            3 * layer_no * layer_no - 3 * layer_no + 1 :
                            3 * (layer_no + 1) * (layer_no + 1) - 3 * (layer_no + 1);
        face_second_id = layer_no % 2 == 0 ?
                            3 * (layer_no + 2) * (layer_no + 2) - 3 * (layer_no + 2) :
                            3 * (layer_no + 1) * (layer_no + 1) - 3 * (layer_no + 1) + 1;
        edges[id] = new slitherlink_edge{
            .id = id,
            .vertices = {vertice_first_id, vertice_second_id},
            .face_ids = {face_first_id, face_second_id},
            .vertex_refs = {},
            .face_refs = {},
            .solution = EDGE_UNKNOWN
        };
        id++;
    }
    *last_id = id;
}

inline void faceLayer(std::ptrdiff_t layer_no,
                      std::vector<slitherlink_face*>& faces,
                      std::ptrdiff_t* last_id,
                      std::ptrdiff_t size) {
    std::ptrdiff_t id = *last_id;
    if (layer_no == 0) { // special case for first layer
        faces[0] = new slitherlink_face{
            .id = 0,
            .value = 0,
            .no_of_edges = 6,
            .edge_ids = {0, 1, 2, 3, 4, 5},
            .face_ids = {OUTER_FACE, OUTER_FACE, OUTER_FACE, OUTER_FACE, OUTER_FACE, OUTER_FACE},
            .edge_refs = {},
            .face_refs = {},
        };
        id = 1;
    }
    else if (layer_no == size - 1) {
        std::ptrdiff_t lower_edge_curr_id = getNoOfEdges(layer_no - 1) + (layer_no - 1) * 6;
        std::ptrdiff_t middle_edge_curr_id = getNoOfEdges(layer_no);
        std::ptrdiff_t upper_edge_curr_id = getNoOfEdges(layer_no) + layer_no * 6;
        std::ptrdiff_t lower_face_curr_id = getNoOfFaces(layer_no - 1);
        std::ptrdiff_t middle_face_curr_id = getNoOfFaces(layer_no);
        std::ptrdiff_t upper_face_curr_id = getNoOfFaces(layer_no + 1);

        // first half of upper side
        if (layer_no % 2 == 0) {
            std::ptrdiff_t edge_first_id =  middle_edge_curr_id - 1;
            std::ptrdiff_t edge_second_id = upper_edge_curr_id - 1;
            std::ptrdiff_t edge_third_id =  getNoOfEdges(layer_no + 1) - 1;
            std::ptrdiff_t edge_fourth_id = lower_edge_curr_id;
            lower_edge_curr_id++;
            std::ptrdiff_t edge_fifth_id =  middle_edge_curr_id;
            std::ptrdiff_t edge_sixth_id = upper_edge_curr_id;
            upper_edge_curr_id++;

            std::ptrdiff_t face_first_id =  middle_face_curr_id - 1;
            std::ptrdiff_t face_second_id = upper_face_curr_id - 1;
            std::ptrdiff_t face_third_id =  OUTER_FACE;
            std::ptrdiff_t face_fourth_id = lower_face_curr_id;
            lower_face_curr_id++;
            std::ptrdiff_t face_fifth_id =  middle_face_curr_id;
            middle_face_curr_id++;
            std::ptrdiff_t face_sixth_id = OUTER_FACE;
            upper_face_curr_id++;
            faces[id] = new slitherlink_face{
                .id = id,
                .value = 0,
                .no_of_edges = 6,
                .edge_ids = {
                    edge_first_id,
                    edge_second_id,
                    edge_third_id,
                    edge_fourth_id,
                    edge_fifth_id,
                    edge_sixth_id
                },
                .face_ids = {
                    face_first_id,
                    face_second_id,
                    face_third_id,
                    face_fourth_id,
                    face_fifth_id,
                    face_sixth_id},
                .edge_refs = {},
                .face_refs = {},
            };
            id++;
        }
        else if (layer_no > 1) {
            std::ptrdiff_t edge_first_id =  lower_edge_curr_id;
            lower_edge_curr_id++;
            std::ptrdiff_t edge_second_id = middle_edge_curr_id;
            middle_edge_curr_id++;
            std::ptrdiff_t edge_third_id =  upper_edge_curr_id;
            upper_edge_curr_id++;
            std::ptrdiff_t edge_fourth_id = lower_edge_curr_id;
            lower_edge_curr_id++;
            std::ptrdiff_t edge_fifth_id =  middle_edge_curr_id;
            std::ptrdiff_t edge_sixth_id = upper_edge_curr_id;
            upper_edge_curr_id++;

            std::ptrdiff_t face_first_id =  lower_face_curr_id;
            lower_face_curr_id++;
            std::ptrdiff_t face_second_id = upper_face_curr_id - 1;
            std::ptrdiff_t face_third_id =  OUTER_FACE;
            upper_face_curr_id++;
            std::ptrdiff_t face_fourth_id = lower_face_curr_id;
            std::ptrdiff_t face_fifth_id =  middle_face_curr_id + 1;
            middle_face_curr_id++;
            std::ptrdiff_t face_sixth_id = OUTER_FACE;
            faces[id] = new slitherlink_face{
                .id = id,
                .value = 0,
                .no_of_edges = 6,
                .edge_ids = {
                    edge_first_id,
                    edge_second_id,
                    edge_third_id,
                    edge_fourth_id,
                    edge_fifth_id,
                    edge_sixth_id
                },
                .face_ids = {
                    face_first_id,
                    face_second_id,
                    face_third_id,
                    face_fourth_id,
                    face_fifth_id,
                    face_sixth_id},
                .edge_refs = {},
                .face_refs = {},
            };
            id++;
        }

        for (std::ptrdiff_t i = 1; i < layer_no / 2; i++) {
            std::ptrdiff_t edge_first_id =  lower_edge_curr_id;
            lower_edge_curr_id++;
            std::ptrdiff_t edge_second_id = middle_edge_curr_id;
            middle_edge_curr_id++;
            std::ptrdiff_t edge_third_id =  upper_edge_curr_id;
            upper_edge_curr_id++;
            std::ptrdiff_t edge_fourth_id = lower_edge_curr_id;
            lower_edge_curr_id++;
            std::ptrdiff_t edge_fifth_id = middle_edge_curr_id;
            std::ptrdiff_t edge_sixth_id = upper_edge_curr_id;
            upper_edge_curr_id++;

            std::ptrdiff_t face_first_id =  lower_face_curr_id;
            lower_face_curr_id++;
            std::ptrdiff_t face_second_id = middle_face_curr_id - 1;
            std::ptrdiff_t face_third_id = OUTER_FACE;
            upper_face_curr_id++;
            std::ptrdiff_t face_fourth_id = lower_face_curr_id;
            std::ptrdiff_t face_fifth_id = middle_face_curr_id + 1;
            middle_face_curr_id++;
            std::ptrdiff_t face_sixth_id = OUTER_FACE;
            faces[id] = new slitherlink_face{
                .id = id,
                .value = 0,
                .no_of_edges = 6,
                .edge_ids = {
                    edge_first_id,
                    edge_second_id,
                    edge_third_id,
                    edge_fourth_id,
                    edge_fifth_id,
                    edge_sixth_id
                },
                .face_ids = {
                    face_first_id,
                    face_second_id,
                    face_third_id,
                    face_fourth_id,
                    face_fifth_id,
                    face_sixth_id
                },
                .edge_refs = {},
                .face_refs = {},
            };
            id++;
        }

        std::ptrdiff_t edge_first_id =  lower_edge_curr_id;
        lower_edge_curr_id++;
        std::ptrdiff_t edge_second_id = middle_edge_curr_id;
        middle_edge_curr_id++;
        std::ptrdiff_t edge_third_id =  upper_edge_curr_id;
        upper_edge_curr_id++;
        std::ptrdiff_t edge_fourth_id = middle_edge_curr_id;
        std::ptrdiff_t edge_fifth_id = upper_edge_curr_id;
        upper_edge_curr_id++;
        std::ptrdiff_t edge_sixth_id = upper_edge_curr_id;
        upper_edge_curr_id++;

        std::ptrdiff_t face_first_id =  lower_face_curr_id;
        std::ptrdiff_t face_second_id = middle_face_curr_id - 1;
        std::ptrdiff_t face_third_id = OUTER_FACE;
        upper_face_curr_id++;
        std::ptrdiff_t face_fourth_id = middle_face_curr_id + 1;
        middle_face_curr_id++;
        std::ptrdiff_t face_fifth_id = OUTER_FACE;
        upper_face_curr_id++;
        std::ptrdiff_t face_sixth_id = OUTER_FACE;
        faces[id] = new slitherlink_face{
            .id = id,
            .value = 0,
            .no_of_edges = 6,
            .edge_ids = {
                edge_first_id,
                edge_second_id,
                edge_third_id,
                edge_fourth_id,
                edge_fifth_id,
                edge_sixth_id
            },
            .face_ids = {
                face_first_id,
                face_second_id,
                face_third_id,
                face_fourth_id,
                face_fifth_id,
                face_sixth_id
            },
            .edge_refs = {},
            .face_refs = {},
        };
        id++;

        // five sides
        for (std::ptrdiff_t side = 0; side < 5; ++side) {
            for (std::ptrdiff_t i = 0; i < layer_no - 1; ++i) {
                std::ptrdiff_t edge_first_id =  lower_edge_curr_id;
                lower_edge_curr_id++;
                std::ptrdiff_t edge_second_id = middle_edge_curr_id;
                middle_edge_curr_id++;
                std::ptrdiff_t edge_third_id =  upper_edge_curr_id;
                upper_edge_curr_id++;
                std::ptrdiff_t edge_fourth_id = lower_edge_curr_id;
                lower_edge_curr_id++;
                std::ptrdiff_t edge_fifth_id = middle_edge_curr_id;
                std::ptrdiff_t edge_sixth_id = upper_edge_curr_id;
                upper_edge_curr_id++;
    
                std::ptrdiff_t face_first_id =  lower_face_curr_id;
                lower_face_curr_id++;
                std::ptrdiff_t face_second_id = middle_face_curr_id - 1;
                std::ptrdiff_t face_third_id = OUTER_FACE;
                upper_face_curr_id++;
                std::ptrdiff_t face_fourth_id = lower_face_curr_id;
                std::ptrdiff_t face_fifth_id = middle_face_curr_id + 1;
                middle_face_curr_id++;
                std::ptrdiff_t face_sixth_id = OUTER_FACE;
                faces[id] = new slitherlink_face{
                    .id = id,
                    .value = 0,
                    .no_of_edges = 6,
                    .edge_ids = {
                        edge_first_id,
                        edge_second_id,
                        edge_third_id,
                        edge_fourth_id,
                        edge_fifth_id,
                        edge_sixth_id
                    },
                    .face_ids = {
                        face_first_id,
                        face_second_id,
                        face_third_id,
                        face_fourth_id,
                        face_fifth_id,
                        face_sixth_id
                    },
                    .edge_refs = {},
                    .face_refs = {},
                };
                id++;
            }
            if (side < 4 || layer_no > 2) {
                std::ptrdiff_t edge_first_id =  lower_edge_curr_id;
                lower_edge_curr_id++;
                std::ptrdiff_t edge_second_id = middle_edge_curr_id;
                middle_edge_curr_id++;
                std::ptrdiff_t edge_third_id =  upper_edge_curr_id;
                upper_edge_curr_id++;
                std::ptrdiff_t edge_fourth_id = middle_edge_curr_id;
                std::ptrdiff_t edge_fifth_id = upper_edge_curr_id;
                upper_edge_curr_id++;
                std::ptrdiff_t edge_sixth_id = upper_edge_curr_id;
                upper_edge_curr_id++;

                std::ptrdiff_t face_first_id =  lower_face_curr_id;
                std::ptrdiff_t face_second_id = middle_face_curr_id - 1;
                std::ptrdiff_t face_third_id = OUTER_FACE;
                upper_face_curr_id++;
                std::ptrdiff_t face_fourth_id = middle_face_curr_id + 1;
                middle_face_curr_id++;
                std::ptrdiff_t face_fifth_id = OUTER_FACE;
                upper_face_curr_id++;
                std::ptrdiff_t face_sixth_id = OUTER_FACE;
                faces[id] = new slitherlink_face{
                    .id = id,
                    .value = 0,
                    .no_of_edges = 6,
                    .edge_ids = {
                        edge_first_id,
                        edge_second_id,
                        edge_third_id,
                        edge_fourth_id,
                        edge_fifth_id,
                        edge_sixth_id
                    },
                    .face_ids = {
                        face_first_id,
                        face_second_id,
                        face_third_id,
                        face_fourth_id,
                        face_fifth_id,
                        face_sixth_id
                    },
                    .edge_refs = {},
                    .face_refs = {},
                };
                id++;                
            }

        }
        // second half of upper side
        for (std::ptrdiff_t i = 0; i < (layer_no - 1) / 2 - 1; ++i) {
            std::ptrdiff_t edge_first_id =  lower_edge_curr_id;
            lower_edge_curr_id++;
            std::ptrdiff_t edge_second_id = middle_edge_curr_id;
            middle_edge_curr_id++;
            std::ptrdiff_t edge_third_id =  upper_edge_curr_id;
            upper_edge_curr_id++;
            std::ptrdiff_t edge_fourth_id = lower_edge_curr_id;
            lower_edge_curr_id++;
            std::ptrdiff_t edge_fifth_id = middle_edge_curr_id;
            std::ptrdiff_t edge_sixth_id = upper_edge_curr_id;
            upper_edge_curr_id++;

            std::ptrdiff_t face_first_id =  lower_face_curr_id;
            lower_face_curr_id++;
            std::ptrdiff_t face_second_id = middle_face_curr_id - 1;
            std::ptrdiff_t face_third_id = OUTER_FACE;
            upper_face_curr_id++;
            std::ptrdiff_t face_fourth_id = lower_face_curr_id;
            std::ptrdiff_t face_fifth_id = middle_face_curr_id + 1;
            middle_face_curr_id++;
            std::ptrdiff_t face_sixth_id = OUTER_FACE;
            faces[id] = new slitherlink_face{
                .id = id,
                .value = 0,
                .no_of_edges = 6,
                .edge_ids = {
                    edge_first_id,
                    edge_second_id,
                    edge_third_id,
                    edge_fourth_id,
                    edge_fifth_id,
                    edge_sixth_id
                },
                .face_ids = {
                    face_first_id,
                    face_second_id,
                    face_third_id,
                    face_fourth_id,
                    face_fifth_id,
                    face_sixth_id
                },
                .edge_refs = {},
                .face_refs = {},
            };
            id++;
        }

        edge_first_id =  lower_edge_curr_id;
        lower_edge_curr_id++;
        edge_second_id = middle_edge_curr_id;
        middle_edge_curr_id++;
        edge_third_id =  upper_edge_curr_id;
        upper_edge_curr_id++;
        edge_fourth_id = layer_no != 2 ?
                            lower_edge_curr_id :
                            upper_edge_curr_id;
        if (layer_no == 2) {
            upper_edge_curr_id++;
        }
        lower_edge_curr_id++;
        edge_fifth_id = layer_no % 2 != 0 ?
                            getNoOfEdges(layer_no) :
                            middle_edge_curr_id;
        edge_sixth_id = upper_edge_curr_id;
        upper_edge_curr_id++;

        face_first_id =  lower_face_curr_id;
        lower_face_curr_id++;
        face_second_id = middle_face_curr_id - 1;
        face_third_id = OUTER_FACE;
        upper_face_curr_id++;
        face_fourth_id = lower_face_curr_id;
        face_fifth_id = getNoOfFaces(layer_no);
        middle_face_curr_id++;
        face_sixth_id = OUTER_FACE;
        faces[id] = new slitherlink_face{
            .id = id,
            .value = 0,
            .no_of_edges = 6,
            .edge_ids = {
                edge_first_id,
                edge_second_id,
                edge_third_id,
                edge_fourth_id,
                edge_fifth_id,
                edge_sixth_id
            },
            .face_ids = {
                face_first_id,
                face_second_id,
                face_third_id,
                face_fourth_id,
                face_fifth_id,
                face_sixth_id
            },
            .edge_refs = {},
            .face_refs = {},
        };
        id++;            


    LOG_DEBUG("upper_edge_current_id: ", upper_edge_curr_id);
    LOG_DEBUG("lower_edge_current_id: ", lower_edge_curr_id);
    LOG_DEBUG("middle_edge_current_id: ", middle_edge_curr_id);
    }
    else { // standard case
        std::ptrdiff_t lower_edge_curr_id = getNoOfEdges(layer_no - 1) + (layer_no - 1) * 6;
        std::ptrdiff_t middle_edge_curr_id = getNoOfEdges(layer_no);
        std::ptrdiff_t upper_edge_curr_id = getNoOfEdges(layer_no) + layer_no * 6;
        std::ptrdiff_t lower_face_curr_id = getNoOfFaces(layer_no - 1);
        std::ptrdiff_t middle_face_curr_id = getNoOfFaces(layer_no);
        std::ptrdiff_t upper_face_curr_id = getNoOfFaces(layer_no + 1);

        // first half of upper side
        if (layer_no % 2 == 0) {
            std::ptrdiff_t edge_first_id =  middle_edge_curr_id - 1;
            std::ptrdiff_t edge_second_id = upper_edge_curr_id - 1;
            std::ptrdiff_t edge_third_id =  getNoOfEdges(layer_no + 1) - 1;
            std::ptrdiff_t edge_fourth_id = lower_edge_curr_id;
            lower_edge_curr_id++;
            std::ptrdiff_t edge_fifth_id =  middle_edge_curr_id;
            std::ptrdiff_t edge_sixth_id = upper_edge_curr_id;
            upper_edge_curr_id++;

            std::ptrdiff_t face_first_id =  middle_face_curr_id - 1;
            std::ptrdiff_t face_second_id = upper_face_curr_id - 1;
            std::ptrdiff_t face_third_id =  getNoOfFaces(layer_no + 2) - 1;
            std::ptrdiff_t face_fourth_id = lower_face_curr_id;
            lower_face_curr_id++;
            std::ptrdiff_t face_fifth_id =  middle_face_curr_id;
            middle_face_curr_id++;
            std::ptrdiff_t face_sixth_id = upper_face_curr_id;
            upper_face_curr_id++;
            faces[id] = new slitherlink_face{
                .id = id,
                .value = 0,
                .no_of_edges = 6,
                .edge_ids = {
                    edge_first_id,
                    edge_second_id,
                    edge_third_id,
                    edge_fourth_id,
                    edge_fifth_id,
                    edge_sixth_id
                },
                .face_ids = {
                    face_first_id,
                    face_second_id,
                    face_third_id,
                    face_fourth_id,
                    face_fifth_id,
                    face_sixth_id},
                .edge_refs = {},
                .face_refs = {},
            };
            id++;
        }
        else if (layer_no > 1) {
            std::ptrdiff_t edge_first_id =  lower_edge_curr_id;
            lower_edge_curr_id++;
            std::ptrdiff_t edge_second_id = middle_edge_curr_id;
            middle_edge_curr_id++;
            std::ptrdiff_t edge_third_id =  upper_edge_curr_id;
            upper_edge_curr_id++;
            std::ptrdiff_t edge_fourth_id = lower_edge_curr_id;
            lower_edge_curr_id++;
            std::ptrdiff_t edge_fifth_id =  middle_edge_curr_id;
            std::ptrdiff_t edge_sixth_id = upper_edge_curr_id;
            upper_edge_curr_id++;

            std::ptrdiff_t face_first_id =  lower_face_curr_id;
            lower_face_curr_id++;
            std::ptrdiff_t face_second_id = upper_face_curr_id - 1;
            std::ptrdiff_t face_third_id =  upper_face_curr_id;
            upper_face_curr_id++;
            std::ptrdiff_t face_fourth_id = lower_face_curr_id;
            std::ptrdiff_t face_fifth_id =  middle_face_curr_id + 1;
            middle_face_curr_id++;
            std::ptrdiff_t face_sixth_id = upper_face_curr_id;
            faces[id] = new slitherlink_face{
                .id = id,
                .value = 0,
                .no_of_edges = 6,
                .edge_ids = {
                    edge_first_id,
                    edge_second_id,
                    edge_third_id,
                    edge_fourth_id,
                    edge_fifth_id,
                    edge_sixth_id
                },
                .face_ids = {
                    face_first_id,
                    face_second_id,
                    face_third_id,
                    face_fourth_id,
                    face_fifth_id,
                    face_sixth_id},
                .edge_refs = {},
                .face_refs = {},
            };
            id++;
        }

        for (std::ptrdiff_t i = 1; i < layer_no / 2; ++i) {
            std::ptrdiff_t edge_first_id =  lower_edge_curr_id;
            lower_edge_curr_id++;
            std::ptrdiff_t edge_second_id = middle_edge_curr_id;
            middle_edge_curr_id++;
            std::ptrdiff_t edge_third_id =  upper_edge_curr_id;
            upper_edge_curr_id++;
            std::ptrdiff_t edge_fourth_id = lower_edge_curr_id;
            lower_edge_curr_id++;
            std::ptrdiff_t edge_fifth_id = middle_edge_curr_id;
            std::ptrdiff_t edge_sixth_id = upper_edge_curr_id;
            upper_edge_curr_id++;

            std::ptrdiff_t face_first_id =  lower_face_curr_id;
            lower_face_curr_id++;
            std::ptrdiff_t face_second_id = middle_face_curr_id - 1;
            std::ptrdiff_t face_third_id = upper_face_curr_id;
            upper_face_curr_id++;
            std::ptrdiff_t face_fourth_id = lower_face_curr_id;
            std::ptrdiff_t face_fifth_id = middle_face_curr_id + 1;
            middle_face_curr_id++;
            std::ptrdiff_t face_sixth_id = upper_face_curr_id;
            faces[id] = new slitherlink_face{
                .id = id,
                .value = 0,
                .no_of_edges = 6,
                .edge_ids = {
                    edge_first_id,
                    edge_second_id,
                    edge_third_id,
                    edge_fourth_id,
                    edge_fifth_id,
                    edge_sixth_id
                },
                .face_ids = {
                    face_first_id,
                    face_second_id,
                    face_third_id,
                    face_fourth_id,
                    face_fifth_id,
                    face_sixth_id
                },
                .edge_refs = {},
                .face_refs = {},
            };
            id++;
        }

        std::ptrdiff_t edge_first_id =  lower_edge_curr_id;
        lower_edge_curr_id++;
        std::ptrdiff_t edge_second_id = middle_edge_curr_id;
        middle_edge_curr_id++;
        std::ptrdiff_t edge_third_id =  upper_edge_curr_id;
        upper_edge_curr_id++;
        std::ptrdiff_t edge_fourth_id = middle_edge_curr_id;
        std::ptrdiff_t edge_fifth_id = upper_edge_curr_id;
        upper_edge_curr_id++;
        std::ptrdiff_t edge_sixth_id = upper_edge_curr_id;
        upper_edge_curr_id++;

        std::ptrdiff_t face_first_id =  lower_face_curr_id;
        std::ptrdiff_t face_second_id = middle_face_curr_id - 1;
        std::ptrdiff_t face_third_id = upper_face_curr_id;
        upper_face_curr_id++;
        std::ptrdiff_t face_fourth_id = middle_face_curr_id + 1;
        middle_face_curr_id++;
        std::ptrdiff_t face_fifth_id = upper_face_curr_id;
        upper_face_curr_id++;
        std::ptrdiff_t face_sixth_id = upper_face_curr_id;
        faces[id] = new slitherlink_face{
            .id = id,
            .value = 0,
            .no_of_edges = 6,
            .edge_ids = {
                edge_first_id,
                edge_second_id,
                edge_third_id,
                edge_fourth_id,
                edge_fifth_id,
                edge_sixth_id
            },
            .face_ids = {
                face_first_id,
                face_second_id,
                face_third_id,
                face_fourth_id,
                face_fifth_id,
                face_sixth_id
            },
            .edge_refs = {},
            .face_refs = {},
        };
        id++;


        // five sides
        for (std::ptrdiff_t side = 0; side < 5; ++side) {
            for (std::ptrdiff_t i = 0; i < layer_no - 1; ++i) {
                std::ptrdiff_t edge_first_id =  lower_edge_curr_id;
                lower_edge_curr_id++;
                std::ptrdiff_t edge_second_id = middle_edge_curr_id;
                middle_edge_curr_id++;
                std::ptrdiff_t edge_third_id =  upper_edge_curr_id;
                upper_edge_curr_id++;
                std::ptrdiff_t edge_fourth_id = lower_edge_curr_id;
                lower_edge_curr_id++;
                std::ptrdiff_t edge_fifth_id = middle_edge_curr_id;
                std::ptrdiff_t edge_sixth_id = upper_edge_curr_id;
                upper_edge_curr_id++;
    
                std::ptrdiff_t face_first_id =  lower_face_curr_id;
                lower_face_curr_id++;
                std::ptrdiff_t face_second_id = middle_face_curr_id - 1;
                std::ptrdiff_t face_third_id = upper_face_curr_id;
                upper_face_curr_id++;
                std::ptrdiff_t face_fourth_id = lower_face_curr_id;
                std::ptrdiff_t face_fifth_id = middle_face_curr_id + 1;
                middle_face_curr_id++;
                std::ptrdiff_t face_sixth_id = upper_face_curr_id;
                faces[id] = new slitherlink_face{
                    .id = id,
                    .value = 0,
                    .no_of_edges = 6,
                    .edge_ids = {
                        edge_first_id,
                        edge_second_id,
                        edge_third_id,
                        edge_fourth_id,
                        edge_fifth_id,
                        edge_sixth_id
                    },
                    .face_ids = {
                        face_first_id,
                        face_second_id,
                        face_third_id,
                        face_fourth_id,
                        face_fifth_id,
                        face_sixth_id
                    },
                    .edge_refs = {},
                    .face_refs = {},
                };
                id++;
            }
            
            if (side < 4 || layer_no > 2) {
                std::ptrdiff_t edge_first_id =  lower_edge_curr_id;
                lower_edge_curr_id++;
                std::ptrdiff_t edge_second_id = middle_edge_curr_id;
                middle_edge_curr_id++;
                std::ptrdiff_t edge_third_id =  upper_edge_curr_id;
                upper_edge_curr_id++;
                std::ptrdiff_t edge_fourth_id = middle_edge_curr_id;
                std::ptrdiff_t edge_fifth_id = upper_edge_curr_id;
                upper_edge_curr_id++;
                std::ptrdiff_t edge_sixth_id = upper_edge_curr_id;
                upper_edge_curr_id++;

                std::ptrdiff_t face_first_id =  lower_face_curr_id;
                std::ptrdiff_t face_second_id = middle_face_curr_id - 1;
                std::ptrdiff_t face_third_id = upper_face_curr_id;
                upper_face_curr_id++;
                std::ptrdiff_t face_fourth_id = middle_face_curr_id + 1;
                middle_face_curr_id++;
                std::ptrdiff_t face_fifth_id = upper_face_curr_id;
                upper_face_curr_id++;
                std::ptrdiff_t face_sixth_id = upper_face_curr_id;
                faces[id] = new slitherlink_face{
                    .id = id,
                    .value = 0,
                    .no_of_edges = 6,
                    .edge_ids = {
                        edge_first_id,
                        edge_second_id,
                        edge_third_id,
                        edge_fourth_id,
                        edge_fifth_id,
                        edge_sixth_id
                    },
                    .face_ids = {
                        face_first_id,
                        face_second_id,
                        face_third_id,
                        face_fourth_id,
                        face_fifth_id,
                        face_sixth_id
                    },
                    .edge_refs = {},
                    .face_refs = {},
                };
                id++;                
            }

        }
        // second half of upper side
        for (std::ptrdiff_t i = 0; i < (layer_no - 1) / 2 - 1; ++i) {
            std::ptrdiff_t edge_first_id =  lower_edge_curr_id;
            lower_edge_curr_id++;
            std::ptrdiff_t edge_second_id = middle_edge_curr_id;
            middle_edge_curr_id++;
            std::ptrdiff_t edge_third_id =  upper_edge_curr_id;
            upper_edge_curr_id++;
            std::ptrdiff_t edge_fourth_id = lower_edge_curr_id;
            lower_edge_curr_id++;
            std::ptrdiff_t edge_fifth_id = middle_edge_curr_id;
            std::ptrdiff_t edge_sixth_id = upper_edge_curr_id;
            upper_edge_curr_id++;

            std::ptrdiff_t face_first_id =  lower_face_curr_id;
            lower_face_curr_id++;
            std::ptrdiff_t face_second_id = middle_face_curr_id - 1;
            std::ptrdiff_t face_third_id = upper_face_curr_id;
            upper_face_curr_id++;
            std::ptrdiff_t face_fourth_id = lower_face_curr_id;
            std::ptrdiff_t face_fifth_id = middle_face_curr_id + 1;
            middle_face_curr_id++;
            std::ptrdiff_t face_sixth_id = upper_face_curr_id;
            faces[id] = new slitherlink_face{
                .id = id,
                .value = 0,
                .no_of_edges = 6,
                .edge_ids = {
                    edge_first_id,
                    edge_second_id,
                    edge_third_id,
                    edge_fourth_id,
                    edge_fifth_id,
                    edge_sixth_id
                },
                .face_ids = {
                    face_first_id,
                    face_second_id,
                    face_third_id,
                    face_fourth_id,
                    face_fifth_id,
                    face_sixth_id
                },
                .edge_refs = {},
                .face_refs = {},
            };
            id++;
        }

        edge_first_id =  lower_edge_curr_id;
        lower_edge_curr_id++;
        edge_second_id = middle_edge_curr_id;
        middle_edge_curr_id++;
        edge_third_id =  upper_edge_curr_id;
        upper_edge_curr_id++;
        edge_fourth_id = lower_edge_curr_id;
        lower_edge_curr_id++;
        if (layer_no == 1 || layer_no == 2) {
            edge_fourth_id = upper_edge_curr_id;
            upper_edge_curr_id++;
        }
        edge_fifth_id = layer_no % 2 != 0 ?
                            getNoOfEdges(layer_no) :
                            middle_edge_curr_id;
        edge_sixth_id = upper_edge_curr_id;
        upper_edge_curr_id++;

        face_first_id =  lower_face_curr_id;
        lower_face_curr_id++;
        face_second_id = middle_face_curr_id - 1;
        face_third_id = upper_face_curr_id;
        upper_face_curr_id++;
        face_fourth_id = lower_face_curr_id;
        face_fifth_id = getNoOfFaces(layer_no);
        middle_face_curr_id++;
        face_sixth_id = upper_face_curr_id;
        faces[id] = new slitherlink_face{
            .id = id,
            .value = 0,
            .no_of_edges = 6,
            .edge_ids = {
                edge_first_id,
                edge_second_id,
                edge_third_id,
                edge_fourth_id,
                edge_fifth_id,
                edge_sixth_id
            },
            .face_ids = {
                face_first_id,
                face_second_id,
                face_third_id,
                face_fourth_id,
                face_fifth_id,
                face_sixth_id
            },
            .edge_refs = {},
            .face_refs = {},
        };
        id++;

    }
    *last_id = id;
}

bool Slitherlink::checkCorrectness() {
    for (std::ptrdiff_t i = 0; i < no_of_edges; i++) {
        slitherlink_vertex* vertex_0 = vertices[edges[i]->vertices[0]];
        if (vertex_0->edge_refs[0]->id != i &&
            vertex_0->edge_refs[1]->id != i) {
            return false;
        }
        slitherlink_vertex* vertex_1 = vertices[edges[i]->vertices[1]];
        if (vertex_1->edge_refs[0]->id != i &&
            vertex_1->edge_refs[1]->id != i) {
            return false;
        }
        slitherlink_face* face_0 = faces[edges[i]->face_ids[0]];
        bool result = false;
        for (std::ptrdiff_t j = 0; j < face_0->no_of_edges; ++j) {
            if (face_0->edge_ids[j] == i) {
                result = true;
                break;
            }
        }
        if (!result) {
            return false;
        }
        result = false;
        slitherlink_face* face_1 = faces[edges[i]->face_ids[1]];
        for (std::ptrdiff_t j = 0; j < face_1->no_of_edges; ++j) {
            if (face_1->edge_ids[j] == i) {
                result = true;
                break;
            }
        }
        if (!result) {
            return false;
        }
    }

    for (std::ptrdiff_t i = 0; i < no_of_faces; i++) {
        slitherlink_face* face = faces[i];
        for (std::ptrdiff_t j = 0; j < face->no_of_edges; ++j) {
            slitherlink_edge* edge = edges[face->edge_ids[j]];
            if (edge->face_ids[0] != i && edge->face_ids[1] != i) {
                return false;
            }
        }
        for (std::ptrdiff_t j = 0; j < face->no_of_edges; ++j) {
            slitherlink_face* face_1 = faces[face->face_ids[j]];
            bool result = false;
            for (std::ptrdiff_t k = 0; k < face_1->no_of_edges; ++k) {
                if (face_1->face_ids[k] == i) {
                    result = true;
                    break;
                }
            }
            if (!result) {
                return false;
            }
        }
    }

    for (std::ptrdiff_t i = 0; i < no_of_vertices; i++) {
        slitherlink_vertex* vertex = vertices[i];
        for (std::ptrdiff_t j = 0; j < vertex->no_of_edges; ++j) {
            if (vertex->edge_refs[j]->vertices[0] != i &&
                vertex->edge_refs[j]->vertices[1] != i) {
                return false;
            }
        }
    }
    return true;
}
