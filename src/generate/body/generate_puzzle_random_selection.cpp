#include "../api/generate_puzzle.hpp"

#include <algorithm>
#include <random>
#include <iostream>

bool checkFacesConnectivity(std::ptrdiff_t no_of_faces, std::vector<slitherlink_face*> faces) {
    std::vector<std::ptrdiff_t> face_to_set(no_of_faces, -1);
    std::ptrdiff_t current_set_id = 0;
    for (slitherlink_face* face : faces) {
        std::vector<std::ptrdiff_t> adjacent_face_to_set(face->no_of_edges, -1);
        for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)face->no_of_edges; ++i) {
            if (face->face_ids[i] != -1) {
                adjacent_face_to_set[i] = face_to_set[face->face_ids[i]];
            }
        }
        bool is_connected = false;
        for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)face->no_of_edges; ++i) {
            if (adjacent_face_to_set[i] != -1) {
                is_connected = true;
                if (face_to_set[face->id] == -1) {
                    face_to_set[face->id] = adjacent_face_to_set[i];
                }
                else if (face_to_set[face->id] != adjacent_face_to_set[i]) {
                    for (std::ptrdiff_t j = 0; j < (std::ptrdiff_t)face_to_set.size(); ++j) {
                        if (face_to_set[j] == adjacent_face_to_set[i]) {
                            face_to_set[j] = face_to_set[face->id];
                        }
                    }
                }
            }
        }
        if (!is_connected && face->id != OUTER_FACE) {
            face_to_set[face->id] = current_set_id;
            current_set_id++;
        }
    }
    std::ptrdiff_t final_set_id = -1;
    for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)face_to_set.size(); ++i) {
        if (face_to_set[i] != -1) {
            if (final_set_id == -1) {
                final_set_id = face_to_set[i];
            }
            else if (final_set_id != face_to_set[i]) {
                return false; // More than one connected component found
            }
        }
    }
    return true;
}

void fillUnchosenHoles(std::vector<slitherlink_face*>* chosen_faces,
                       std::vector<slitherlink_face*>* unchosen_faces) {

    std::vector<std::ptrdiff_t> face_to_set(unchosen_faces->size() + chosen_faces->size(), -1);
    std::ptrdiff_t current_set_id = 0;

    std::vector<std::ptrdiff_t> set_ids;

    for (slitherlink_face* face : *unchosen_faces) {
        if (face->id == OUTER_FACE) {
            continue; // Skip the outer face
        }
        std::vector<std::ptrdiff_t> adjacent_face_to_set(face->no_of_edges, -1);
        for (std::ptrdiff_t i = 0; i < face->no_of_edges; ++i) {
            if (face->face_ids[i] != -1) {
                adjacent_face_to_set[i] = face_to_set[face->face_ids[i]];
            }
        }
        bool is_connected = false;
        for (std::ptrdiff_t i = 0; i < face->no_of_edges; ++i) {
            if (adjacent_face_to_set[i] != -1) {
                is_connected = true;
                if (face_to_set[face->id] == -1) {
                    face_to_set[face->id] = adjacent_face_to_set[i];
                }
                else if (face_to_set[face->id] != adjacent_face_to_set[i]) {
                    set_ids.erase(std::remove(set_ids.begin(), set_ids.end(), adjacent_face_to_set[i]), set_ids.end());
                    for (std::ptrdiff_t j = 0; j < (std::ptrdiff_t)face_to_set.size(); ++j) {
                        if (face_to_set[j] == adjacent_face_to_set[i]) {
                            face_to_set[j] = face_to_set[face->id];
                        }
                    }
                }
            }
        }
        if (!is_connected) {
            face_to_set[face->id] = current_set_id;
            set_ids.push_back(current_set_id);
            current_set_id++;
        }
    }

    // for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)face_to_set.size(); ++i) {
    //     if (face_to_set[i] != -1 &&
    //         std::find(set_ids.begin(), set_ids.end(), face_to_set[i]) != set_ids.end()) {
    //         for (std::ptrdiff_t j = 0; j < (std::ptrdiff_t)unchosen_faces->at(i)->no_of_edges; ++j) {
    //             if (unchosen_faces->at(i)->face_ids[j] == -1) {
    //                 set_ids.erase(std::remove(set_ids.begin(), set_ids.end(), face_to_set[i]), set_ids.end());
    //                 break;
    //             }
    //         }
    //     }
    // }

    for (slitherlink_face* face : *unchosen_faces) {
        if (face->id != OUTER_FACE &&
            face_to_set[face->id] != -1 &&
            std::find(set_ids.begin(), set_ids.end(), face_to_set[face->id]) != set_ids.end()) {
            for (std::ptrdiff_t i = 0; i < face->no_of_edges; ++i) {
                if (face->face_ids[i] == OUTER_FACE) {
                    set_ids.erase(std::remove(set_ids.begin(), set_ids.end(), face_to_set[face->id]), set_ids.end());
                    break;
                }
            }
        }
    }

    // for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)face_to_set.size(); ++i) {
    //     if (face_to_set[i] != -1 &&
    //         std::find(set_ids.begin(), set_ids.end(), face_to_set[i]) != set_ids.end()) {
    //         slitherlink_face* face = nullptr;
    //         for (std::ptrdiff_t j = 0; j < (std::ptrdiff_t)unchosen_faces->size(); ++j) {
    //             if (unchosen_faces->at(j)->id == i) {
    //                 face = unchosen_faces->at(j);
    //                 break;
    //             }
    //         }
    //         if (face != nullptr) {
    //             chosen_faces->push_back(face);
    //             unchosen_faces->erase(std::remove(unchosen_faces->begin(), unchosen_faces->end(), face), unchosen_faces->end());
    //         }
    //     }
    // }
    for (slitherlink_face* face : *unchosen_faces) {
        if (face->id != OUTER_FACE &&
            face_to_set[face->id] != -1 &&
            std::find(set_ids.begin(), set_ids.end(), face_to_set[face->id]) != set_ids.end()) {
            chosen_faces->push_back(face);
        }
    }
}

std::vector<std::ptrdiff_t> convertChosenFacesToFaceIndicators(std::ptrdiff_t no_of_faces,
                                                               std::vector<slitherlink_face*> chosen_faces) {
    std::vector<std::ptrdiff_t> face_indicators(no_of_faces, FACE_REJECTED);
    for (slitherlink_face* face : chosen_faces) {
        if (face->id == OUTER_FACE) {
            continue; // Skip the outer face
        }
        face_indicators[face->id] = FACE_ACCEPTED;
    }
    return face_indicators;
}

std::vector<std::ptrdiff_t> convertChosenFacesToEdgeIndicators(std::ptrdiff_t no_of_edges,
                                                               std::vector<slitherlink_face*> chosen_faces) {
    std::vector<std::ptrdiff_t> edge_indicators(no_of_edges, EDGE_REJECTED);
    for (slitherlink_face* face : chosen_faces) {
        if (face->id == OUTER_FACE) {
            continue; // Skip the outer face
        }
        for (std::ptrdiff_t edge_id : face->edge_ids) {
            if (edge_indicators[edge_id] == EDGE_REJECTED) {
                edge_indicators[edge_id] = EDGE_ACCEPTED;
            }
            else {
                edge_indicators[edge_id] = EDGE_REJECTED;
            }
        }
    }
    return edge_indicators;
}

std::vector<std::ptrdiff_t> convertFaceIndicatorsToEdgeIndicators(Slitherlink* slitherlink, std::ptrdiff_t no_of_edges, std::vector<std::ptrdiff_t> face_indicators){
    std::vector<std::ptrdiff_t> edge_indicators(no_of_edges, EDGE_REJECTED);
    for (std::ptrdiff_t i = 0; i < slitherlink->no_of_faces; ++i) {
        if (face_indicators[i] == FACE_ACCEPTED) {
            slitherlink_face* face = slitherlink->faces[i];
            for (std::ptrdiff_t edge_id : face->edge_ids) {
                if (edge_indicators[edge_id] == EDGE_REJECTED) {
                    edge_indicators[edge_id] = EDGE_ACCEPTED;
                }
                else {
                    edge_indicators[edge_id] = EDGE_REJECTED;
                }
            }
        }
    }
    return edge_indicators;
}

std::vector<slitherlink_face*> convertFaceIndicatorsToChosenFaces(Slitherlink* slitherlink, std::vector<std::ptrdiff_t> face_indicators) {
    std::vector<slitherlink_face*> chosen_faces;
    for (std::ptrdiff_t i = 0; i < slitherlink->no_of_faces; ++i) {
        if (face_indicators[i] == FACE_ACCEPTED) {
            chosen_faces.push_back(slitherlink->faces[i]);
        }
    }
    return chosen_faces;
}

Slitherlink* generatePuzzleRandomSelection(std::ptrdiff_t size) {
    Slitherlink* slitherlink = new Slitherlink(size);
    
    std::vector<slitherlink_face*> chosen_faces;
    std::vector<slitherlink_face*> unchosen_faces;

    std::mt19937 gen(std::random_device{}());

    std::sample(slitherlink->faces.begin(),
                slitherlink->faces.end(),
                std::back_inserter(chosen_faces),
                (int)slitherlink->no_of_faces / 2,
                gen);

    std::ptrdiff_t chosen_face_iterator = 0;
    for (std::ptrdiff_t i = 0; i < slitherlink->no_of_faces; ++i) {
        if (chosen_face_iterator >= (std::ptrdiff_t)chosen_faces.size()) {
            unchosen_faces.push_back(slitherlink->faces[i]);
            continue;
        }
        else if (slitherlink->faces[i]->id == chosen_faces[chosen_face_iterator]->id) {
            chosen_face_iterator++;
        }
        else {
            unchosen_faces.push_back(slitherlink->faces[i]);
        }
    }

    bool is_chosen_faces_connected = checkFacesConnectivity(slitherlink->no_of_faces, chosen_faces);
    if (!is_chosen_faces_connected && checkFacesConnectivity(slitherlink->no_of_faces, unchosen_faces)) {
        std::swap(chosen_faces, unchosen_faces);
        is_chosen_faces_connected = true;
    }


    while (!is_chosen_faces_connected) {   
        std::uniform_int_distribution<> distrib(0, (int)unchosen_faces.size() - 1); 
        slitherlink_face* face_to_add = unchosen_faces[distrib(gen)];
        chosen_faces.push_back(face_to_add);


        unchosen_faces.erase(std::remove(unchosen_faces.begin(), unchosen_faces.end(), face_to_add), unchosen_faces.end());


        is_chosen_faces_connected = checkFacesConnectivity(slitherlink->no_of_faces, chosen_faces);
    }

    fillUnchosenHoles(&chosen_faces, &unchosen_faces);

    addFaceValues(slitherlink, convertChosenFacesToEdgeIndicators(slitherlink->no_of_edges, chosen_faces));
    
    addEdgeValues(slitherlink, convertChosenFacesToEdgeIndicators(slitherlink->no_of_edges, chosen_faces));

    return slitherlink;
}
