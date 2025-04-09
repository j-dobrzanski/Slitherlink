#include "../api/parser.hpp"
#include <cassert>

std::size_t convertToBitmap(std::string line){
    std::size_t bitmap = 0;
    std::size_t value = std::stoi(line);
    std::size_t i = 0;
    while (value > 0){
        if (value % 10) {
            bitmap |= (0b1 << i);
        }
        value /= 10;
        i++;
    }
    return bitmap;
}

parserState getNextState(parserState current_state, std::size_t params_bitmap){
    if((params_bitmap & SOLVER_PARAMS_REQUIRED) ^ SOLVER_PARAMS_REQUIRED){
        return PARSER_STATE_ERROR;
    }
    switch(current_state){
        case PARSER_STATE_DEFAULT:
            return PARSER_STATE_READ_V;
        case PARSER_STATE_READ_V:
            return PARSER_STATE_READ_E;
        case PARSER_STATE_READ_E:
            return PARSER_STATE_READ_F;
        case PARSER_STATE_READ_F:
            return PARSER_STATE_READ_LIST_OF_VERTICES;
        case PARSER_STATE_READ_LIST_OF_VERTICES:
            return (params_bitmap & SOLVED_EDGE_BIT_PRESENT) ?
                    PARSER_STATE_READ_LIST_OF_EDGES_W_SOLVED : PARSER_STATE_READ_LIST_OF_EDGES;
        case PARSER_STATE_READ_LIST_OF_EDGES:
        case PARSER_STATE_READ_LIST_OF_EDGES_W_SOLVED:
            return PARSER_STATE_READ_LIST_OF_FACES;
        case PARSER_STATE_READ_LIST_OF_FACES:
            return (params_bitmap & TYPE_OF_GRID_PRESENT) ? PARSER_STATE_READ_GRID_TYPE : PARSER_STATE_FINISH;
        case PARSER_STATE_READ_GRID_TYPE:
            return PARSER_STATE_FINISH;
        case PARSER_STATE_FINISH:
            return PARSER_STATE_FINISH;
        default:
            return PARSER_STATE_ERROR;
    }
    return PARSER_STATE_ERROR;
}

parserState readVertices(std::ifstream* file, std::ptrdiff_t no_of_vertices, std::vector<slitherlink_vertex*>* vertices){
    std::ptrdiff_t i = 0;
    std::string line = "";
    while(i < no_of_vertices){
        parserGetLine(file, &line);
        std::istringstream iss(line);
        std::string word = "";
        if(!(iss >> word)){
            ERROR("Can't read vertice no", i);
            return PARSER_STATE_ERROR;
        }
        std::ptrdiff_t id = std::stoi(word);
        if(id != i){
            ERROR("Wrong vertice id", id, "expected", i, "in line", line);
            return PARSER_STATE_ERROR;
        }
        if(!(iss >> word)){
            ERROR("Can't read number of edges of vertice no", i);
            return PARSER_STATE_ERROR;
        }
        std::ptrdiff_t no_of_edges = std::stoi(word);
        std::ptrdiff_t edge_counter = 0;
        slitherlink_vertex* vertex = new slitherlink_vertex{
            .id = i,
            .no_of_edges = no_of_edges,
            .edge_ids = {},
            .edge_refs = {}
        };
        while(edge_counter < no_of_edges){
            if(!(iss >> word)){
                ERROR("Can't read edge no", edge_counter, "of vertice no", i);
                return PARSER_STATE_ERROR;
            }
            vertex->edge_ids.push_back(std::stoi(word));
            edge_counter++;
        }
        vertices->push_back(vertex);
        i++;
    }
    return PARSER_STATE_READ_LIST_OF_VERTICES;
}

parserState readEdges(std::ifstream* file, std::ptrdiff_t no_of_edges, std::vector<slitherlink_edge*>* edges){
    std::ptrdiff_t i = 0;
    std::string line = "";
    while(i < no_of_edges){
        parserGetLine(file, &line);
        std::istringstream iss(line);
        std::string word = "";
        if(!(iss >> word)){
            ERROR("Can't read index of edge no ", i);
            return PARSER_STATE_ERROR;
        }
        std::ptrdiff_t id = std::stoi(word);
        if(id != i){
            ERROR("Wrong face id", id, "expected", i);
            return PARSER_STATE_ERROR;
        }
        if(!(iss >> word)){
            ERROR("Can't read first vertex of edge no ", i);
            return PARSER_STATE_ERROR;
        }
        std::ptrdiff_t vertex_1_id = std::stoi(word);
        if(!(iss >> word)){
            ERROR("Can't read second vertex of edge no ", i);
            return PARSER_STATE_ERROR;
        }
        std::ptrdiff_t vertex_2_id = std::stoi(word);
        if(!(iss >> word)){
            ERROR("Can't read first face of edge no ", i);
            return PARSER_STATE_ERROR;
        }
        std::ptrdiff_t face_1_id = std::stoi(word);
        if(!(iss >> word)){
            ERROR("Can't read second face of edge no ", i);
            return PARSER_STATE_ERROR;
        }
        std::ptrdiff_t face_2_id = std::stoi(word);
        slitherlink_edge* edge = new slitherlink_edge{
            .id = i,
            .vertices = {vertex_1_id, vertex_2_id},
            .face_ids = {face_1_id, face_2_id},
            .vertex_refs = {nullptr, nullptr},
            .face_refs = {nullptr, nullptr},
            .solution = EDGE_UNKNOWN
        };
        edges->push_back(edge);
        i++;
    }
    return PARSER_STATE_READ_LIST_OF_EDGES;
}

parserState readFaces(std::ifstream* file, std::ptrdiff_t no_of_faces, std::vector<slitherlink_face*>* faces){
    std::ptrdiff_t i = 0;
    std::string line = "";
    while(i < no_of_faces){
        parserGetLine(file, &line);
        std::istringstream iss(line);
        std::string word = "";
        if(!(iss >> word)){
            ERROR("Can't read face no", i);
            return PARSER_STATE_ERROR;
        }
        std::ptrdiff_t id = std::stoi(word);
        if(id != i){
            ERROR("Wrong face id", id, "expected", i);
            return PARSER_STATE_ERROR;
        }
        if(!(iss >> word)){
            ERROR("Can't read value of face no", i);
            return PARSER_STATE_ERROR;
        }
        std::ptrdiff_t value = std::stoi(word);
        if(!(iss >> word)){
            ERROR("Can't read number of edges of face no", i);
            return PARSER_STATE_ERROR;
        }
        std::ptrdiff_t no_of_edges = std::stoi(word);
        std::ptrdiff_t edge_counter = 0;
        slitherlink_face* face = new slitherlink_face{
            .id = i,
            .value = value,
            .no_of_edges = no_of_edges,
            .edge_ids = {},
            .face_ids = {},
            .edge_refs = {},
            .face_refs = {}
        };
        while(edge_counter < no_of_edges){
            if(!(iss >> word)){
                ERROR("Can't read edge no", edge_counter, "of face no", i);
                return PARSER_STATE_ERROR;
            }
            face->edge_ids.push_back(std::stoi(word));
            edge_counter++;
        }
        faces->push_back(face);
        i++;
    }
    // Add outer face
    slitherlink_face* outer_face = new slitherlink_face{
        .id = OUTER_FACE,
        .value = 0,
        .no_of_edges = 0,
        .edge_ids = {},
        .face_ids = {},
        .edge_refs = {},
        .face_refs = {}
    };
    faces->push_back(outer_face);
    return PARSER_STATE_READ_LIST_OF_FACES;
}

parserState readCoords(std::ifstream* file, std::ptrdiff_t no_of_vertices){
    std::ptrdiff_t i = 0;
    std::string line = "";
    while(i < no_of_vertices){
        std::getline(*file, line);
        if(isComment(line)){
            continue;
        }
        i++;
    }
    return PARSER_STATE_READ_LIST_OF_COORDS;
}