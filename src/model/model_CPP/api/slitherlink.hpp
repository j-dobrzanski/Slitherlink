#include "common.hpp"

#include <vector>
#include <string>

#ifndef SLITHERLINK_H
#define SLITHERLINK_H

class Slitherlink {
    public:
        std::size_t params_bitmap;
        std::ptrdiff_t no_of_vertices;
        std::ptrdiff_t no_of_edges;
        std::ptrdiff_t no_of_faces; 
        std::vector<slitherlink_vertex*> vertices;
        std::vector<slitherlink_edge*> edges;
        std::vector<slitherlink_face*> faces;

        /**
         * Construct Slitherlink puzzle by reading values from a file of given name
         */
        Slitherlink(std::string file_name);

        /**
         * Construct Slitherlink puzzle by passing parameters
         */
        Slitherlink(std::size_t params_bitmap,
                    std::ptrdiff_t no_of_vertices,
                    std::ptrdiff_t no_of_edges,
                    std::ptrdiff_t no_of_faces,
                    std::vector<slitherlink_vertex*> vertices,
                    std::vector<slitherlink_edge*> edges,
                    std::vector<slitherlink_face*> faces);

        ~Slitherlink();

        void printPuzzle(std::ofstream* ofstream);

        void savePuzzle(std::string file_name);
};

#endif // SLITHERLINK_H