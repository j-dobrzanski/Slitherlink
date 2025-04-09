#include "../api/slitherlink.hpp"
#include "../api/parser.hpp"
#include "../../../utilities/api/trace_lib.hpp"
#include <fstream>
#include <cassert>
#include <bitset>

static int readDataFromFile(std::string file_name, Slitherlink* slitherlink){
    LOG("Reading data from file: ", file_name);
    std::ifstream file(file_name);
    if (!file) {
        ERROR("Can't open file of name: ", file_name);
    }

    std::string line = "";
    parserState state = PARSER_STATE_DEFAULT;
    std::size_t params_bitmap = 0;
    while(state != PARSER_STATE_FINISH && state != PARSER_STATE_ERROR){
        LOG("Current state: ", state);
        switch(state){
            case PARSER_STATE_DEFAULT:
                parserGetLine(&file, &line);
                params_bitmap = convertToBitmap(line);
                slitherlink->params_bitmap = params_bitmap;
                if((params_bitmap & SOLVER_PARAMS_REQUIRED) ^ SOLVER_PARAMS_REQUIRED){
                    ERROR("Missing parameters in file: ", file_name, " bitmap: ", params_bitmap);
                    state = PARSER_STATE_ERROR;
                }
                state = getNextState(state, params_bitmap);
                break;
            case PARSER_STATE_READ_V:
                parserGetLine(&file, &line);
                slitherlink->no_of_vertices = std::stoi(line);
                state = getNextState(state, params_bitmap);
                break;
            case PARSER_STATE_READ_E:
                parserGetLine(&file, &line);
                slitherlink->no_of_edges = std::stoi(line);
                state = getNextState(state, params_bitmap);
                break;
            case PARSER_STATE_READ_F:
                parserGetLine(&file, &line);
                slitherlink->no_of_faces = std::stoi(line);
                state = getNextState(state, params_bitmap);
                break;
            case PARSER_STATE_READ_LIST_OF_VERTICES:
                state = readVertices(&file, slitherlink->no_of_vertices, &slitherlink->vertices);
                state = getNextState(state, params_bitmap);
                break;
            case PARSER_STATE_READ_LIST_OF_EDGES:
                state = readEdges(&file, slitherlink->no_of_edges, &slitherlink->edges);
                state = getNextState(state, params_bitmap);
                break;
            case PARSER_STATE_READ_LIST_OF_FACES:
                state = readFaces(&file, slitherlink->no_of_faces, &slitherlink->faces);
                slitherlink->no_of_faces += 1;
                state = getNextState(state, params_bitmap);
                break;
            case PARSER_STATE_FINISH:
                break;
            default:
                state = PARSER_STATE_ERROR;
                break;
        }
    }
    file.close();
    if (state == PARSER_STATE_ERROR) {
        ERROR("Error while reading file: ", file_name);
        return -1;
    }
    return 0;
}

int Slitherlink::evaluateReferences(){
    LOG("Evaluating references");
    LOG("Vertex references");
    // Vertex references
    for (slitherlink_vertex* vertex : vertices) {
        LOG("Vertex ", vertex->id);
        for (std::ptrdiff_t edge_id : vertex->edge_ids) {
            LOG("Edge ", edge_id);
            if (edge_id >= no_of_edges) {
                ERROR("Vertex ", vertex->id, " references non-existing edge ", edge_id);
                return -1;
            }
            vertex->edge_refs.push_back(edges[edge_id]);
        }
    }
    LOG("Edge references");
    // Edge references
    for (slitherlink_edge* edge : edges) {
        std::ptrdiff_t vertex_id = edge->vertices[0];
        if (vertex_id >= no_of_vertices) {
            ERROR("Edge ", edge->id, " references non-existing vertex ", vertex_id);
            return -1;
        }
        edge->vertex_refs[0] = vertices[vertex_id];
        vertex_id = edge->vertices[1];
        if (vertex_id >= no_of_vertices) {
            ERROR("Edge ", edge->id, " references non-existing vertex ", vertex_id);
            return -1;
        }
        edge->vertex_refs[1] = vertices[vertex_id];

        std::ptrdiff_t face_id = edge->face_ids[0];
        if (face_id >= no_of_faces) {
            ERROR("Edge ", edge->id, " references non-existing face ", face_id);
            return -1;
        }
        else if (face_id == OUTER_FACE) {
            edge->face_refs[0] = faces[no_of_faces - 1];
        }
        else {
            edge->face_refs[0] = faces[face_id];
        }
        face_id = edge->face_ids[1];
        if (face_id >= no_of_faces) {
            ERROR("Edge ", edge->id, " references non-existing face ", face_id);
            return -1;
        }
        else if (face_id == OUTER_FACE) {
            edge->face_refs[1] = faces[no_of_faces - 1];
        }
        else {
            edge->face_refs[1] = faces[face_id];
        }
    }
    LOG("Face references");
    // Face references
    for (slitherlink_face* face : faces) {
        if (face->id == OUTER_FACE) {
            continue;
        }
        for (std::ptrdiff_t edge_id : face->edge_ids) {
            if (edge_id >= no_of_edges) {
                ERROR("Face ", face->id, " references non-existing edge ", edge_id);
                return -1;
            }
            face->edge_refs.push_back(edges[edge_id]);
            std::ptrdiff_t face_id = edges[edge_id]->face_ids[0] == face->id ?
                                        edges[edge_id]->face_ids[1] :
                                        edges[edge_id]->face_ids[0];
            face->face_ids.push_back(face_id);
            if (face_id == OUTER_FACE) {
                face->face_refs.push_back(faces[no_of_faces - 1]);
            }
            else if (face_id >= no_of_faces) {
                ERROR("Face ", face->id, " references non-existing face ", face_id);
                return -1;
            }
            else {
                face->face_refs.push_back(faces[face_id]);
            }
        }
    }
    return 0;
}

Slitherlink::Slitherlink(std::string file_name){
    assert(readDataFromFile(file_name, this) == 0);
    assert(this->evaluateReferences() == 0);
}

Slitherlink::Slitherlink(std::size_t params_bitmap,
                         std::ptrdiff_t no_of_vertices,
                         std::ptrdiff_t no_of_edges,
                         std::ptrdiff_t no_of_faces,
                         std::vector<slitherlink_vertex*> vertices,
                         std::vector<slitherlink_edge*> edges,
                         std::vector<slitherlink_face*> faces){
    this->params_bitmap = params_bitmap;
    this->no_of_vertices = no_of_vertices;
    this->no_of_edges = no_of_edges;
    this->no_of_faces = no_of_faces;
    this->vertices = vertices;
    this->edges = edges;
    this->faces = faces;
    assert(this->evaluateReferences() == 0);
}

Slitherlink::~Slitherlink(){
    for (slitherlink_vertex* vertex : vertices) {
        delete vertex;
    }
    for (slitherlink_edge* edge : edges) {
        delete edge;
    }
    for (slitherlink_face* face : faces) {
        delete face;
    }
}

void Slitherlink::printPuzzle(std::ofstream* ofstream){
    *ofstream << "# Bitmap" << std::endl;
    *ofstream << std::bitset<NO_OF_PARAMS>(params_bitmap) << std::endl;
    *ofstream << "# No of vertices: " << std::endl;
    *ofstream << no_of_vertices << std::endl;
    *ofstream << "# No of edges: " << std::endl;
    *ofstream << no_of_edges << std::endl;
    *ofstream << "# No of faces: " << std::endl;
    *ofstream << no_of_faces - 1 << std::endl;
    *ofstream << "# Vertices: " << std::endl;
    for (slitherlink_vertex* vertex : vertices) {
        *ofstream << vertex->id << " ";
        *ofstream << vertex->no_of_edges << " ";
        for (slitherlink_edge* edge : vertex->edge_refs) {
            *ofstream << edge->id << " ";
        }
        *ofstream << std::endl;
    }
    *ofstream << "# Edges: " << std::endl;
    for (slitherlink_edge* edge : edges) {
        *ofstream << edge->id << " ";
        *ofstream << edge->vertices[0] << " " << edge->vertices[1] << " ";
        *ofstream << edge->face_ids[0] << " " << edge->face_ids[1];
        *ofstream << " " << edge->solution;
        *ofstream << std::endl;
    }
    *ofstream << "# Faces: ";
    for (slitherlink_face* face : faces) {
        *ofstream << std::endl;
        if (face->id == OUTER_FACE) {
            continue;
        }
        *ofstream << face->id << " ";
        *ofstream << face->value << " ";
        *ofstream << face->no_of_edges;
        for (std::size_t edge_id : face->edge_ids) {
            *ofstream << " " << edge_id;
        }
    }
}

void Slitherlink::savePuzzle(std::string file_name){
    std::ofstream file(file_name);
    if (!file) {
        ERROR("Can't open file of name: ", file_name);
    }
    printPuzzle(&file);
    file.close();
}