#include "../api/solver.hpp"
#include "../../utilities/api/trace_lib.hpp"

#include <algorithm>
#include <cassert>

void Solver::updateVertex(slitherlink_vertex* vertex) {
    std::ptrdiff_t no_of_edges_in_solution = 0;
    std::ptrdiff_t no_of_unknown_edges = 0;
    for (std::ptrdiff_t i = 0; i < vertex->no_of_edges; ++i) {
        if (vertex->edge_refs[i]->solution == EDGE_IN_SOLUTION) {
            no_of_edges_in_solution++;
        }
        else if (vertex->edge_refs[i]->solution == EDGE_UNKNOWN) {
            no_of_unknown_edges++;
        }
    }
    if ((no_of_edges_in_solution % 2 == 1)) {
        if (no_of_unknown_edges == 0) {
            is_tes_correct = false;
            return;
        }
        else if (no_of_unknown_edges == 1) {
            for (std::ptrdiff_t i = 0; i < vertex->no_of_edges; ++i) {
                if (vertex->edge_refs[i]->solution == EDGE_UNKNOWN) {
                    vertex->edge_refs[i]->solution = EDGE_IN_SOLUTION;
                    push_tes_edge(vertex->edge_refs[i]);
                    return;
                }
            }
        }
    }
    else if (no_of_unknown_edges == 1) {
        for (std::ptrdiff_t i = 0; i < vertex->no_of_edges; ++i) {
            if (vertex->edge_refs[i]->solution == EDGE_UNKNOWN) {
                vertex->edge_refs[i]->solution = EDGE_NOT_IN_SOLUTION;
                push_tes_edge(vertex->edge_refs[i]);
                return;
            }
        }
    }
}

void Solver::updateFace(slitherlink_face* face) {
    std::ptrdiff_t no_of_edges_in_solution = 0;
    std::ptrdiff_t no_of_unknown_edges = 0;
    for (std::ptrdiff_t i = 0; i < face->no_of_edges; ++i) {
        if (face->edge_refs[i]->solution == EDGE_IN_SOLUTION) {
            no_of_edges_in_solution++;
        }
        else if (face->edge_refs[i]->solution == EDGE_UNKNOWN) {
            no_of_unknown_edges++;
        }
    }

    if (no_of_edges_in_solution % 2 == 1 &&
        no_of_unknown_edges == 1) {
        for (std::ptrdiff_t i = 0; i < face->no_of_edges; ++i) {
            if (face->edge_refs[i]->solution == EDGE_UNKNOWN) {
                face->edge_refs[i]->solution = EDGE_IN_SOLUTION;
                push_tes_edge(face->edge_refs[i]);
                return;
            }
        }
    }
    else if (no_of_edges_in_solution % 2 == 0 &&
             no_of_unknown_edges == 1) {
        for (std::ptrdiff_t i = 0; i < face->no_of_edges; ++i) {
            if (face->edge_refs[i]->solution == EDGE_UNKNOWN) {
                face->edge_refs[i]->solution = EDGE_NOT_IN_SOLUTION;
                push_tes_edge(face->edge_refs[i]);
                return;
            }
        }
    }
    else if (no_of_edges_in_solution % 2 == 1 &&
             no_of_unknown_edges == 0) {
        is_tes_correct = false;
    }
}

bool Solver::isVertexSolvedInTes(slitherlink_vertex* vertex_p) {
    if (vertex_p->no_of_edges == 0) {
        return true;
    }
    std::ptrdiff_t no_of_edges_in_solution = 0;
    for (std::ptrdiff_t i = 0; i < vertex_p->no_of_edges; ++i) {
        if (vertex_p->edge_refs[i]->solution == EDGE_IN_SOLUTION) {
            no_of_edges_in_solution++;
        }
    }
    tes_vertices_solved[vertex_p->id] = (no_of_edges_in_solution % 2 == 0);
    return (no_of_edges_in_solution % 2 == 0);
}

bool Solver::isFaceSolvedInTes(slitherlink_face* face_p) {
    if (face_p->id == OUTER_FACE) {
        return true;
    }
    std::ptrdiff_t no_of_edges_in_solution = 0;
    for (std::ptrdiff_t i = 0; i < face_p->no_of_edges; ++i) {
        if (face_p->edge_refs[i]->solution == EDGE_IN_SOLUTION) {
            no_of_edges_in_solution++;
        }
    }
    tes_faces_solved[face_p->id] = (no_of_edges_in_solution % 2 == 0);
    return (no_of_edges_in_solution % 2 == 0);
}

bool Solver::isSolvedInTes() {
    for (std::ptrdiff_t i = 0; i < slitherlink->no_of_faces; i++) {
        if (!isFaceSolvedInTes(slitherlink->faces[i])) {
            return false;
        }
    }
    for (std::ptrdiff_t i = 0; i < slitherlink->no_of_vertices; i++) {
        if (!isVertexSolvedInTes(slitherlink->vertices[i])) {
            return false;
        }
    }
    return true;
}

void Solver::push_tes(std::pair<queue_item_type, std::ptrdiff_t> item) {
    if (std::find(tes_queue.begin(), tes_queue.end(), item) == tes_queue.end()) {
        tes_queue.erase(std::remove(tes_queue.begin(), tes_queue.end(), item), tes_queue.end());
    }
    tes_queue.push_back(item);
}

void Solver::push_tes_edge(slitherlink_edge* edge_p) {
    std::size_t queue_size = tes_queue.size();
    if ((edge_p->face_ids[0] != -1) &&
        (!tes_faces_solved[edge_p->face_ids[0]])) {
        if (!isFaceSolvedInTes(edge_p->face_refs[0])) {
            LOG_DEBUG("Pushing face ", edge_p->face_ids[0], " to queue from edge ", edge_p->id);
            tes_queue.push_back(std::make_pair(QUEUE_ITEM_FACE, edge_p->face_ids[0]));
        }
    }

    if ((edge_p->face_ids[1] != -1) &&
        (!tes_faces_solved[edge_p->face_ids[1]])) {
        if (!isFaceSolvedInTes(edge_p->face_refs[1])) {
            LOG_DEBUG("Pushing face ", edge_p->face_ids[1], " to queue from edge ", edge_p->id);
            tes_queue.push_back(std::make_pair(QUEUE_ITEM_FACE, edge_p->face_ids[1]));
        }
    }

    if (!tes_vertices_solved[edge_p->vertices[0]]) {
        if (!isVertexSolvedInTes(edge_p->vertex_refs[0])) {
            LOG_DEBUG("Pushing vertex ", edge_p->vertices[0], " to queue from edge ", edge_p->id);
            tes_queue.push_back(std::make_pair(QUEUE_ITEM_VERTEX, edge_p->vertices[0]));
        }
    }

    if (!tes_vertices_solved[edge_p->vertices[1]]) {
        if (!isVertexSolvedInTes(edge_p->vertex_refs[1])) {
            LOG_DEBUG("Pushing vertex ", edge_p->vertices[1], " to queue from edge ", edge_p->id);
            tes_queue.push_back(std::make_pair(QUEUE_ITEM_VERTEX, edge_p->vertices[1]));
        }
    }
    if(tes_queue.size() == queue_size) {
        LOG_DEBUG("Nothing was pushed to queue");
    }
}

std::pair<queue_item_type, std::ptrdiff_t> Solver::pop_tes() {
    assert(!tes_queue.empty());
    auto item = tes_queue.back();
    tes_queue.pop_back();
    return item;
}

void Solver::rotateOneTesEdge(std::ptrdiff_t edge_id) {
    std::ptrdiff_t layer_no = 0;
    while (edge_id >= 9 * layer_no * layer_no - 3 * layer_no) {
        layer_no++;
    }

    std::ptrdiff_t id_in_layer = edge_id - (9 * (layer_no - 1) * (layer_no - 1) - 3 * (layer_no - 1));

    if (id_in_layer < 6 * (layer_no - 1)) {
        std::ptrdiff_t base_id = 9 * (layer_no - 1) * (layer_no - 1) - 3 * (layer_no - 1) + id_in_layer % (layer_no - 1);
        slitherlink_edge_type edge_type = totaly_even_set[base_id];
        for (std::ptrdiff_t j = 1; j < 6; j++) {
            std::ptrdiff_t rotated_id = edge_id + j * (layer_no - 1);
            if (totaly_even_set[rotated_id] == EDGE_UNKNOWN) {
                totaly_even_set[rotated_id] = edge_type;
                push_tes_edge(slitherlink->edges[rotated_id]);
            }
            else if (totaly_even_set[rotated_id] != edge_type) {
                is_tes_correct = false;
                return;
            }
        }
    }
    else {
        id_in_layer -= 6 * (layer_no - 1);
        std::ptrdiff_t base_id = 9 * (layer_no - 1) * (layer_no - 1) + 3 * (layer_no - 1) + id_in_layer % (2 * layer_no - 1);
        slitherlink_edge_type edge_type = totaly_even_set[base_id];
        for (std::ptrdiff_t j = 1; j < 6; j++) {
            std::ptrdiff_t rotated_id = base_id + j * (2 * layer_no - 1);
            if (totaly_even_set[rotated_id] == EDGE_UNKNOWN) {
                totaly_even_set[rotated_id] = edge_type;
                push_tes_edge(slitherlink->edges[rotated_id]);
            }
            else if (totaly_even_set[rotated_id] != edge_type) {
                is_tes_correct = false;
                return;
            }
        }
    }
}

void Solver::rotateAllTesEdges() {
    std::ptrdiff_t max_layers = slitherlink->getNoOfLayers();
    for (std::ptrdiff_t layer_no = 0; layer_no < max_layers; layer_no++) {
        std::ptrdiff_t base_id = 9 * layer_no * layer_no - 3 * layer_no; 
        for (std::ptrdiff_t i = 0; i < layer_no - 1; i++) {
            std::ptrdiff_t edge_id = base_id + i;
            slitherlink_edge_type edge_type = totaly_even_set[edge_id];
            for (std::ptrdiff_t j = 1; j < 6; j++) {
                std::ptrdiff_t rotated_id = edge_id + j * (layer_no - 1);
                if (totaly_even_set[rotated_id] == EDGE_UNKNOWN) {
                    totaly_even_set[rotated_id] = edge_type;
                    push_tes_edge(slitherlink->edges[rotated_id]);
                }
                else if (totaly_even_set[rotated_id] != edge_type) {
                    is_tes_correct = false;
                    return;
                }
            }
        }

        base_id += 6 * layer_no;

        for (std::ptrdiff_t i = 0; i < 2 * layer_no - 1; i++) {
            std::ptrdiff_t edge_id = base_id + i;
            slitherlink_edge_type edge_type = totaly_even_set[edge_id];
            for (std::ptrdiff_t j = 1; j < 6; j++) {
                std::ptrdiff_t rotated_id = edge_id + j * (2 * layer_no - 1);
                if (totaly_even_set[rotated_id] == EDGE_UNKNOWN) {
                    totaly_even_set[rotated_id] = edge_type;
                    push_tes_edge(slitherlink->edges[rotated_id]);
                }
                else if (totaly_even_set[rotated_id] != edge_type) {
                    is_tes_correct = false;
                    return;
                }
            }
        }
    }
}

void Solver::initializeTes() {
    totaly_even_set.clear();
    totaly_even_set.resize(slitherlink->no_of_edges, EDGE_UNKNOWN);
    is_tes_correct = true;

    tes_faces_solved.clear();
    tes_faces_solved.resize(slitherlink->no_of_faces, false);
    tes_vertices_solved.clear();
    tes_vertices_solved.resize(slitherlink->no_of_vertices, false);
    if (slitherlink_solution != nullptr && slitherlink_solution->size() == 0) {
        is_tes_correct = true;
        return;
    }

    for (std::ptrdiff_t i = 0; i < slitherlink->no_of_edges; ++i) {
        if (slitherlink->edges[i]->solution == slitherlink_solution->back()->edges[i]->solution) {
            totaly_even_set[i] = slitherlink->edges[i]->solution;
            push_tes_edge(slitherlink->edges[i]);
        }
        else {
            totaly_even_set[i] = EDGE_UNKNOWN;
        }
    }

}

void Solver::updateTes() {
    while(!tes_queue.empty()) {
        std::pair<queue_item_type, std::ptrdiff_t> item = pop_tes();
        if (item.first == QUEUE_ITEM_FACE) {
            slitherlink_face* face_p = slitherlink->faces[item.second];
            updateFace(face_p);
        }
        else if (item.first == QUEUE_ITEM_VERTEX) {
            slitherlink_vertex* vertex_p = slitherlink->vertices[item.second];
            updateVertex(vertex_p);
        }
        if (!is_tes_correct) {
            return;
        }
    }
}

bool Solver::applyTes() {
    if (!is_tes_correct) {
        ERROR("TES is not correct, cannot apply TES");
        return false;
    }

    if (slitherlink_solution == nullptr || slitherlink_solution->size() == 0) {
        ERROR("Slitherlink solution is not set, cannot apply TES");
        return false;
    }

    Slitherlink* last_solution = slitherlink_solution->back();

    for (std::ptrdiff_t i = 0; i < slitherlink->no_of_edges; ++i) {
        if (totaly_even_set[i] == EDGE_NOT_IN_SOLUTION) {
            if (slitherlink->edges[i]->solution != EDGE_UNKNOWN &&
                slitherlink->edges[i]->solution != last_solution->edges[i]->solution) {
                return false;
            }
            else if (slitherlink->edges[i]->solution == EDGE_UNKNOWN) {
                slitherlink->edges[i]->solution = last_solution->edges[i]->solution;
                push_edge(slitherlink->edges[i]);
            }
        }
        else if (totaly_even_set[i] == EDGE_IN_SOLUTION) {
            if (slitherlink->edges[i]->solution != EDGE_UNKNOWN &&
                slitherlink->edges[i]->solution == last_solution->edges[i]->solution) {
                return false;
            }
            else if (slitherlink->edges[i]->solution == EDGE_UNKNOWN) {
                slitherlink->edges[i]->solution = (last_solution->edges[i]->solution == EDGE_IN_SOLUTION) ?
                    EDGE_IN_SOLUTION : EDGE_NOT_IN_SOLUTION;
                push_edge(slitherlink->edges[i]);
            }
        }
    }
    return true;
}
