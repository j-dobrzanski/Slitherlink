#include "../api/slitherlink_generic.hpp"
#include "../../../utilities/api/trace_lib.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <bitset>

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
            return PARSER_STATE_READ_LIST_OF_FACES;
        case PARSER_STATE_READ_LIST_OF_FACES:
            return (params_bitmap & TYPE_OF_GRID_PRESENT) ? PARSER_STATE_READ_GRID_TYPE : PARSER_STATE_FINISH;
        default:
            return PARSER_STATE_ERROR;
    }
    return PARSER_STATE_ERROR;
}

static inline bool isComment(const std::string& str){
    size_t i = 0;
    while (i < str.length() && std::isspace(str[i])) {
        ++i;
    }
    return i < str.length() && str[i] == '#';
}

static inline int readEdges(std::ifstream* file, std::size_t no_of_edges, std::vector<edge>* edges){
    std::size_t i = 0;
    std::string line = "";
    while(i < no_of_edges){
        std::getline(*file, line);
        if(isComment(line)){
            ERROR("comment");
            continue;
        }
        std::istringstream iss(line);
        std::string word1 = "";
        std::string word2 = "";
        if(!(iss >> word1)){
            ERROR("Can't read first vertice of edge no", i);
            return 1;
        }
        if(!(iss >> word2)){
            ERROR("Can't read second vertice of edge no", i);
            return 1;
        }
        edges->push_back(edge(std::stoi(word1), std::stoi(word2)));
        i++;
    }
    return 0;
}

static inline int readFaces(std::ifstream* file, std::size_t no_of_faces, std::vector<face>* faces){
    std::size_t i = 0;
    std::string line = "";
    while(i < no_of_faces){
        std::getline(*file, line);
        if(isComment(line)){
            continue;
        }
        std::istringstream iss(line);
        std::string word1 = "";
        std::string word2 = "";
        std::vector<std::size_t> edges;
        if(!(iss >> word1)){
            ERROR("Can't read clue of face no", i);
            return 1;
        }
        while(iss >> word2){
            edges.push_back(std::stoi(word2));
        }
        faces->push_back(face(std::stoi(word1), edges));
        i++;
    }
    return 0;
}

static inline int readCoords(std::ifstream* file, std::size_t no_of_vertices){
    std::size_t i = 0;
    std::string line = "";
    while(i < no_of_vertices){
        std::getline(*file, line);
        if(isComment(line)){
            continue;
        }
        i++;
    }
    return 0;
}

SlitherlinkGeneric::SlitherlinkGeneric(std::string file_name){
    std::ifstream file(file_name);
    if (!file) {
        ERROR("Can't open file of name: ", file_name);
    }

    std::string line = "";
    parserState state = PARSER_STATE_DEFAULT;
    std::size_t params_bitmap = 0;
    while(state != PARSER_STATE_FINISH && state != PARSER_STATE_ERROR){
        switch(state){
            case PARSER_STATE_DEFAULT:
                std::getline(file, line);
                params_bitmap = std::stoi(line);
                if((params_bitmap & SOLVER_PARAMS_REQUIRED) ^ SOLVER_PARAMS_REQUIRED){
                    state = PARSER_STATE_ERROR;
                }
                state = PARSER_STATE_READ_V;
                break;
            case PARSER_STATE_READ_V:
                std::getline(file, line);
                V = std::stoi(line);
                state = PARSER_STATE_READ_E;
                break;
            case PARSER_STATE_READ_E:
                std::getline(file, line);
                E = std::stoi(line);
                state = (params_bitmap & SOLVED_EDGE_BIT_PRESENT) ?
                    PARSER_STATE_READ_LIST_OF_EDGES_W_SOLVED : PARSER_STATE_READ_LIST_OF_EDGES;
                break;
            case PARSER_STATE_READ_LIST_OF_EDGES_W_SOLVED:
            case PARSER_STATE_READ_LIST_OF_EDGES:
                readEdges(&file, E, &edges);
                state = PARSER_STATE_READ_F;
                break;
            case PARSER_STATE_READ_F:
                std::getline(file, line);
                F = std::stoi(line);
                state = PARSER_STATE_READ_LIST_OF_FACES;
                break;
            case PARSER_STATE_READ_LIST_OF_FACES:
                readFaces(&file, F, &faces);
                state = PARSER_STATE_FINISH;
                break;
            default:
                state = PARSER_STATE_ERROR;
        }
    }
    file.close();
}

void SlitherlinkGeneric::printPuzzle(std::ofstream* ofstream){
    std::size_t params_bitmap = (V_PRESENT | E_PRESENT | LIST_OF_EDGES_PRESENT |
        F_PRESENT | LIST_OF_FACES_PRESENT); // TODO: add gridType
    *ofstream << std::bitset<16>(params_bitmap) << std::endl;
    *ofstream << V << std::endl;
    *ofstream << E << std::endl;
    for(std::size_t i = 0; i < E; i++){
        *ofstream << edges[i].first << " " << edges[i].second << std::endl;
    }
    *ofstream << F << std::endl;
    for(std::size_t i = 0; i < F; i++){
        *ofstream << faces[i].first;
        for(std::size_t j = 0; j < faces[i].second.size(); j++){
           *ofstream << " " << faces[i].second[j];
        }
        *ofstream << std::endl;
    }
}

void SlitherlinkGeneric::savePuzzle(std::string file_name){
    std::ofstream file(file_name);
    if (!file) {
        ERROR("Can't open file of name: ", file_name);
    }
    printPuzzle(&file);
    file.close();
}