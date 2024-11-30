#include <cstdint>
#include <vector>
#include <string>

#ifndef SLITHERLINK_TEMPLATE_H
#define SLITHERLINK_TEMPLATE_H

#define INVALID_INDEX UINTMAX_MAX

/**
 * Template for Slitherlink puzzle representation
 *  - T_edge: type for edge
 */
template<typename T_edge, typename T_face>
class SlitherlinkTemplate {
    private:
        /* Number of vertices in puzzle */
        std::size_t V;
        /* Number of edges in puzzle */
        std::size_t E;
        /* List of edges - pairs of vertices */
        std::vector<T_edge> edges;
        /* Number of faces in puzzle */
        std::size_t F;
        /* List of faces */
        std::vector<T_face> faces;

    public:
        SlitherlinkTemplate();
        /**
         * Construct Slitherlink puzzle from a set of parameters
         */
        SlitherlinkTemplate(std::size_t V,
                    std::size_t E,
                    std::vector<T_edge> edges,
                    std::size_t F,
                    std::vector<T_face> faces);

        /* Get number of vertices in puzzle */
        std::size_t getV() const {
            return V;
        };

        /* Get number of edges in puzzle */
        std::size_t getE() const {
            return E;
        };

        /**
         * Get a copy of edges vector
         *  (May be inefficient!)
         */
        std::vector<T_edge> getEdges() const {
            return std::vector<edge>(edges);
        };

        /**
         * Get an edge by an index
         * Returns (INVALID_INDEX,INVALID_INDEX) if index out of edge list
         */
        T_edge getEdge(std::size_t index) const {
            if(index >= E){
                return edge(INVALID_INDEX,INVALID_INDEX);
            }
            return edges[index];
        }

        /* Get number of faces in puzzle */
        std::size_t getF() const {
            return F;
        };

        /**
         * Get a copy of faces vector 
         *  (May be inefficient!) 
         */
        std::vector<T_face> getFaces() const {
            return std::vector<face>(faces);
        };

        /**
         * Get a face by an index
         * Returns (INVALID_INDEX,{}) if index out of edge list
         */
        T_face getFace(std::size_t index) const {
            if(index >= F){
                return face(INVALID_INDEX, {});
            }
            return faces[index];
        }

        /**
         * Print puzzle representation to standard output stream
         */
        void print() const;

        /**
         * Save puzzle representation to file of given name
         */
        void saveToFile(std::string file_name) const;
};

#endif