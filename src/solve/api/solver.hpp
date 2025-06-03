#include "../../model/model_CPP/api/slitherlink.hpp"

#include <iostream>

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
    std::vector<slitherlink_edge_type> totaly_even_set;
    std::ptrdiff_t max_loop_part_id = -1;
    std::ptrdiff_t no_of_loop_parts = 0;
} solver_state;

typedef struct rule_state {
    bool is_copied = false;
    std::ptrdiff_t base_size;
    std::ptrdiff_t no_of_vertices;
    std::ptrdiff_t no_of_edges;
    std::ptrdiff_t no_of_faces;
    std::vector<slitherlink_vertex*> vertices;
    std::vector<slitherlink_edge*> edges;
    std::vector<slitherlink_face*> faces;

    rule_state* copy();

    ~rule_state();

} rule_state;

typedef struct statistics {
    std::ptrdiff_t no_of_solutions;
    std::ptrdiff_t no_of_guesses;
    double total_time;
    std::ptrdiff_t guesses_for_first_solution;
    double time_for_first_solution;

    statistics() : no_of_solutions(0),
                   no_of_guesses(0),
                   total_time(0.0),
                   guesses_for_first_solution(0),
                   time_for_first_solution(0.0) {}
    
    void reset() {
        no_of_solutions = 0;
        no_of_guesses = 0;
        total_time = 0.0;
        guesses_for_first_solution = 0;
        time_for_first_solution = 0.0;
    }
    void print() {
        std::cout << no_of_solutions << ", "
                  << no_of_guesses << ", "
                  << total_time << ", "
                  << guesses_for_first_solution << ", "
                  << time_for_first_solution << std::endl;
    }
} statistics;

typedef struct rule {
    std::vector<std::ptrdiff_t> face_values;
    std::vector<slitherlink_edge_type> base_edge_states;
    bool is_positive;
    std::vector<slitherlink_edge_type> derived_edge_states;
} rule;

class Solver {
    public:
        Solver();
        ~Solver();

        void solvePuzzle(Slitherlink* new_slitherlink,
                         bool use_rules,
                         bool use_tes,
                         std::vector<Slitherlink*>* slitherlink_solution,
                         statistics* stats);

        void generateAndSaveRules(std::string file_name,
                                  std::ptrdiff_t base_size);

        void refineAndSaveRules(std::string raw_rules_file_name,
                                std::string refined_rules_file_name,
                                std::ptrdiff_t base_size);

        void loadRules(std::string file_name,
                       std::ptrdiff_t base_size);

        void test();
    private:

        Slitherlink* original_slitherlink;
        Slitherlink* slitherlink;

        std::vector<Slitherlink*>* slitherlink_solution;

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
        std::vector<rule*> ready_rules;

        rule_state* generateBase(std::ptrdiff_t base_size);

        std::vector<rule*> generateRules(rule_state* base);

        void saveRule(std::ofstream* file,
                      rule* rule_p);

        rule* readRule(std::ifstream* file);

        // apply rules to the face of slitherlink puzzle
        void applyRules(std::vector<rule*> rules,
                        slitherlink_face* face_p);

        /**
         * Totaly Even Sets
         */
        
        // try out multiple solutions by using totaly even sets
        std::vector<slitherlink_edge_type> totaly_even_set;
        bool is_tes_correct;
        
        std::vector<bool> tes_faces_solved;
        std::vector<bool> tes_vertices_solved;

        // Used to get the baseline for tes
        void initializeTes();

        void updateTes();

        void rotateOneTesEdge(std::ptrdiff_t edge_id);

        void rotateAllTesEdges();

        std::vector<std::pair<queue_item_type, std::ptrdiff_t>> tes_queue;

        void push_tes(std::pair<queue_item_type, std::ptrdiff_t> item);

        void push_tes_edge(slitherlink_edge* edge_p);

        std::pair<queue_item_type, std::ptrdiff_t> pop_tes();

        void updateVertex(slitherlink_vertex* vertex);

        void updateFace(slitherlink_face* face_p);

        bool isVertexSolvedInTes(slitherlink_vertex* vertex_p);
        
        bool isFaceSolvedInTes(slitherlink_face* face_p);
        
        bool isSolvedInTes();

        bool applyTes();

    };



#endif // SOLVER_HPP