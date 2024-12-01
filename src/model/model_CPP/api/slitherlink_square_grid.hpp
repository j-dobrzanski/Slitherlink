#include "slitherlink_template.hpp"
#include <array>

#ifndef SLITHERLINK_SQUARE_GRID_H
#define SLITHERLINK_SQUARE_GRID_H

#ifdef FACE_SIZE
#undef FACE_SIZE
#endif
#define FACE_SIZE 4

/**
 * Edge of puzzle represented by pair of vertices at both ends of edge.
 * Vertices should be ordered ascending.
 */
typedef std::pair<std::size_t, std::size_t> edge;

/**
 * Face of puzzle represented by pair of:
 *  - clue (number of edges in solution)
 *  - array of four edge indices around that face
 */
typedef std::pair<std::size_t, std::array<std::size_t, FACE_SIZE>> face;

class SlitherlinkSquareGrid : SlitherlinkTemplate<face> {
    
};

#endif