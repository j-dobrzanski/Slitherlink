#include "../api/solver.hpp"
#include "../../utilities/api/trace_lib.hpp"

#include <array>
#include <fstream>
#include <sstream>
#include <algorithm>

// Local function definitions
bool isOuter(slitherlink_edge* edge_p) {
    return (edge_p->face_ids[0] == -1 ||
            edge_p->face_ids[1] == -1);
}

bool isValidState(rule_state* rule) {
    if (rule->base_size == 2) {

        std::ptrdiff_t no_of_edges_in_solution = 0;
        for (std::ptrdiff_t i = 0; i < rule->faces[0]->no_of_edges; i++) {
            slitherlink_edge* edge = rule->faces[0]->edge_refs[i];
            if (edge->solution == EDGE_IN_SOLUTION) {
                no_of_edges_in_solution++;
            }
        }
        if (no_of_edges_in_solution > rule->faces[0]->value) {
            LOG_DEBUG("Face has too many edges in solution: ",
                no_of_edges_in_solution, " > ", rule->faces[0]->value);
            return false;
        }

        for (std::ptrdiff_t i = 0; i < 6; i++) {
            slitherlink_vertex* vertex = rule->vertices[i];
            no_of_edges_in_solution = 0;
            std::ptrdiff_t no_of_unknown_edges = 0;
            for (std::ptrdiff_t j = 0; j < vertex->no_of_edges; j++) {
                slitherlink_edge* edge = vertex->edge_refs[j];
                if (edge->solution == EDGE_IN_SOLUTION) {
                    no_of_edges_in_solution++;
                }
                else if (edge->solution == EDGE_UNKNOWN) {
                    no_of_unknown_edges++;
                }
            }
            if ((no_of_edges_in_solution > 2) ||
                ((no_of_edges_in_solution == 1) &&
                 (no_of_unknown_edges == 0))) {
                LOG_DEBUG("Vertex has wrong number of edges in solution");
                return false;
            }
        }
        return true;     
    }

    return false;
}

bool isSolvedState(rule_state* rule) {
    LOG_DEBUG("Checking solved state");
    if (rule->base_size == 2) {
        std::ptrdiff_t no_of_edges_in_solution = 0;
        for (std::ptrdiff_t i = 0; i < rule->faces[0]->no_of_edges; i++) {
            slitherlink_edge* edge = rule->faces[0]->edge_refs[i];
            if (edge->solution == EDGE_IN_SOLUTION) {
                no_of_edges_in_solution++;
            }
        }
        if (no_of_edges_in_solution != rule->faces[0]->value) {
            return false;
        }
        for (std::ptrdiff_t i = 0; i < 6; i++) {
            slitherlink_vertex* vertex = rule->vertices[i];
            no_of_edges_in_solution = 0;
            std::ptrdiff_t no_of_unknown_edges = 0;
            for (std::ptrdiff_t j = 0; j < vertex->no_of_edges; j++) {
                slitherlink_edge* edge = vertex->edge_refs[j];
                if (edge->solution == EDGE_IN_SOLUTION) {
                    no_of_edges_in_solution++;
                }
                else if (edge->solution == EDGE_UNKNOWN) {
                    no_of_unknown_edges++;
                }
            }
            if ((no_of_edges_in_solution != 2) &&
                (no_of_edges_in_solution != 0)) {
                return false;
            }
        }
        return true;
    }
    return false;
}

bool edgeCounterBase(std::vector<slitherlink_edge*> edges) {
    for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)edges.size(); i++) {
        slitherlink_edge* edge = edges[i];
        LOG_DEBUG("Edge counter: ", edge->id, " is ", edge->solution);
        if (edge->solution == EDGE_UNKNOWN) {
            edge->solution = EDGE_IN_SOLUTION;
            LOG_DEBUG("Edge counter: ", edge->id, " is now in solution");
            return true;
        }
        else if (edge->solution == EDGE_IN_SOLUTION) {
            edge->solution = EDGE_NOT_IN_SOLUTION;
            LOG_DEBUG("Edge counter: ", edge->id, " is now not in solution");
            return true;
        }
        else if (edge->solution == EDGE_NOT_IN_SOLUTION) {
            LOG_DEBUG("Edge counter: ", edge->id, " is now unknown");
            edge->solution = EDGE_UNKNOWN;
        }
    }
    return false;
}

bool edgeCounterState(std::vector<slitherlink_edge*> edges) {
    for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)edges.size(); i++) {
        slitherlink_edge* edge = edges[i];
        LOG_DEBUG("Edge counter: ", edge->id, " is ", edge->solution);
        if (edge->solution == EDGE_UNKNOWN) {
            ERROR("Edge counter: ", edge->id, " is unknown while getting next state");
            return false;
        }
        else if (edge->solution == EDGE_IN_SOLUTION) {
            edge->solution = EDGE_NOT_IN_SOLUTION;
            LOG_DEBUG("Edge counter: ", edge->id, " is now not in solution");
            return true;
        }
        else if (edge->solution == EDGE_NOT_IN_SOLUTION) {
            LOG_DEBUG("Edge counter: ", edge->id, " is now in solution");
            edge->solution = EDGE_IN_SOLUTION;
        }
    }
    return false;
}

bool faceCounter(std::vector<slitherlink_face*> faces) {
    for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)faces.size(); i++) {
        slitherlink_face* face = faces[i];
        face->value = (face->value + 1) % face->no_of_edges;
        LOG("Face value changed: ", face->id, " to ", face->value);
        if (face->value != 0) {
            return true;
        }
    }
    return false;
}

void updateRuleRefs(rule_state* rule) {
    if (rule->base_size == 2) {
        rule->faces[0]->edge_refs = {rule->edges[0], rule->edges[1], rule->edges[2], rule->edges[3], rule->edges[4], rule->edges[5]};
        rule->faces[0]->face_refs = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
        


        for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)rule->vertices.size(); i++) {
            slitherlink_vertex* vertex = rule->vertices[i];
            for (std::ptrdiff_t j = 0; j < vertex->no_of_edges; j++) {
                rule->vertices[i]->edge_refs.push_back(rule->edges[vertex->edge_ids[j]]);
            }
        }

        for (std::ptrdiff_t i = 0; i < 6; i++) {
            slitherlink_edge* edge = rule->edges[i];
            edge->vertex_refs[0] = rule->vertices[edge->vertices[0]];
            edge->vertex_refs[1] = rule->vertices[edge->vertices[1]];
            edge->face_refs[0] = rule->faces[edge->face_ids[0]];
            edge->face_refs[1] = nullptr;
        }

        for (std::ptrdiff_t i = 6; i < 12; i++) {
            slitherlink_edge* edge = rule->edges[i];
            edge->vertex_refs[0] = rule->vertices[edge->vertices[0]];
            edge->vertex_refs[1] = rule->vertices[edge->vertices[1]];
            edge->face_refs[0] = nullptr;
            edge->face_refs[1] = nullptr;
        }

        for (std::ptrdiff_t i = 0; i < 6; i++) {
            rule->edges[i]->face_refs[0] = rule->faces[0];
            rule->edges[i]->face_refs[1] = nullptr;
        }
    }
}

bool getNextBase(rule_state* rule_state) {

    if (rule_state->base_size == 2) {
        if(!edgeCounterBase(rule_state->edges)) {
            return faceCounter(rule_state->faces);
        }
        return true;
    }

    return false;
}

bool getNextState(rule_state* base,
                  rule_state* rule_state) {
    std::vector<slitherlink_edge*> edges;
    if (base->edges.size() != rule_state->edges.size()) {
        ERROR("Base and rule state have different number of edges");
        return false;
    }
    for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)base->edges.size(); i++) {
        if (base->edges[i]->solution == EDGE_UNKNOWN) {
            edges.push_back(rule_state->edges[i]);
        }
    }
    if (edges.size() == 0) {
        return false;
    }
    return edgeCounterState(edges);
}

bool addSolutionToRule(rule_state* base,
                       std::vector<slitherlink_edge_type>* current_rule,
                       std::vector<slitherlink_edge_type>* new_solution) {
    if ((base->edges.size() != current_rule->size()) ||
        (base->edges.size() != new_solution->size())) {
        return false;
    }

    bool common_part = false;
    for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)base->edges.size(); i++) {
        if ((base->edges[i]->solution != EDGE_UNKNOWN) ||
            ((*current_rule)[i] == EDGE_UNKNOWN)) {
            continue;
        }
        else if((*current_rule)[i] != (*new_solution)[i]) {
            (*current_rule)[i] = EDGE_UNKNOWN;
        }
        else if ((*current_rule)[i] == (*new_solution)[i]) {
            common_part = true;
        }
    }
    return common_part;
}

std::ptrdiff_t isReducible(rule* first,
                 rule* second) {
    // // print derived edge states
    // for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)first->derived_edge_states.size(); i++) {
    //     std::cout << first->derived_edge_states[i] << " ";
    // }
    // std::cout << std::endl;
    // for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)second->derived_edge_states.size(); i++) {
    //     std::cout << second->derived_edge_states[i] << " ";
    // }
    // std::cout << std::endl;
    if (first->derived_edge_states.size() != second->derived_edge_states.size()) {
        return -1; // not reducible
    }
    for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)first->derived_edge_states.size(); i++) {
        if (first->derived_edge_states[i] != second->derived_edge_states[i]) {
            return -1; // not reducible
        }
    }
    // std::cout << "Derived equal" << std::endl;
    std::ptrdiff_t stronger = 0;
    for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)first->base_edge_states.size(); i++) {
        if (first->base_edge_states[i] != second->base_edge_states[i]) {
            if (first->base_edge_states[i] == EDGE_UNKNOWN) {
                if (stronger == 2) {
                    return -1; // not reducible
                }
                stronger = 1; 
            }
            else if (second->base_edge_states[i] == EDGE_UNKNOWN) {
                if (stronger == 1) {
                    return -1; // not reducible
                }
                stronger = 2;
            }
            else {
                return -1; // not reducible
            }
        }
    }
    return stronger; // 1 - first is stronger, 2 - second is stronger, 0 - equal, -1 - not reducible
}

bool checkIfSimpleRulesapply(rule* rule_p,
                             rule_state* base,
                             rule_state* derived) {
    for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)rule_p->base_edge_states.size(); i++) {
        base->edges[i]->solution = rule_p->base_edge_states[i];
    }

    for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)rule_p->derived_edge_states.size(); i++) {
        derived->edges[i]->solution = rule_p->derived_edge_states[i];
    }

    for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)rule_p->face_values.size(); i++) {
        base->faces[i]->value = rule_p->face_values[i];
        derived->faces[i]->value = rule_p->face_values[i];
    }

    // // print base edges
    // std::cout << "Base edges: ";
    // for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)base->edges.size(); i++) {
    //     std::cout << base->edges[i]->solution << " ";
    // }
    // std::cout << std::endl;

    // apply vertice rule
    for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)base->vertices.size(); i++) {
        if(base->vertices[i]->no_of_edges == 1) {
            continue;
        }
        slitherlink_vertex* vertex = base->vertices[i];
        std::ptrdiff_t no_of_edges_in_solution = 0;
        std::ptrdiff_t no_of_unknown_edges = 0;
        for (std::ptrdiff_t j = 0; j < vertex->no_of_edges; j++) {
            slitherlink_edge* edge = vertex->edge_refs[j];
            if (edge->solution == EDGE_IN_SOLUTION) {
                no_of_edges_in_solution++;
            }
            else if (edge->solution == EDGE_UNKNOWN) {
                no_of_unknown_edges++;
            }
        }
        if (no_of_edges_in_solution == 1 &&
            no_of_unknown_edges == 1) {
            for (std::ptrdiff_t j = 0; j < vertex->no_of_edges; j++) {
                slitherlink_edge* edge = vertex->edge_refs[j];
                if (edge->solution == EDGE_UNKNOWN) {
                    edge->solution = EDGE_IN_SOLUTION;
                }
            }
        }
        else if (no_of_edges_in_solution == 0 &&
                 no_of_unknown_edges == 1) {
            for (std::ptrdiff_t j = 0; j < vertex->no_of_edges; j++) {
                slitherlink_edge* edge = vertex->edge_refs[j];
                if (edge->solution == EDGE_UNKNOWN) {
                    edge->solution = EDGE_NOT_IN_SOLUTION;
                }
            }
        }
        else if (no_of_edges_in_solution == 2 &&
                 no_of_unknown_edges > 0) {
            for (std::ptrdiff_t j = 0; j < vertex->no_of_edges; j++) {
                slitherlink_edge* edge = vertex->edge_refs[j];
                if (edge->solution == EDGE_UNKNOWN) {
                    edge->solution = EDGE_NOT_IN_SOLUTION;
                }
            }
        }
    }

    // std::cout << "Edge states after applying rule: " << std::endl;
    // for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)base->edges.size(); i++) {
    //     std::cout << base->edges[i]->solution << " ";
    // }
    // std::cout << std::endl;

    // apply face rule
    for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)base->faces.size(); i++) {
        slitherlink_face* face = base->faces[i];
        std::ptrdiff_t no_of_edges_in_solution = 0;
        std::ptrdiff_t no_of_unknown_edges = 0;
        for (std::ptrdiff_t j = 0; j < face->no_of_edges; j++) {
            slitherlink_edge* edge = face->edge_refs[j];
            if (edge->solution == EDGE_IN_SOLUTION) {
                no_of_edges_in_solution++;
            }
            else if (edge->solution == EDGE_UNKNOWN) {
                no_of_unknown_edges++;
            }
        }
        if (no_of_edges_in_solution == face->value) {
            for (std::ptrdiff_t j = 0; j < face->no_of_edges; j++) {
                slitherlink_edge* edge = face->edge_refs[j];
                if (edge->solution == EDGE_UNKNOWN) {
                    edge->solution = EDGE_NOT_IN_SOLUTION;
                }
            }
        }
        else if (no_of_edges_in_solution + no_of_unknown_edges == face->value) {
            for (std::ptrdiff_t j = 0; j < face->no_of_edges; j++) {
                slitherlink_edge* edge = face->edge_refs[j];
                if (edge->solution == EDGE_UNKNOWN) {
                    edge->solution = EDGE_IN_SOLUTION;
                }
            }
        }
    }

    // std::cout << "Edge states after applying rule: " << std::endl;
    // for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)base->edges.size(); i++) {
    //     std::cout << base->edges[i]->solution << " ";
    // }
    // std::cout << std::endl;

    // apply vertice rule again
    for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)base->vertices.size(); i++) {
        if(base->vertices[i]->no_of_edges == 1) {
            continue;
        }
        slitherlink_vertex* vertex = base->vertices[i];
        std::ptrdiff_t no_of_edges_in_solution = 0;
        std::ptrdiff_t no_of_unknown_edges = 0;
        for (std::ptrdiff_t j = 0; j < vertex->no_of_edges; j++) {
            slitherlink_edge* edge = vertex->edge_refs[j];
            if (edge->solution == EDGE_IN_SOLUTION) {
                no_of_edges_in_solution++;
            }
            else if (edge->solution == EDGE_UNKNOWN) {
                no_of_unknown_edges++;
            }
        }
        if (no_of_edges_in_solution == 1 &&
            no_of_unknown_edges == 1) {
            for (std::ptrdiff_t j = 0; j < vertex->no_of_edges; j++) {
                slitherlink_edge* edge = vertex->edge_refs[j];
                if (edge->solution == EDGE_UNKNOWN) {
                    edge->solution = EDGE_IN_SOLUTION;
                }
            }
        }
        else if (no_of_edges_in_solution == 0 &&
                 no_of_unknown_edges == 1) {
            for (std::ptrdiff_t j = 0; j < vertex->no_of_edges; j++) {
                slitherlink_edge* edge = vertex->edge_refs[j];
                if (edge->solution == EDGE_UNKNOWN) {
                    edge->solution = EDGE_NOT_IN_SOLUTION;
                }
            }
        }
        else if (no_of_edges_in_solution == 2 &&
                 no_of_unknown_edges > 0) {
            for (std::ptrdiff_t j = 0; j < vertex->no_of_edges; j++) {
                slitherlink_edge* edge = vertex->edge_refs[j];
                if (edge->solution == EDGE_UNKNOWN) {
                    edge->solution = EDGE_NOT_IN_SOLUTION;
                }
            }
        }
    }

    // //print edge states
    // std::cout << "Edge states after applying rule: " << std::endl;
    // for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)base->edges.size(); i++) {
    //     std::cout << base->edges[i]->solution << " ";
    // }
    // std::cout << std::endl;
    // std::cout << "Derived edge states after applying rule: " << std::endl;
    // for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)derived->edges.size(); i++) {
    //     std::cout << derived->edges[i]->solution << " ";
    // }
    // std::cout << std::endl;
    for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)base->edges.size(); i++) {
        if (base->edges[i]->solution != derived->edges[i]->solution) {
            return false;
        }
    }

    return true;
}

void Solver::saveRule(std::ofstream* file,
                      rule* rule_p) {
    if (!file || !(*file)) {
        ERROR("Can't open file for saving rules");
    }
    *file << "# Rule: " << std::endl;
    // file << "# Face values: " << std::endl;
    for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)rule_p->face_values.size(); i++) {
        *file << rule_p->face_values[i] << " ";
    }
    *file << std::endl;
    // file << "# Base edge states: " << std::endl;
    for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)rule_p->base_edge_states.size(); i++) {
        *file << rule_p->base_edge_states[i] << " ";
    }
    *file << std::endl;
    if (rule_p->is_positive) {
        // file << "# Positive rule: " << std::endl;
        *file << rule_p->is_positive << std::endl;
        for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)rule_p->derived_edge_states.size(); i++) {
            *file << rule_p->derived_edge_states[i] << " ";
        }
        *file << std::endl;
    }
    else {
        // file << "# Negative rule: " << std::endl;
        *file << rule_p->is_positive << std::endl;
    }
}

rule* Solver::readRule(std::ifstream* file) {
    if (!file || !(*file)) {
        ERROR("Can't open file for reading rules");
    }

    rule* new_rule = new rule();
    std::string line;

    // Read face values
    std::getline(*file, line);
    std::istringstream face_values_stream(line);
    while (face_values_stream >> line) {
        new_rule->face_values.push_back(std::stoi(line));
    }

    // Read base edge states
    std::getline(*file, line);
    std::istringstream base_edge_states_stream(line);
    while (base_edge_states_stream >> line) {
        new_rule->base_edge_states.push_back(static_cast<slitherlink_edge_type>(std::stoi(line)));
    }

    // Read is_positive flag
    std::getline(*file, line);
    new_rule->is_positive = (line == "1");

    if (new_rule->is_positive) {
        // Read derived edge states
        std::getline(*file, line);
        std::istringstream derived_edge_states_stream(line);
        while (derived_edge_states_stream >> line) {
            new_rule->derived_edge_states.push_back(static_cast<slitherlink_edge_type>(std::stoi(line)));
        }
    }

    return new_rule;
}

std::pair<bool, std::vector<slitherlink_edge_type>> searchForRules(rule_state* base) {
    rule_state* derived = base->copy();

    // We check only states that are fully decided:
    for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)derived->edges.size(); i++) {
        if (derived->edges[i]->solution == EDGE_UNKNOWN) {
            derived->edges[i]->solution = EDGE_IN_SOLUTION;
        }
    }

    if (!isValidState(base)) {
        delete derived;
        return {false, {}};
    }

    std::pair<bool, std::vector<slitherlink_edge_type>> result = {false, {}};
    while (getNextState(base, derived)) {

        if (isValidState(derived) && isSolvedState(derived)) {
            if (!result.first) {
                result.first = true;
                for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)derived->edges.size(); i++) {
                    result.second.push_back(derived->edges[i]->solution);
                }
            }
            else{
                std::vector<slitherlink_edge_type> new_solution;
                for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)derived->edges.size(); i++) {
                    new_solution.push_back(derived->edges[i]->solution);
                }
                if (!addSolutionToRule(base, &(result.second), &new_solution)) {
                    delete derived;
                    return {false, {}};
                }
            }
        }
    }

    delete derived;
    return {true, result.second};
}

rule_state* rule_state::copy() {
    rule_state* copy = new rule_state();
    copy->base_size = base_size;
    copy->no_of_vertices = no_of_vertices;
    copy->no_of_edges = no_of_edges;
    copy->no_of_faces = no_of_faces;

    for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)vertices.size(); i++) {
        slitherlink_vertex* vertex = new slitherlink_vertex();
        *vertex = *vertices[i];
        vertex->edge_refs.clear();
        copy->vertices.push_back(vertex);
    }

    for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)edges.size(); i++) {
        slitherlink_edge* edge = new slitherlink_edge();
        *edge = *edges[i];
        edge->vertex_refs[0] = nullptr;
        edge->vertex_refs[1] = nullptr;
        edge->face_refs[0] = nullptr;
        edge->face_refs[1] = nullptr;
        copy->edges.push_back(edge);
    }

    for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)faces.size(); i++) {
        slitherlink_face* face = new slitherlink_face();
        *face = *faces[i];
        face->edge_refs.clear();
        face->face_refs.clear();
        copy->faces.push_back(face);
    }

    updateRuleRefs(copy);

    return copy;
}

rule_state::~rule_state() {
    if (is_copied) {
        return; // Do not delete if it is a copy
    }
    for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)vertices.size(); i++) {
        delete vertices[i];
    }
    for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)edges.size(); i++) {
        delete edges[i];
    }
    for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)faces.size(); i++) {
        delete faces[i];
    }
}

void Solver::generateAndSaveRules(std::string file_name,
                                  std::ptrdiff_t base_size) {
    (void) file_name;
    rule_state* base = generateBase(base_size);
    std::vector<rule*> rules = generateRules(base);

    std::ofstream file(file_name);
    if (!file) {
        ERROR("Can't open file of name: ", file_name);
    }
    file << "# Base size: " << base_size << std::endl;
    for (rule* rule : rules) {
        saveRule(&file, rule);
        delete rule;
    }
    delete base;
}

rule_state* Solver::generateBase(std::ptrdiff_t base_size) {
    rule_state* rule = new rule_state();
    rule->base_size = base_size;

    if (base_size == 2) {

        for (std::ptrdiff_t i = 0; i < 6; i++) {
            slitherlink_vertex* vertex = new slitherlink_vertex();
            vertex->id = i;
            vertex->no_of_edges = 3;
            vertex->edge_ids = {(i + 5) % 6, i, i + 6};
            
            rule->vertices.push_back(vertex);
        }
        for (std::ptrdiff_t i = 6; i < 12; i++) {
            slitherlink_vertex* vertex = new slitherlink_vertex();
            vertex->id = i;
            vertex->no_of_edges = 1;
            vertex->edge_ids = {i};
            
            rule->vertices.push_back(vertex);
        }

        for (std::ptrdiff_t i = 0; i < 6; i++) {
            slitherlink_edge* edge = new slitherlink_edge();
            edge->id = i;
            edge->vertices[0] = i;
            edge->vertices[1] = (i + 1) % 6;
            edge->face_ids[0] = 0;
            edge->face_ids[1] = -1;
            edge->solution = EDGE_UNKNOWN;
            
            rule->edges.push_back(edge);
        }

        for (std::ptrdiff_t i = 6; i < 12; i++) {
            slitherlink_edge* edge = new slitherlink_edge();
            edge->id = i;
            edge->vertices[0] = i - 6;
            edge->vertices[1] = i;
            edge->face_ids[0] = -1;
            edge->face_ids[1] = -1;
            edge->solution = EDGE_UNKNOWN;
            
            rule->edges.push_back(edge);
        }

        slitherlink_face* face = new slitherlink_face();
        face->id = 0;
        face->value = 1;
        face->no_of_edges = 6;
        face->edge_ids = {0, 1, 2, 3, 4, 5};
        rule->faces.push_back(face);

        updateRuleRefs(rule);
    }

    return rule;
}

std::vector<rule*> Solver::generateRules(rule_state* base) {
    std::vector<rule*> rules;

    while (getNextBase(base)) {
        if (!isValidState(base)) {
            LOG_DEBUG("Base is not valid");
            continue;
        }

        std::pair<bool, std::vector<slitherlink_edge_type>> result = searchForRules(base);
        if (result.first) {
            rule* new_rule = new rule();
            for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)base->edges.size(); i++) {
                new_rule->base_edge_states.push_back(base->edges[i]->solution);
            }
            for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)base->faces.size(); i++) {
                new_rule->face_values.push_back(base->faces[i]->value);
            }
            if (result.second.size() != 0) {
                new_rule->is_positive = true;
                for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)result.second.size(); i++) {
                    new_rule->derived_edge_states.push_back(result.second[i]);
                }
            }
            else {
                new_rule->is_positive = false;
            }
            rules.push_back(new_rule);
        }
    }

    return rules;
}

void Solver::refineAndSaveRules(std::string raw_rules_file_name,
                                std::string refined_rules_file_name,
                                std::ptrdiff_t base_size) {
    std::ifstream file(raw_rules_file_name);
    if (!file) {
        ERROR("Can't open file of name: ", raw_rules_file_name);
    }
    std::string line;
    std::getline(file, line);
    if (line != "# Base size: " + std::to_string(base_size)) {
        ERROR("File does not match base size: ", base_size);
    }
    rule_state* base = generateBase(base_size);
    rule_state* derived = generateBase(base_size);
    std::vector<std::vector<rule*>> rules(6);

    std::getline(file, line);

    while(line == "# Rule: "){
        rule* new_rule = readRule(&file);
        rules[new_rule->face_values[0]].push_back(new_rule);
        std::getline(file, line);
    }
    file.close();

    std::cout << "Refining rules..." << std::endl;

    for (std::ptrdiff_t i = 1; i < (std::ptrdiff_t)rules.size(); i++) {
        std::cout << "Refining rules for face value: " << i << std::endl;
        std::vector<rule*>& rules_for_face = rules[i];

        std::cout << "Number of rules before refinement: " << rules_for_face.size() << std::endl; 
        // refine rules
        std::vector<rule*> refined_rules;
        std::vector<bool> reduced_rules(rules_for_face.size(), true);
        for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)rules_for_face.size(); i++) {
            if (reduced_rules[i]) {
                rule* rule_p = rules_for_face[i];
                for (std::ptrdiff_t j = i + 1; j < (std::ptrdiff_t)rules_for_face.size(); j++) {
                    std::ptrdiff_t stronger = isReducible(rule_p, rules_for_face[j]);
                    if (stronger == 1) {
                        reduced_rules[j] = false;
                    }
                    else if (stronger == 2) {
                        reduced_rules[i] = false;
                    }
                }                
            }
        }

        for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)rules_for_face.size(); i++) {
            if (reduced_rules[i]) {
                refined_rules.push_back(rules_for_face[i]);
            }
            else {
                delete rules_for_face[i];
            }
        }

        std::cout << "Number of rules after first reduction: ";
        std::cout << refined_rules.size() << std::endl;

        rules_for_face = refined_rules;
        refined_rules.clear();

        for (rule* rule_p : rules_for_face) {
            if (!rule_p->is_positive) {
                refined_rules.push_back(rule_p);
                continue;
            }
            if (!checkIfSimpleRulesapply(rule_p, base, derived)) {
                refined_rules.push_back(rule_p);
            }
            else {
                delete rule_p;
            }
        }

        std::cout << "Number of rules after second reduction: ";
        std::cout << refined_rules.size() << std::endl;

        rules_for_face = refined_rules;
        refined_rules.clear();
    }


    // save rules
    std::ofstream out_file(refined_rules_file_name);
    if (!out_file) {
        ERROR("Can't open file of name: ", refined_rules_file_name);
    }
    out_file << "# Base size: " << base_size << std::endl;
    for (std::vector<rule*> rule_vector_p : rules) {
        for (rule* rule_p : rule_vector_p) {
            saveRule(&out_file, rule_p);
            delete rule_p;
        }
    }

    delete base;
    delete derived;
    out_file.close();
}

void Solver::applyRules(std::vector<rule*> rules,
                        slitherlink_face* face_p) {
    if (!face_p) {
        ERROR("Face is null");
        return;
    }

    for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)face_p->face_ids.size(); i++) {
        if (face_p->face_ids[i] == OUTER_FACE) {
            return;
        }
    }

    rule_state* base = new rule_state();
    base->is_copied = true; // This is a copied base, do not delete it in destructor
    base->base_size = 2;
    base->no_of_vertices = 12;
    base->no_of_edges = 12;
    base->no_of_faces = 1;
    base->faces.push_back(face_p);

    base->vertices.push_back(face_p->edge_refs[0]->vertex_refs[0]);
    slitherlink_vertex* vertex = base->vertices[0];
    // std::cout << "Starting vertex: " << (base->vertices[0] ? std::to_string(base->vertices[0]->id) : "null") << std::endl;
    while (base->vertices.size() != 6) {
        // std::cout << "Current vertex: " << (vertex ? std::to_string(vertex->id) : "null") << std::endl;
        // std::cout << "Vertices in base: ";
        // for (slitherlink_vertex* v : base->vertices) {
        //     std::cout << v->id << " ";
        // }
        // std::cout << std::endl;
        for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)vertex->no_of_edges; i++) {
            slitherlink_edge* edge = vertex->edge_refs[i];
            if (std::find(face_p->edge_refs.begin(), face_p->edge_refs.end(), edge) != face_p->edge_refs.end()) {
                // std::cout << "Found edge in face: " << edge->id << std::endl;
                auto first = std::find(base->vertices.begin(), base->vertices.end(), edge->vertex_refs[0]);
                auto second = std::find(base->vertices.begin(), base->vertices.end(), edge->vertex_refs[1]);
                if (first == base->vertices.end()) {
                    vertex = edge->vertex_refs[0];
                    base->vertices.push_back(vertex);
                    base->edges.push_back(edge);
                }
                else if (second == base->vertices.end()) {
                    vertex = edge->vertex_refs[1];
                    base->vertices.push_back(vertex);
                    base->edges.push_back(edge);
                }
            }
        }
    }
    for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)vertex->no_of_edges; i++) {
        slitherlink_edge* edge = vertex->edge_refs[i];
        if (std::find(face_p->edge_refs.begin(), face_p->edge_refs.end(), edge) != face_p->edge_refs.end()) {
            if (edge->vertex_refs[0] == base->vertices[0] ||
                edge->vertex_refs[1] == base->vertices[0]) {
                base->edges.push_back(edge);
            }
        }
    }


    for (std::ptrdiff_t i = 0; i < 6; i++) {
        slitherlink_vertex* vertex = base->vertices[i];
        for (std::ptrdiff_t j = 0; j < vertex->no_of_edges; j++) {
            if (std::find(base->vertices.begin(), base->vertices.end(), vertex->edge_refs[j]->vertex_refs[0]) == base->vertices.end()) {
                base->edges.push_back(vertex->edge_refs[j]);
                base->vertices.push_back(vertex->edge_refs[j]->vertex_refs[0]);
            }
            else if (std::find(base->vertices.begin(), base->vertices.end(), vertex->edge_refs[j]->vertex_refs[1]) == base->vertices.end()) {
                base->edges.push_back(vertex->edge_refs[j]);
                base->vertices.push_back(vertex->edge_refs[j]->vertex_refs[1]);
            }
        }
    }

    for (std::ptrdiff_t i = 0; i < 6; i++) {
        vertex = base->vertices[6 + i];
        for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)vertex->no_of_edges; i++) {
            slitherlink_edge* edge = vertex->edge_refs[i];
            if (std::find(face_p->edge_refs.begin(), face_p->edge_refs.end(), edge) == face_p->edge_refs.end()) {
                auto first = std::find(base->vertices.begin(), base->vertices.end(), edge->vertex_refs[0]);
                auto second = std::find(base->vertices.begin(), base->vertices.end(), edge->vertex_refs[1]);
                if (first == base->vertices.end()) {
                    base->edges.push_back(edge);
                }
                else if (second == base->vertices.end()) {
                    base->edges.push_back(edge);
                }
            }
        }
    }

    for(rule* rule_p : rules) {
        bool base_correct = true;
        for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)rule_p->face_values.size(); i++) {
            if (rule_p->face_values[i] != face_p->value) {
                base_correct = false;
                break;
            }
        }

        if (!base_correct) {
            continue;
        }

        for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)rule_p->base_edge_states.size(); i++) {
            if (rule_p->base_edge_states[i] != base->edges[i]->solution) {
                base_correct = false;
                break;
            }
        }

        if (!base_correct) {
            continue;
        }

        for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)rule_p->derived_edge_states.size(); i++) {
            if (base->edges[i]->solution != rule_p->derived_edge_states[i]) {
                base->edges[i]->solution = rule_p->derived_edge_states[i];
                push_edge(base->edges[i]);
            }
        }
        delete base;
        return;
    }
    delete base;
}

void Solver::loadRules(std::string file_name,
                       std::ptrdiff_t base_size) {
    std::ifstream file(file_name);
    if (!file) {
        ERROR("Can't open file of name: ", file_name);
    }
    std::string line;
    std::getline(file, line);
    if (line != "# Base size: " + std::to_string(base_size)) {
        ERROR("File does not match base size: ", base_size);
    }

    std::getline(file, line);

    while(line == "# Rule: "){
        rule* new_rule = readRule(&file);
        ready_rules.push_back(new_rule);
        std::getline(file, line);
    }
    file.close();
    LOG_DEBUG("Loaded ", ready_rules.size(), " rules from file: ", file_name);
}

void Solver::test() {
    rule_state* base = generateBase(2);
    std::cout << "Base: " << std::endl;
    for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)base->edges.size(); i++) {
        std::cout << base->edges[i]->id << ":" << base->edges[i]->solution << " ";
    }
    std::cout << std::endl;
    std::cout << "Vertices: " << std::endl;
    for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)base->vertices.size(); i++) {
        std::cout << base->vertices[i]->id << ":" << base->vertices[i]->no_of_edges << "={";
        for (std::ptrdiff_t j = 0; j < base->vertices[i]->no_of_edges; j++) {
            std::cout << base->vertices[i]->edge_ids[j] << ",";
        }
        std::cout << "}" << std::endl;
    }
    base->edges[0]->solution = EDGE_NOT_IN_SOLUTION;
    std::cout << "isValidState: " << isValidState(base) << std::endl;
    std::cout << "isSolvedState: " << isSolvedState(base) << std::endl;

    std::pair<bool, std::vector<slitherlink_edge_type>> result = searchForRules(base);
    if (result.first) {
        std::cout << "Found rule: ";
        for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)result.second.size(); i++) {
            std::cout << result.second[i] << " ";
        }
        std::cout << std::endl;
    }
    else {
        std::cout << "No rule found" << std::endl;
    }
    // std::cout << "isValidState: " << isValidState(base) << std::endl;
    // std::cout << "isSolvedState: " << isSolvedState(base) << std::endl;

    // base->edges[0]->solution = EDGE_NOT_IN_SOLUTION;
    // base->edges[1]->solution = EDGE_NOT_IN_SOLUTION;
    // base->edges[2]->solution = EDGE_NOT_IN_SOLUTION;
    // base->edges[3]->solution = EDGE_IN_SOLUTION;
    // base->edges[4]->solution = EDGE_NOT_IN_SOLUTION;
    // base->edges[5]->solution = EDGE_NOT_IN_SOLUTION;
    // base->edges[6]->solution = EDGE_NOT_IN_SOLUTION;
    // base->edges[7]->solution = EDGE_NOT_IN_SOLUTION;
    // base->edges[8]->solution = EDGE_NOT_IN_SOLUTION;
    // base->edges[9]->solution = EDGE_IN_SOLUTION;
    // base->edges[10]->solution = EDGE_IN_SOLUTION;
    // base->edges[11]->solution = EDGE_NOT_IN_SOLUTION;

    // std::cout << "Checking base: " << std::endl;
    // for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)base->edges.size(); i++) {
    //     std::cout << base->edges[i]->id << ":" << base->edges[i]->solution << " ";
    // }
    // std::cout << std::endl;

    // std::cout << "isValidState: " << isValidState(base) << std::endl;
    // std::cout << "isSolvedState: " << isSolvedState(base) << std::endl;
    delete base;
}