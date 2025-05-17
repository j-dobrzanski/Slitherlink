#include "../../model/model_CPP/api/slitherlink.hpp"


#ifndef SOLVER_HPP
#define SOLVER_HPP

typedef enum euque_item_type {
    QUEUE_ITEM_FACE,
    QUEUE_ITEM_VERTEX
} queue_item_type;

typedef struct solver_state {
    Slitherlink* slitherlink;
    std::ptrdiff_t edge_id;
    slitherlink_edge_type edge_solution;
    std::vector<bool> faces_solved;
    std::vector<bool> vertices_solved;
    std::vector<std::ptrdiff_t> edge_to_loop_part;
    std::ptrdiff_t max_loop_part_id = -1;
    std::ptrdiff_t no_of_loop_parts = 0;
} solver_state;

typedef struct rule_state {
    std::ptrdiff_t no_of_vertices;
    std::ptrdiff_t no_of_edges;
    std::ptrdiff_t no_of_faces;
    std::vector<slitherlink_vertex*> vertices;
    std::vector<slitherlink_edge*> edges;
    std::vector<slitherlink_face*> faces;
} rule_state;

class Solver {
    public:
        Solver();
        ~Solver();

        void solvePuzzle(Slitherlink* slitherlink,
                         std::vector<Slitherlink*>* slitherlink_solution);
    private:

        Slitherlink* original_slitherlink;
        Slitherlink* slitherlink;

        bool updateFaceEdges(std::ptrdiff_t face_id);
        
        bool updateVertexEdges(std::ptrdiff_t vertex_id);


        /**
         * Check what parts of puzzle are solved
         */

        std::vector<bool> faces_solved;
        std::vector<bool> vertices_solved;

        bool isVertexSolved(slitherlink_vertex* vertex_p);
        bool isFaceSolved(slitherlink_face* face_p);
        bool isSolved();

        /**
         * Assure that only one loop is made
         */
        std::vector<std::ptrdiff_t> edge_to_loop_part;
        std::ptrdiff_t max_loop_part_id = -1;
        std::ptrdiff_t no_of_loop_parts = 0;


        bool addToLoops(std::ptrdiff_t edge_id);

        /**
         * Queue for BFS solution finding
         * Each item is a pointer to a slitherlink object 
         * and a guess that was made
         */

        std::vector<solver_state*> slitherlink_queue;

        std::ptrdiff_t makeGuess();

        bool restoreGuess();

        /**
         * Queue to store the items to be processed.
         * The queue is a vector of pairs, where each pair consists of
         * a queue_item_type and a std::ptrdiff_t.
         * The queue_item_type indicates whether the item is a face or a vertex,
         * and the std::ptrdiff_t is the id of the face or vertex.
         */
        std::vector<std::pair<queue_item_type, std::ptrdiff_t>> queue;

        void push(std::pair<queue_item_type,
                  std::ptrdiff_t> item);

        void push_edge(slitherlink_edge* edge_p);

        std::pair<queue_item_type, std::ptrdiff_t> pop();


        /** 
         * Generate, save, load and check all possible rules
         * for the puzzle
         */

        void generateAndSaveRules(std::string file_name,
                                  std::ptrdiff_t base_size);
        
        void generateBase(std::ptrdiff_t base_size);

};



#endif // SOLVER_HPP