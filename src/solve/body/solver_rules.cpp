#include "../api/solver.hpp"
#include "../../utilities/api/trace_lib.hpp"

#include <array>
#include <fstream>

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
        ERROR("Base and rule state have different number of edges");
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

std::pair<bool, std::vector<slitherlink_edge_type>> searchForRules(rule_state* base) {
    rule_state* derived = base->copy();

    // We check only states that are fully decided:
    for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)derived->edges.size(); i++) {
        if (derived->edges[i]->solution == EDGE_UNKNOWN) {
            derived->edges[i]->solution = EDGE_IN_SOLUTION;
        }
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
    return result;
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
        file << "# Rule: " << std::endl;
        // file << "# Face values: " << std::endl;
        for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)base->faces.size(); i++) {
            file << rule->face_values[i] << " ";
        }
        file << std::endl;
        // file << "# Base edge states: " << std::endl;
        for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)base->edges.size(); i++) {
            file << rule->base_edge_states[i] << " ";
        }
        file << std::endl;
        if (rule->is_positive) {
            // file << "# Positive rule: " << std::endl;
            file << rule->is_positive << std::endl;
            for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)base->edges.size(); i++) {
                file << rule->derived_edge_states[i] << " ";
            }
            file << std::endl;
        }
        else {
            // file << "# Negative rule: " << std::endl;
            file << rule->is_positive << std::endl;
        }
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