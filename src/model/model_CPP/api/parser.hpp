#include "../../../utilities/api/trace_lib.hpp"
#include "../api/common.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

#ifndef PARSER_H
#define PARSER_H

#define V_PRESENT                   (0b1)
#define E_PRESENT                   (0b1 << 1)
#define F_PRESENT                   (0b1 << 2)
#define LIST_OF_VERTICES_PRESENT    (0b1 << 3)
#define LIST_OF_EDGES_PRESENT       (0b1 << 4)
#define LIST_OF_FACES_PRESENT       (0b1 << 5)
#define SOLVED_EDGE_BIT_PRESENT     (0b1 << 6)
#define LIST_OF_COORDS_PRESENT      (0b1 << 7)
#define TYPE_OF_GRID_PRESENT        (0b1 << 8)

#define NO_OF_PARAMS        (9)

#define SOLVER_PARAMS_REQUIRED  (V_PRESENT | \
    E_PRESENT | \
    F_PRESENT | \
    LIST_OF_VERTICES_PRESENT | \
    LIST_OF_EDGES_PRESENT | \
    LIST_OF_FACES_PRESENT)

typedef enum parserState {
    PARSER_STATE_DEFAULT,
    PARSER_STATE_READ_V,
    PARSER_STATE_READ_E,
    PARSER_STATE_READ_F,
    PARSER_STATE_READ_LIST_OF_VERTICES,
    PARSER_STATE_READ_LIST_OF_EDGES,
    PARSER_STATE_READ_LIST_OF_EDGES_W_SOLVED,
    PARSER_STATE_READ_LIST_OF_FACES,
    PARSER_STATE_READ_LIST_OF_COORDS,
    PARSER_STATE_READ_GRID_TYPE,
    PARSER_STATE_FINISH,
    PARSER_STATE_ERROR,
} parserState;


/**
 * Edge of puzzle represented by pair of vertices at both ends of edge.
 * Vertices should be ordered ascending.
 */
typedef std::pair<std::size_t, std::size_t> edge;

/**
 * Returns true if string starts with '#' character after spaces.
 */
inline bool isComment(const std::string& str){
    size_t i = 0;
    while (i < str.length() && std::isspace(str[i])) {
        ++i;
    }
    return i < str.length() && str[i] == '#';
}

inline std::istream& parserGetLine(std::ifstream* file, std::string* line){
    do {
        std::getline(*file, *line);
    } while (isComment(*line));
    LOG("Reading non-comment line: ", *line);
    return *file;
}

std::size_t convertToBitmap(std::string line);

parserState getNextState(parserState current_state, std::size_t params_bitmap);

parserState readVertices(std::ifstream* file, std::ptrdiff_t no_of_vertices, std::vector<slitherlink_vertex*>* vertices);

parserState readEdges(std::ifstream* file, std::ptrdiff_t no_of_edges, std::vector<slitherlink_edge*>* edges);

parserState readFaces(std::ifstream* file, std::ptrdiff_t no_of_faces, std::vector<slitherlink_face*>* faces);

parserState readCoords(std::ifstream* file, std::ptrdiff_t no_of_vertices);

#endif // PARSER_H