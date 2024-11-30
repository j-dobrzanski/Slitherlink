#include "../api/slitherlink_generic.hpp"
#include "../../../utilities/api/trace_lib.hpp"
#include <iostream>
#include <fstream>
#include <string>

#define V_PRESENT               (0b1)
#define E_PRESENT               (0b1 << 1)
#define LIST_OF_EDGES_PRESENT   (0b1 << 2)
#define F_PRESENT               (0b1 << 3)
#define LIST_OF_FACES_PRESENT   (0b1 << 4)
#define SOLVED_EDGE_BIT_PRESENT (0b1 << 5)
#define LIST_OF_COORDS_PRESENT  (0b1 << 6)
#define TYPE_OF_GRID_PRESENT    (0b1 << 7)

#define SOLVER_PARAMS_REQUIRED  (V_PRESENT | E_PRESENT | LIST_OF_EDGES_PRESENT | F_PRESENT | LIST_OF_FACES_PRESENT)

typedef enum parserState {
    PARSER_STATE_DEFAULT,
    PARSER_STATE_READ_V,
    PARSER_STATE_READ_E,
    PARSER_STATE_READ_LIST_OF_EDGES,
    PARSER_STATE_READ_LIST_OF_EDGES_W_SOLVED,
    PARSER_STATE_READ_F,
    PARSER_STATE_READ_LIST_OF_FACES,
    PARSER_STATE_READ_LIST_OF_COORDS,
    PARSER_STATE_READ_GRID_TYPE,
    PARSER_STATE_FINISH,
    PARSER_STATE_ERROR,
} parserState;

static inline parserState getNextState(parserState current_state, std::size_t params_bitmap){
    if((params_bitmap & SOLVER_PARAMS_REQUIRED) ^ SOLVER_PARAMS_REQUIRED){
        return PARSER_STATE_ERROR;
    }
    switch(current_state){
        case PARSER_STATE_DEFAULT:
            return PARSER_STATE_READ_V;
        case PARSER_STATE_READ_V:
            return PARSER_STATE_READ_E;
        case PARSER_STATE_READ_E:
            return (params_bitmap & SOLVED_EDGE_BIT_PRESENT) ?
                    PARSER_STATE_READ_LIST_OF_EDGES_W_SOLVED : PARSER_STATE_READ_LIST_OF_EDGES;
        case PARSER_STATE_READ_LIST_OF_EDGES:
        case PARSER_STATE_READ_LIST_OF_EDGES_W_SOLVED:
            return PARSER_STATE_READ_F;
        case PARSER_STATE_READ_F:
            return (params_bitmap & LIST_OF_FACES_PRESENT) ? PARSER_STATE_READ_LIST_OF_FACES :
                    ((params_bitmap & TYPE_OF_GRID_PRESENT) ? PARSER_STATE_READ_GRID_TYPE : PARSER_STATE_FINISH);
        case PARSER_STATE_READ_LIST_OF_FACES:
            return (params_bitmap & TYPE_OF_GRID_PRESENT) ? PARSER_STATE_READ_GRID_TYPE : PARSER_STATE_FINISH;
    }
    return PARSER_STATE_ERROR;
}

static inline bool isComment(const std::string& str){
    size_t i = 0;
    while (i < str.length() && std::isspace(str[i])) {
        ++i;
    }
    return i < str.length() && str[i] != '#';
}

SlitherlinkGeneric::SlitherlinkGeneric(std::string file_name){
    std::ifstream file(file_name);
    if (!file) {
        ERROR("Can't open file of name: ", file_name);
    }

    std::size_t V = 0;
    std::size_t E = 0;
    std::size_t F = 0;
    std::vector<edge> edges;

    std::string line = "";
    while(std::getline(file, line)){
        
    }

    file.close();
}