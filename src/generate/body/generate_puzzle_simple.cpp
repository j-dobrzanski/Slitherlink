#include <random>
#include <ctime>
#include <cassert>
#include <chrono>

#include "../../utilities/api/trace_lib.hpp"
#include "../api/generate_puzzle.hpp"

#define FACE_UNPROCESSED 0
#define FACE_ACCEPTED 1
#define FACE_REJECTED 2

#define EDGE_UNPROCESSED 0
#define EDGE_ACCEPTED 1
#define EDGE_NOT_ACCEPTED 2

std::ptrdiff_t getRandomFaceId(std::ptrdiff_t no_of_faces, std::mt19937 rng){

    std::uniform_int_distribution<int> uni(0, no_of_faces - 2);

    std::ptrdiff_t x = uni(rng);
    std::ptrdiff_t r = x % (no_of_faces - 1);
    while (x - r > (no_of_faces - 1)){
        x = uni(rng);
        r = x % (no_of_faces - 1);
    }

    return r;
}

std::ptrdiff_t getRandomFaceFromQueue(std::vector<std::ptrdiff_t>& face_queue, std::mt19937 rng){
    assert(face_queue.size() > 0);

    std::uniform_int_distribution<int> uni(0, face_queue.size() - 1);

    std::ptrdiff_t x = uni(rng);
    std::ptrdiff_t r = x % face_queue.size();
    while (x - r > (std::ptrdiff_t)face_queue.size()) { // realistically size of queue is never bigger then max ptrdiff_t
        x = uni(rng);
        r = x % face_queue.size();
    }
    std::ptrdiff_t face_id = face_queue[r];
    face_queue.erase(face_queue.begin() + r);

    LOG_DEBUG("Face taken from queue: ", face_id);
    return face_id;
}

void processAcceptedFace(Slitherlink* slitherlink,
                         std::ptrdiff_t face_id,
                         std::vector<std::ptrdiff_t>* face_queue,
                         std::vector<int>* face_indicators,
                         std::vector<int>* edge_indicators) {
    LOG_DEBUG("Processing accepted face: ", face_id);
    slitherlink_face* face = slitherlink->faces[face_id];
    (*face_indicators)[face_id] = FACE_ACCEPTED;
    for (std::ptrdiff_t i = 0; i < face->no_of_edges; ++i) {
        std::ptrdiff_t edge_id = face->edge_ids[i];
        if ((*edge_indicators)[edge_id] == EDGE_UNPROCESSED) {
            (*edge_indicators)[edge_id] = EDGE_ACCEPTED;
            slitherlink_edge* edge = slitherlink->edges[edge_id];
            std::ptrdiff_t new_face_id = edge->face_ids[0] == face_id ?
                                            edge->face_ids[1] :
                                            edge->face_ids[0];
            if (new_face_id == OUTER_FACE) {
                continue;
            }
            assert(new_face_id != face_id);
            LOG_DEBUG("New face id: ", new_face_id);
            assert((*face_indicators)[new_face_id] != FACE_ACCEPTED);
            face_queue->push_back(new_face_id);
            LOG_DEBUG("Face added to queue: ", new_face_id);
        }
        else {
            (*edge_indicators)[edge_id] = EDGE_NOT_ACCEPTED;
        }
    }
}

void logIndicators(std::vector<int>* indicators, std::string name){
    LOG_DEBUG(name, ":");
    for (std::size_t i = 0; i < indicators->size(); ++i) {
        if ((*indicators)[i] == FACE_ACCEPTED ||
            (*indicators)[i] == EDGE_ACCEPTED) {
            LOG_DEBUG(" ", i);
        }
    }
    (void)name;
}

void createRandomLoop(Slitherlink* slitherlink, std::vector<int>* face_indicators, std::vector<int>* edge_indicators) {
    LOG("Creating random loop");
    std::vector<std::ptrdiff_t> face_queue = std::vector<std::ptrdiff_t>();
    
    std::random_device rd;
    std::mt19937 rng(rd());

    // initialize the first face
    std::ptrdiff_t first_face_id = getRandomFaceId(slitherlink->no_of_faces - 1, rng);
    processAcceptedFace(slitherlink, first_face_id, &face_queue, face_indicators, edge_indicators);

    // find other faces
    while (face_queue.size() > 0) {
        std::ptrdiff_t face_id = getRandomFaceFromQueue(face_queue, rng);
        if ((*face_indicators)[face_id] == FACE_ACCEPTED ||
            (*face_indicators)[face_id] == FACE_REJECTED) {
            continue;
        }
        slitherlink_face* face = slitherlink->faces[face_id];
        // check if exactly one edge is already in edge_indicators
        std::ptrdiff_t edge_count = 0;
        for (std::ptrdiff_t i = 0; i < face->no_of_edges; ++i) {
            std::ptrdiff_t edge_id = face->edge_ids[i];
            if ((*edge_indicators)[edge_id] == EDGE_ACCEPTED) {
                edge_count++;
            }
        }

        LOG_DEBUG("Face ", face_id, " has ", edge_count, " edge(s) in edge_indicators");

        // if no edge is in edge_indicators, then how this face ended up here?
        assert(edge_count > 0);

        // if more than one edge found then we skip this face
        if (edge_count > 1) {
            continue;
        }

        processAcceptedFace(slitherlink, face_id, &face_queue, face_indicators, edge_indicators);
    }
    logIndicators(face_indicators, "Face indicators");
    logIndicators(edge_indicators, "Edge indicators");
}

void addFaceValues(Slitherlink* slitherlink, std::vector<int>* edge_indicators) {
    LOG("Adding face values");
    for (std::ptrdiff_t i = 0; i < slitherlink->no_of_faces - 1; ++i) {
        std::ptrdiff_t no_of_chosen_edges = 0;
        for (std::ptrdiff_t j = 0; j < slitherlink->faces[i]->no_of_edges; ++j) {
            std::ptrdiff_t edge_id = slitherlink->faces[i]->edge_ids[j];
            if ((*edge_indicators)[edge_id] == EDGE_ACCEPTED) {
                no_of_chosen_edges++;
            }
        }
        slitherlink->faces[i]->value = no_of_chosen_edges;
    }
}

void addEdgeValues(Slitherlink* slitherlink, std::vector<int>* edge_indicators) {
    LOG("Adding edge values");
    for (std::ptrdiff_t i = 0; i < slitherlink->no_of_edges; ++i) {
        if ((*edge_indicators)[i] == EDGE_ACCEPTED) {
            slitherlink->edges[i]->solution = EDGE_IN_SOLUTION;
        }
        else {
            slitherlink->edges[i]->solution = EDGE_NOT_IN_SOLUTION;
        }
    }
}

Slitherlink* generatePuzzleSimple(std::ptrdiff_t size){
    Slitherlink* slitherlink = new Slitherlink(size);
    std::vector<int> face_indicators = std::vector<int>(slitherlink->no_of_faces, FACE_UNPROCESSED);
    std::vector<int> edge_indicators = std::vector<int>(slitherlink->no_of_edges, EDGE_UNPROCESSED);
    
    // create a random loop
    createRandomLoop(slitherlink, &face_indicators, &edge_indicators);

    // add values to the faces
    addFaceValues(slitherlink, &edge_indicators);

    // add values to the edges
    addEdgeValues(slitherlink, &edge_indicators);

    return slitherlink;
}
