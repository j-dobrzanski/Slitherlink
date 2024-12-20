#include "slitherlink_template.hpp"
#include <string>


#ifndef SLITHERLINK_GENERIC_H
#define SLITHERLINK_GENERIC_H

/**
 * Face of puzzle represented by pair of:
 *  - clue (number of edges in solution)
 *  - vector of edge indices around that face
 */
typedef std::pair<std::size_t, std::vector<std::size_t>> face;

class SlitherlinkGeneric : SlitherlinkTemplate<face> {
    public:
        /**
         * Construct Slitherlink puzzle by reading values from a file of given name
         */
        SlitherlinkGeneric(std::string file_name);

        void printPuzzle(std::ofstream* ofstream);

        void savePuzzle(std::string file_name);

};

#endif