#include <vector>
#include <ostream>

#ifndef SLITHERLINK_COMMON_H
#define SLITHERLINK_COMMON_H

#define OUTER_FACE -1

/**
 * Vertex with all adjacent edge ids.
 */
typedef struct slitherlink_vertex slitherlink_vertex;

/**
 * Edge type for slitherlink.
 * EDGE_IN_SOLUTION: edge is part of the solution
 * EDGE_NOT_IN_SOLUTION: edge is not part of the solution
 * EDGE_UNKNOWN: edge is not yet known
 */
typedef enum slitherlink_edge_type {
    EDGE_UNKNOWN = 0,
    EDGE_IN_SOLUTION,
    EDGE_NOT_IN_SOLUTION
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

    slitherlink_vertex() : id(-1), no_of_edges(0) {}

    slitherlink_vertex(slitherlink_vertex* other) {
        id = other->id;
        no_of_edges = other->no_of_edges;
        edge_ids = other->edge_ids;
        edge_refs = other->edge_refs;
    }

    friend std::ostream & operator<<(std::ostream& os, const slitherlink_vertex& vertex);
} slitherlink_vertex;

typedef struct slitherlink_edge {
    std::ptrdiff_t id;
    std::ptrdiff_t vertices[2];
    std::ptrdiff_t face_ids[2];
    slitherlink_vertex* vertex_refs[2];
    slitherlink_face* face_refs[2];
    slitherlink_edge_type solution;

    slitherlink_edge() : id(-1), solution(EDGE_UNKNOWN) {
        vertices[0] = -1;
        vertices[1] = -1;
        face_ids[0] = OUTER_FACE;
        face_ids[1] = OUTER_FACE;
        vertex_refs[0] = nullptr;
        vertex_refs[1] = nullptr;
        face_refs[0] = nullptr;
        face_refs[1] = nullptr;
    }

    slitherlink_edge(slitherlink_edge* other) {
        id = other->id;
        vertices[0] = other->vertices[0];
        vertices[1] = other->vertices[1];
        face_ids[0] = other->face_ids[0];
        face_ids[1] = other->face_ids[1];
        vertex_refs[0] = other->vertex_refs[0];
        vertex_refs[1] = other->vertex_refs[1];
        face_refs[0] = other->face_refs[0];
        face_refs[1] = other->face_refs[1];
        solution = other->solution;
    }

    friend std::ostream & operator<<(std::ostream& os, const slitherlink_edge& edge);
} slitherlink_edge;

typedef struct slitherlink_face {
    std::ptrdiff_t id;
    std::ptrdiff_t value;
    std::ptrdiff_t no_of_edges;
    std::vector<std::ptrdiff_t> edge_ids;
    std::vector<std::ptrdiff_t> face_ids;
    std::vector<slitherlink_edge*> edge_refs;
    std::vector<slitherlink_face*> face_refs;

    slitherlink_face() : id(-1), value(0), no_of_edges(0) {}

    slitherlink_face(slitherlink_face* other) {
        id = other->id;
        value = other->value;
        no_of_edges = other->no_of_edges;
        edge_ids = other->edge_ids;
        face_ids = other->face_ids;
        edge_refs = other->edge_refs;
        face_refs = other->face_refs;
    }

    friend std::ostream & operator<<(std::ostream& os, const slitherlink_face& face);
} slitherlink_face;

/**
 * Returns the number of vertices in all layers up to given.
 * @param layer_no The layer number.
 * @return The number of vertices in all layers up to given.
 */
std::ptrdiff_t getNoOfVertices(std::ptrdiff_t layer_no);

/**
 * Returns the number of edges in all layers up to given.
 * @param layer_no The layer number.
 * @return The number of edges in all layers up to given.
 */
std::ptrdiff_t getNoOfEdges(std::ptrdiff_t layer_no);

/**
 * Returns the number of faces in all layers up to given.
 * @param layer_no The layer number.
 * @return The number of faces in all layers up to given.
 */
std::ptrdiff_t getNoOfFaces(std::ptrdiff_t layer_no);

#endif // SLITHERLINK_COMMON_H