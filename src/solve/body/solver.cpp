#include "../api/solver.hpp"
#include "../../utilities/api/trace_lib.hpp"

#include <cassert>
#include <random>
#include <algorithm>
#include <chrono>

bool compareSolutions(Slitherlink* solution1, Slitherlink* solution2) {
    if (solution1->no_of_edges != solution2->no_of_edges ||
        solution1->no_of_faces != solution2->no_of_faces ||
        solution1->no_of_vertices != solution2->no_of_vertices) {
        return false;
    }
    for (std::ptrdiff_t i = 0; i < solution1->no_of_edges; ++i) {
        if (solution1->edges[i]->solution != solution2->edges[i]->solution) {
            return false;
        }
    }
    return true;
}

Solver::Solver() {
    slitherlink = nullptr;
    original_slitherlink = nullptr;
    slitherlink_solution = nullptr;

    is_tes_correct = true;
}

Solver::~Solver() {
    if(slitherlink != nullptr) {
        delete slitherlink;
    }
    for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)slitherlink_queue.size(); ++i) {
        delete slitherlink_queue[i]->slitherlink;
        delete slitherlink_queue[i];
    }
    for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)ready_rules.size(); ++i) {
        delete ready_rules[i];
    }
}

void Solver::solvePuzzle(Slitherlink* new_slitherlink,
                         bool use_rules,
                         bool use_tes,
                         std::vector<Slitherlink*>* slitherlink_solution,
                         statistics* stats) {
    stats->reset();
    auto start_time = std::chrono::high_resolution_clock::now();
    auto first_solution_time = std::chrono::high_resolution_clock::now();
    auto end_time = std::chrono::high_resolution_clock::now();
    std::ptrdiff_t no_of_guesses = 0;
    
    (void) use_tes; // use_tes is not used in this implementation, but can be used in the future
    this->slitherlink_solution = slitherlink_solution;

    this->original_slitherlink = new_slitherlink;
    this->slitherlink = new_slitherlink->copy();

    this->faces_solved.resize(slitherlink->no_of_faces, false);
    this->vertices_solved.resize(slitherlink->no_of_vertices, false);
    this->slitherlink_queue.clear();
    this->queue.clear();
    this->queue.reserve(slitherlink->no_of_faces + slitherlink->no_of_vertices);
    this->edge_to_loop_part.clear();
    this->edge_to_loop_part.resize(slitherlink->no_of_edges, -1);


    for (std::ptrdiff_t i = 0; i < slitherlink->no_of_edges; ++i) {
        slitherlink->edges[i]->solution = EDGE_UNKNOWN;
    }

    std::ptrdiff_t step_count = 0;

    bool can_continue = true;

    while ((!isSolved() || !slitherlink_queue.empty()) && can_continue) {
        LOG_DEBUG("Solving puzzle");
        bool is_correct = true;
        if (isSolved()) {
            bool new_solution = true;
            for (Slitherlink* solution : *slitherlink_solution) {
                if (compareSolutions(slitherlink, solution)) {
                    new_solution = false;
                    break;
                }
            }
            if (new_solution) {
                LOG_DEBUG("Solution found");
                slitherlink_solution->push_back(slitherlink->copy());
                if(slitherlink_solution->size() == 1) {
                    first_solution_time = std::chrono::high_resolution_clock::now();
                    stats->guesses_for_first_solution = no_of_guesses;
                }
                // slitherlink->savePuzzle("solver_solution" + std::to_string(slitherlink_solution->size()) + ".txt");
                restoreGuess();
            }
        }
        if (use_tes && slitherlink_solution->size() > 0) {
            initializeTes();
            updateTes();
            is_correct = is_tes_correct;
        }
        if (is_correct && queue.empty()) {
            // slitherlink->savePuzzle("solver_step_count" + std::to_string(step_count) + ".txt");
            no_of_guesses++;
            is_correct = addToLoops(makeGuess());
        }
        LOG_DEBUG("Queue size: ", queue.size());
        while(is_correct && !queue.empty()) {
            std::ptrdiff_t edges_in_solution = 0;
            std::ptrdiff_t edges_not_in_solution = 0;
            for (std::ptrdiff_t i = 0; i < slitherlink->no_of_edges; ++i) {
                if (slitherlink->edges[i]->solution == EDGE_IN_SOLUTION) {
                    edges_in_solution++;
                }
                else if (slitherlink->edges[i]->solution == EDGE_NOT_IN_SOLUTION) {
                    edges_not_in_solution++;
                }
            }

            auto item = pop();
            if (item.first == QUEUE_ITEM_FACE) {
                is_correct = updateFaceEdges(item.second);
                LOG_DEBUG("Face ", item.second, " updated");
                if (use_rules) {
                    applyRules(ready_rules, slitherlink->faces[item.second]);
                }
            }
            else if (item.first == QUEUE_ITEM_VERTEX) {
                is_correct = updateVertexEdges(item.second);
                LOG_DEBUG("Vertex ", item.second, " updated");
            }
            else {
                ERROR("Unknown item type in queue");
                return;
            }
            // slitherlink->savePuzzle("solver_step_count" + std::to_string(step_count) + ".txt");
            if (!is_correct) {
                LOG_DEBUG("Guess was incorrect - dropping queue");
                break;
            }

        }
        step_count++;

        if (!is_correct) {
            // slitherlink->savePuzzle("solver_step_count" + std::to_string(step_count) + ".txt");
            // LOG_DEBUG("Guess was incorrect - restoring state before last guess after step: ", step_count);
            can_continue = restoreGuess();
            // slitherlink->savePuzzle("solver_step_count" + std::to_string(step_count) + "_restored.txt");
            // break;
        }
    }

    if (isSolved()) {
        bool new_solution = true;
        for (Slitherlink* solution : *slitherlink_solution) {
            if (compareSolutions(slitherlink, solution)) {
                new_solution = false;
                break;
            }
        }
        if (new_solution) {
            LOG_DEBUG("Solution found");
            slitherlink_solution->push_back(slitherlink->copy());
            if(slitherlink_solution->size() == 1) {
                first_solution_time = std::chrono::high_resolution_clock::now();
                stats->guesses_for_first_solution = no_of_guesses;
            }
            // slitherlink->savePuzzle("solver_solution" + std::to_string(slitherlink_solution->size()) + ".txt");
        }
    }
    end_time = std::chrono::high_resolution_clock::now();
    stats->no_of_solutions = slitherlink_solution->size();
    stats->no_of_guesses = no_of_guesses;
    stats->total_time = std::chrono::duration<double>(end_time - start_time).count();
    if (slitherlink_solution->size() > 0) {
        stats->time_for_first_solution = std::chrono::duration<double>(first_solution_time - start_time).count();
    }
}

bool Solver::isVertexSolved(slitherlink_vertex* edge_p) {
    for (std::ptrdiff_t i = 0; i < edge_p->no_of_edges; ++i) {
        if (edge_p->edge_refs[i]->solution == EDGE_UNKNOWN) {
            return false;
        }
    }
    vertices_solved[edge_p->id] = true;
    return true;
}

bool Solver::isFaceSolved(slitherlink_face* face_p) {
    if (face_p->id == OUTER_FACE) {
        return true;
    }
    for (std::ptrdiff_t i = 0; i < face_p->no_of_edges; ++i) {
        if ((face_p->edge_refs[i] != nullptr) &&
            (face_p->edge_refs[i]->solution == EDGE_UNKNOWN)) {
            return false;
        }
    }
    faces_solved[face_p->id] = true;
    return true;
}

bool Solver::isSolved() {
    for (std::ptrdiff_t i = 0; i < slitherlink->no_of_faces; i++) {
        std::ptrdiff_t no_of_faces_in_solution = 0;
        for (std::ptrdiff_t j = 0; j < slitherlink->faces[i]->no_of_edges; ++j) {
            if (slitherlink->faces[i]->edge_refs[j]->solution == EDGE_IN_SOLUTION) {
                no_of_faces_in_solution++;
            }
        }
        if (no_of_faces_in_solution != slitherlink->faces[i]->value) {
            return false;
        }
    }

    // std::vector<std::ptrdiff_t> edge_to_set_mapping(slitherlink->no_of_edges, -1);
    // std::ptrdiff_t set_id = 0;
    // for (std::ptrdiff_t i = 0; i < slitherlink->no_of_edges; ++i) {
    //     slitherlink_edge* edge = slitherlink->edges[i];
    //     if (edge->solution == EDGE_IN_SOLUTION) {
    //         slitherlink_edge* first_edge = nullptr;
    //         slitherlink_edge* second_edge = nullptr;
    //         for (std::ptrdiff_t j = 0; j < (std::ptrdiff_t)edge->vertex_refs[0]->no_of_edges; ++j) {
    //             if (edge->vertex_refs[0]->edge_refs[j]->solution == EDGE_IN_SOLUTION &&
    //                 edge->vertex_refs[0]->edge_refs[j] != edge) {
    //                 first_edge = edge->vertex_refs[0]->edge_refs[j];
    //             }
    //         }
    //         for (std::ptrdiff_t j = 0; j < (std::ptrdiff_t)edge->vertex_refs[1]->no_of_edges; ++j) {
    //             if (edge->vertex_refs[1]->edge_refs[j]->solution == EDGE_IN_SOLUTION &&
    //                 edge->vertex_refs[1]->edge_refs[j] != edge) {
    //                 second_edge = edge->vertex_refs[1]->edge_refs[j];
    //             }
    //         }
    //         if (first_edge == nullptr || second_edge == nullptr) {
    //             std::cout << "Edge: " << edge->id << " is not connected to other edges" << std::endl;
    //             LOG_DEBUG("Edge: ", edge->id, " is not connected to other edges");
    //             return false;
    //         }
    //         if (edge_to_set_mapping[first_edge->id] == -1 &&
    //             edge_to_set_mapping[second_edge->id] == -1) {
    //             edge_to_set_mapping[edge->id] = set_id;
    //             set_id++;
    //         }
    //         else if (edge_to_set_mapping[first_edge->id] != -1 &&
    //                  edge_to_set_mapping[second_edge->id] == -1) {
    //             edge_to_set_mapping[edge->id] = edge_to_set_mapping[first_edge->id];
    //             edge_to_set_mapping[second_edge->id] = edge_to_set_mapping[first_edge->id];
    //         }
    //         else if (edge_to_set_mapping[first_edge->id] == -1 &&
    //                  edge_to_set_mapping[second_edge->id] != -1) {
    //             edge_to_set_mapping[edge->id] = edge_to_set_mapping[second_edge->id];
    //             edge_to_set_mapping[first_edge->id] = edge_to_set_mapping[second_edge->id];
    //         }
    //         else if (edge_to_set_mapping[first_edge->id] !=
    //                  edge_to_set_mapping[second_edge->id]) {
    //             std::ptrdiff_t first_set_id = edge_to_set_mapping[first_edge->id];
    //             edge_to_set_mapping[edge->id] = first_set_id;
    //             for (std::ptrdiff_t j = 0; j < slitherlink->no_of_edges; ++j) {
    //                 if (edge_to_set_mapping[j] == edge_to_set_mapping[second_edge->id]) {
    //                     edge_to_set_mapping[j] = first_set_id;
    //                 }
    //             }
    //         }
    //     }
    // }

    // // print mapping
    // std::cout << "Edge to set mapping: ";
    // for (std::ptrdiff_t i = 0; i < slitherlink->no_of_edges; ++i) {
    //     std::cout << edge_to_set_mapping[i] << " ";
    // }
    // std::cout << std::endl;

    // std::ptrdiff_t final_set_id = -1;
    // for (std::ptrdiff_t i = 0; i < slitherlink->no_of_edges; ++i) {
    //     if (edge_to_set_mapping[i] != -1) {
    //         if (final_set_id == -1) {
    //             final_set_id = edge_to_set_mapping[i];
    //         }
    //         else if (edge_to_set_mapping[i] != final_set_id) {
    //             return false;
    //         }
    //     }
    // }

    return true;
}

bool Solver::addToLoops(std::ptrdiff_t edge_id) {
    if (edge_id == -1) {
        return false;
    }
    
    slitherlink_vertex* vertex_0 = slitherlink->vertices[slitherlink->edges[edge_id]->vertices[0]];
    slitherlink_vertex* vertex_1 = slitherlink->vertices[slitherlink->edges[edge_id]->vertices[1]];
    std::ptrdiff_t first_edge_id = -1;
    std::ptrdiff_t second_edge_id = -1;

    if (vertex_0->edge_refs[0]->id != edge_id &&
        vertex_0->edge_refs[0]->solution == EDGE_IN_SOLUTION) {
        first_edge_id = vertex_0->edge_refs[0]->id;
    }
    else if (vertex_0->edge_refs[1]->id != edge_id &&
             vertex_0->edge_refs[1]->solution == EDGE_IN_SOLUTION) {
        first_edge_id = vertex_0->edge_refs[1]->id;
    }

    if (vertex_1->edge_refs[0]->id != edge_id &&
        vertex_1->edge_refs[0]->solution == EDGE_IN_SOLUTION) {
        second_edge_id = vertex_1->edge_refs[0]->id;
    }
    else if (vertex_1->edge_refs[1]->id != edge_id &&
             vertex_1->edge_refs[1]->solution == EDGE_IN_SOLUTION) {
        second_edge_id = vertex_1->edge_refs[1]->id;
    }

    std::ptrdiff_t first_part_id = first_edge_id != -1 ?
                                    edge_to_loop_part[first_edge_id] :
                                    -1;
    std::ptrdiff_t second_part_id = second_edge_id != -1 ?
                                    edge_to_loop_part[second_edge_id] :
                                    -1;

    if (first_part_id == -1 && second_part_id == -1) {
        max_loop_part_id++;
        no_of_loop_parts++;
        edge_to_loop_part[edge_id] = max_loop_part_id;
        LOG_DEBUG("New loop part: ", max_loop_part_id, " for edge: ", edge_id);
    }
    else if (first_part_id == -1 && second_part_id != -1) {
        edge_to_loop_part[edge_id] = second_part_id;
        LOG_DEBUG("Edge: ", edge_id, " is in loop part: ", second_part_id);
    }
    else if (first_part_id != -1 && second_part_id == -1) {
        edge_to_loop_part[edge_id] = first_part_id;
        LOG_DEBUG("Edge: ", edge_id, " is in loop part: ", first_part_id);
    }
    else if (first_part_id != -1 && second_part_id != -1) {
        if (first_part_id == second_part_id) {
            if (no_of_loop_parts > 1) {
                LOG_DEBUG("Edge: ", edge_id, " is in closed loop leaving other parts behind: ", first_part_id);
                return false;
            }
            else {
                edge_to_loop_part[edge_id] = first_part_id;
                LOG_DEBUG("Edge: ", edge_id, " is in closed loop: ", first_part_id);
                return isSolved();
            }
        }
        else {
            for (std::ptrdiff_t i = 0; i < slitherlink->no_of_edges; ++i) {
                if (edge_to_loop_part[i] == second_part_id) {
                    edge_to_loop_part[i] = first_part_id;
                }
            }
            no_of_loop_parts--;
            edge_to_loop_part[edge_id] = first_part_id;
            LOG_DEBUG("Edge: ", edge_id, " is in last loop part: ", first_part_id);
        }
    }
    return true;
}

std::ptrdiff_t Solver::makeGuess() {

    solver_state* state = new solver_state;

    std::ptrdiff_t no_of_unknown_edges = 0;
    std::ptrdiff_t no_of_not_in_solution_edges = 0;
    std::ptrdiff_t no_of_in_solution_edges = 0;
    for (std::ptrdiff_t i = 0; i < slitherlink->no_of_edges; ++i) {
        if (slitherlink->edges[i]->solution == EDGE_UNKNOWN) {
            no_of_unknown_edges++;
        }
        else if (slitherlink->edges[i]->solution == EDGE_NOT_IN_SOLUTION) {
            no_of_not_in_solution_edges++;
        }
        else if (slitherlink->edges[i]->solution == EDGE_IN_SOLUTION) {
            no_of_in_solution_edges++;
        }

    }
    // assert(no_of_unknown_edges > 0);
    if (no_of_unknown_edges == 0) {
        LOG_DEBUG("No unknown edges");
        // slitherlink->savePuzzle("solver_no_unknown_edges.txt");
        delete state;
        return -1;
    }


    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(1, no_of_unknown_edges);
    std::ptrdiff_t random_edge = distrib(gen);
    std::ptrdiff_t edge_id = -1;
    for (std::ptrdiff_t i = 0; i < slitherlink->no_of_edges; i++) {
        if (slitherlink->edges[i]->solution == EDGE_UNKNOWN) {
            random_edge--;
            if (random_edge == 0) {
                edge_id = i;
                break;
            }
        }
    }

    assert(edge_id != -1);

    state->slitherlink = slitherlink->copy();
    state->edge_id = edge_id;
    state->edge_solution = EDGE_IN_SOLUTION;
    state->faces_solved = std::vector<bool>(faces_solved);
    state->vertices_solved = std::vector<bool>(vertices_solved);
    state->edge_to_loop_part = std::vector<std::ptrdiff_t>(edge_to_loop_part);
    state->max_loop_part_id = max_loop_part_id;
    state->no_of_loop_parts = no_of_loop_parts;
    state->totaly_even_set = std::vector<slitherlink_edge_type>(totaly_even_set);
    slitherlink_queue.push_back(state);

    // Possible TODO: randomize guess
    slitherlink->edges[edge_id]->solution = EDGE_IN_SOLUTION;

    LOG_DEBUG("Guessing edge ", edge_id, " to be in solution: ", slitherlink->edges[edge_id]->solution);

    push_edge(slitherlink->edges[edge_id]);

    return edge_id;

    // queue.push_back(std::make_pair(QUEUE_ITEM_FACE,
    //                                slitherlink->edges[edge_id]->face_ids[0]));
    // queue.push_back(std::make_pair(QUEUE_ITEM_FACE,
    //                                slitherlink->edges[edge_id]->face_ids[1]));
    // queue.push_back(std::make_pair(QUEUE_ITEM_VERTEX,
    //                                slitherlink->edges[edge_id]->vertices[0]));
    // queue.push_back(std::make_pair(QUEUE_ITEM_VERTEX,
    //                                slitherlink->edges[edge_id]->vertices[1]));
}

bool Solver::restoreGuess() {
    if (slitherlink_queue.empty()) {
        LOG_DEBUG("No more guesses to restore");
        return false;
    }

    delete slitherlink;

    solver_state* state = slitherlink_queue.back();
    slitherlink_queue.pop_back();
    slitherlink = state->slitherlink;
    edge_to_loop_part = state->edge_to_loop_part;
    max_loop_part_id = state->max_loop_part_id;
    no_of_loop_parts = state->no_of_loop_parts;
    faces_solved = state->faces_solved;
    vertices_solved = state->vertices_solved;
    totaly_even_set = state->totaly_even_set;

    slitherlink->edges[state->edge_id]->solution = (state->edge_solution == EDGE_IN_SOLUTION) ?
        EDGE_NOT_IN_SOLUTION : EDGE_IN_SOLUTION;
    
    queue.clear();

    push_edge(slitherlink->edges[state->edge_id]);

    delete state;

    return true;
}

bool Solver::updateFaceEdges(std::ptrdiff_t face_id) {

    slitherlink_face* face = slitherlink->faces[face_id];

    if (face->value == 0) {
        for (std::ptrdiff_t i = 0; i < face->no_of_edges; ++i) {
            if (face->edge_refs[i]->solution == EDGE_IN_SOLUTION) {
                LOG_DEBUG("Face with value 0 has edge in solution");
                return false;
            }
            if (face->edge_refs[i]->solution == EDGE_UNKNOWN) {
                face->edge_refs[i]->solution = EDGE_NOT_IN_SOLUTION;
                push_edge(face->edge_refs[i]);
                LOG_DEBUG("Push from face");
            }
        }
        return true;
    }

    std::ptrdiff_t edges_in_solution = 0;
    std::ptrdiff_t edges_unknown = 0;
    for (std::ptrdiff_t i = 0; i < face->no_of_edges; ++i) {
        if (face->edge_refs[i]->solution == EDGE_IN_SOLUTION) {
            edges_in_solution++;
        }
        else if (face->edge_refs[i]->solution == EDGE_UNKNOWN) {
            edges_unknown++;
        }
    }

    if (edges_in_solution > face->value) {
        LOG_DEBUG("Face ", face_id, " has too many edges in solution: ",
              edges_in_solution, " > ", face->value);
        return false;
    }


    LOG_DEBUG("Face ", face_id, " edges in solution: ", edges_in_solution,
        " edges unknown: ", edges_unknown);
    if (edges_in_solution == face->value) {
        for (std::ptrdiff_t i = 0; i < face->no_of_edges; ++i) {
            LOG_DEBUG("Face ", face_id, " edge id", face->edge_refs[i]->id);
            if (face->edge_refs[i]->solution == EDGE_UNKNOWN) {
                face->edge_refs[i]->solution = EDGE_NOT_IN_SOLUTION;
                push_edge(face->edge_refs[i]);
                LOG_DEBUG("Push from face - edge not in solution");
            }
        }
    }
    else if (edges_in_solution + edges_unknown == face->value) {
        for (std::ptrdiff_t i = 0; i < face->no_of_edges; ++i) {
            LOG_DEBUG("Face ", face_id, " edge id", face->edge_refs[i]->id);
            if (face->edge_refs[i]->solution == EDGE_UNKNOWN) {
                face->edge_refs[i]->solution = EDGE_IN_SOLUTION;
                push_edge(face->edge_refs[i]);
                LOG_DEBUG("Push from face - edge in solution");
                if (!addToLoops(face->edge_refs[i]->id)) {
                    LOG_DEBUG("Face ", face_id, " edge id", face->edge_refs[i]->id,
                          " creates wrong looping");
                    return false;
                }
            }
        }
    }
    return true;
}

bool Solver::updateVertexEdges(std::ptrdiff_t vertex_id) {
    slitherlink_vertex* vertex = slitherlink->vertices[vertex_id];

    std::ptrdiff_t edges_in_solution = 0;
    std::ptrdiff_t edges_unknown = 0;
    std::ptrdiff_t edges_not_in_solution = 0;
    for (std::ptrdiff_t i = 0; i < vertex->no_of_edges; ++i) {
        if (vertex->edge_refs[i]->solution == EDGE_IN_SOLUTION) {
            edges_in_solution++;
        }
        else if (vertex->edge_refs[i]->solution == EDGE_UNKNOWN) {
            edges_unknown++;
        }
        else if (vertex->edge_refs[i]->solution == EDGE_NOT_IN_SOLUTION) {
            edges_not_in_solution++;
        }
    }
    if (edges_in_solution > 2) {
        LOG_DEBUG("Vertex has more than 2 edges in solution");
        return false;
    }

    LOG_DEBUG("Vertex ", vertex_id, " edges in solution: ", edges_in_solution,
        " edges unknown: ", edges_unknown);

    if (edges_unknown > 0) {
        if (edges_in_solution == 2) {
            for (std::ptrdiff_t i = 0; i < vertex->no_of_edges; ++i) {
                if (vertex->edge_refs[i]->solution == EDGE_UNKNOWN) {
                    vertex->edge_refs[i]->solution = EDGE_NOT_IN_SOLUTION;
                    push_edge(vertex->edge_refs[i]);
                    LOG_DEBUG("Push from vertex");
                }
            }
        }
        else if ((edges_in_solution == 1) &&
                 (edges_unknown == 1)) {
            for (std::ptrdiff_t i = 0; i < vertex->no_of_edges; ++i) {
                if (vertex->edge_refs[i]->solution == EDGE_UNKNOWN) {
                    vertex->edge_refs[i]->solution = EDGE_IN_SOLUTION;
                    push_edge(vertex->edge_refs[i]);
                    LOG_DEBUG("Push from vertex");
                    if (!addToLoops(vertex->edge_refs[i]->id)) {
                        LOG_DEBUG("Vertex ", vertex_id, " edge id", vertex->edge_refs[i]->id,
                              " creates wrong looping");
                        return false;
                    }
                }
            }
        }
        else if (vertex->no_of_edges - edges_not_in_solution == 1) {
            for (std::ptrdiff_t i = 0; i < vertex->no_of_edges; ++i) {
                if (vertex->edge_refs[i]->solution == EDGE_UNKNOWN) {
                    vertex->edge_refs[i]->solution = EDGE_NOT_IN_SOLUTION;
                    push_edge(vertex->edge_refs[i]);
                    LOG_DEBUG("Push from vertex");
                }
            }
        }
    }
    return true;
}
