#include "../api/solver.hpp"
#include "../../utilities/api/trace_lib.hpp"

#include <cassert>
#include <algorithm>


void Solver::push(std::pair<queue_item_type, std::ptrdiff_t> item) {
    if (find(queue.begin(), queue.end(), item) != queue.end()) {
        queue.erase(std::remove(queue.begin(), queue.end(), item), queue.end());
    }
    queue.push_back(item);
}

void Solver::push_edge(slitherlink_edge* edge_p) {
    std::size_t queue_size = queue.size();
    if (!faces_solved[edge_p->face_ids[0]]) {
        if (!isFaceSolved(edge_p->face_refs[0])) {
            LOG("Pushing face ", edge_p->face_ids[0], " to queue from edge ", edge_p->id);
            queue.push_back(std::make_pair(QUEUE_ITEM_FACE, edge_p->face_ids[0]));
        }
    }


    if (!faces_solved[edge_p->face_ids[1]]) {
        if (!isFaceSolved(edge_p->face_refs[1])) {
            LOG("Pushing face ", edge_p->face_ids[1], " to queue from edge ", edge_p->id);
            queue.push_back(std::make_pair(QUEUE_ITEM_FACE, edge_p->face_ids[1]));
        }
    }

    if (!vertices_solved[edge_p->vertices[0]]) {
        if (!isVertexSolved(edge_p->vertex_refs[0])) {
            LOG("Pushing vertex ", edge_p->vertices[0], " to queue from edge ", edge_p->id);
            queue.push_back(std::make_pair(QUEUE_ITEM_VERTEX, edge_p->vertices[0]));
        }
    }

    if (!vertices_solved[edge_p->vertices[1]]) {
        if (!isVertexSolved(edge_p->vertex_refs[1])) {
            LOG("Pushing vertex ", edge_p->vertices[1], " to queue from edge ", edge_p->id);
            queue.push_back(std::make_pair(QUEUE_ITEM_VERTEX, edge_p->vertices[1]));
        }
    }
    if(queue.size() == queue_size) {
        ERROR("Nothing was pushed to queue");
    }
}

std::pair<queue_item_type, std::ptrdiff_t> Solver::pop() {
    assert(!queue.empty());
    auto item = queue.back();
    queue.pop_back();
    return item;
}
