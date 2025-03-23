#include <vector>

#ifndef SLITHERLINK_COMMON_H
#define SLITHERLINK_COMMON_H

#define OUTER_FACE -1

/**
 * Vertex with all adjacent edge ids.
 */
typedef struct slitherlink_vertex slitherlink_vertex;

typedef enum slitherlink_edge_type {
    EDGE_IN_SOLUTION,
    EDGE_NOT_IN_SOLUTION,
    EDGE_UNKNOWN
} slitherlink_edge_type;

/**
 * Edge with two vertices and two face ids.
 */
typedef struct slitherlink_edge slitherlink_edge;

/**
 * Face with value and list of edge ids.
 */
typedef struct slitherlink_face slitherlink_face;


typedef struct slitherlink_vertex {
    std::ptrdiff_t id;
    std::ptrdiff_t no_of_edges;
    std::vector<std::ptrdiff_t> edge_ids;
    std::vector<slitherlink_edge*> edge_refs;
} slitherlink_vertex;

typedef struct slitherlink_edge {
    std::ptrdiff_t id;
    std::ptrdiff_t vertices[2];
    std::ptrdiff_t face_ids[2];
    slitherlink_vertex* vertex_refs[2];
    slitherlink_face* face_refs[2];
    slitherlink_edge_type solution;
} slitherlink_edge;

typedef struct slitherlink_face {
    std::ptrdiff_t id;
    std::ptrdiff_t value;
    std::ptrdiff_t no_of_edges;
    std::vector<std::ptrdiff_t> edge_ids;
    std::vector<slitherlink_edge*> edge_refs;
    std::vector<std::ptrdiff_t> face_ids;
    std::vector<slitherlink_face*> face_refs;
} slitherlink_face;

#endif // SLITHERLINK_COMMON_H