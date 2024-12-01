#include <cstdint>
#include <vector>
#include <string>

#ifndef SLITHERLINK_TEMPLATE_H
#define SLITHERLINK_TEMPLATE_H

#define INVALID_INDEX UINTMAX_MAX

/**
 * Edge of puzzle represented by pair of vertices at both ends of edge.
 * Vertices should be ordered ascending.
 */
typedef std::pair<std::size_t, std::size_t> edge;

/**
 * Template for Slitherlink puzzle representation
 *  - T_face: type for face
 */
template<typename T_face>
class SlitherlinkTemplate {
    protected:
        /* Number of vertices in puzzle */
        std::size_t V;
        /* Number of edges in puzzle */
        std::size_t E;
        /* List of edges - pairs of vertices */
        std::vector<edge> edges;
        /* Number of faces in puzzle */
        std::size_t F;
        /* List of faces */
        std::vector<T_face> faces;

    public:
        SlitherlinkTemplate(){
            V = 0;
            E = 0;
            F = 0;
            edges = {edge(INVALID_INDEX,INVALID_INDEX)};
            faces = {T_face()};
        }
        /**
         * Construct Slitherlink puzzle from a set of parameters
         */
        SlitherlinkTemplate(std::size_t V,
                            std::size_t E,
                            std::vector<edge> edges,
                            std::size_t F,
                            std::vector<T_face> faces)
            : V{ V },
            E{ E },
            edges{ edges },
            F{ F },
            faces{ faces }{}

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
        std::vector<edge> getEdges() const {
            return std::vector<edge>(edges);
        };

        /**
         * Get an edge by an index
         * Returns edge(INVALID_INDEX,INVALID_INDEX) if index out of edge list
         */
        edge getEdge(std::size_t index) const {
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
            return std::vector<T_face>(faces);
        };

        /**
         * Get a face by an index
         * Returns T_face() if index out of edge list
         */
        T_face getFace(std::size_t index) const {
            if(index >= F){
                return T_face();
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