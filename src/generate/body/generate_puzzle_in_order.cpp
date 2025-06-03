#include "../api/generate_puzzle.hpp"

#include <iostream>

bool checkIfLastState(std::vector<std::ptrdiff_t>* face_indicators) {
    for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)face_indicators->size(); ++i) {
        if ((*face_indicators)[i] == FACE_ACCEPTED) {
            return false;
        }
    }
    return true;
}

bool checkIfTouchesOuter(Slitherlink* slitherlink,
                         std::vector<std::ptrdiff_t> face_indicators) {
    for (std::ptrdiff_t i = 0; i < slitherlink->no_of_faces; ++i) {
        slitherlink_face* face = slitherlink->faces[i];
        if (face_indicators[i] == FACE_REJECTED ||
            face->id == OUTER_FACE) {
            continue;
        }
        for (std::ptrdiff_t i = 0; i < face->no_of_edges; ++i) {
            if (face->face_ids[i] == OUTER_FACE) {
                return true;
            }
        }
    }
    return false;
}

void getNextFaceIndicators(std::vector<std::ptrdiff_t>* face_indicators) {
    for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)face_indicators->size(); ++i) {
        if ((*face_indicators)[i] == FACE_ACCEPTED) {
            (*face_indicators)[i] = FACE_REJECTED;
        }
        else if ((*face_indicators)[i] == FACE_REJECTED) {
            (*face_indicators)[i] = FACE_ACCEPTED;
            return;
        }
    }
}

Slitherlink* generateFirstPuzzleInOrder(std::ptrdiff_t size) {
    Slitherlink* slitherlink = new Slitherlink(size);
    
    addFaceValues(slitherlink, std::vector<std::ptrdiff_t>(slitherlink->no_of_edges, FACE_REJECTED));
    addEdgeValues(slitherlink, std::vector<std::ptrdiff_t>(slitherlink->no_of_edges, EDGE_REJECTED));

    return slitherlink;
}


Slitherlink* generateNextPuzzleInOrder(std::vector<std::ptrdiff_t>* face_indicators,
                                       std::ptrdiff_t size) {
    Slitherlink* slitherlink = new Slitherlink(size);
    getNextFaceIndicators(face_indicators);

    if (checkIfLastState(face_indicators)) {
        delete slitherlink;
        return nullptr;
    }
    
    while (!checkFacesConnectivity(slitherlink->no_of_faces, convertFaceIndicatorsToChosenFaces(slitherlink, *face_indicators)) ||
           !checkIfTouchesOuter(slitherlink, *face_indicators)) {
        getNextFaceIndicators(face_indicators);
        if (checkIfLastState(face_indicators)) {
            delete slitherlink;
            return nullptr;
        }
    }
    addFaceValues(slitherlink, convertFaceIndicatorsToEdgeIndicators(slitherlink, slitherlink->no_of_edges, *face_indicators));
    addEdgeValues(slitherlink, convertFaceIndicatorsToEdgeIndicators(slitherlink, slitherlink->no_of_edges, *face_indicators));
    return slitherlink;
}

