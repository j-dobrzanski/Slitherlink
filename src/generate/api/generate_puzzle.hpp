#include "../../model/model_CPP/api/slitherlink.hpp"

#ifndef GENERATE_PUZZLE_H
#define GENERATE_PUZZLE_H

#define FACE_UNPROCESSED 0
#define FACE_ACCEPTED 1
#define FACE_REJECTED 2

#define EDGE_UNPROCESSED 0
#define EDGE_ACCEPTED 1
#define EDGE_REJECTED 2

void addFaceValues(Slitherlink* slitherlink, std::vector<std::ptrdiff_t> face_indicators);

void addEdgeValues(Slitherlink* slitherlink, std::vector<std::ptrdiff_t> edge_indicators);

std::vector<std::ptrdiff_t> convertChosenFacesToFaceIndicators(std::ptrdiff_t no_of_faces, std::vector<slitherlink_face*> chosen_faces);

std::vector<std::ptrdiff_t> convertChosenFacesToEdgeIndicators(std::ptrdiff_t no_of_edges, std::vector<slitherlink_face*> chosen_faces);

std::vector<std::ptrdiff_t> convertFaceIndicatorsToEdgeIndicators(Slitherlink* slitherlink, std::ptrdiff_t no_of_edges, std::vector<std::ptrdiff_t> face_indicators);

std::vector<slitherlink_face*> convertFaceIndicatorsToChosenFaces(Slitherlink* slitherlink, std::vector<std::ptrdiff_t> face_indicators);

bool checkFacesConnectivity(std::ptrdiff_t no_of_faces, std::vector<slitherlink_face*> faces);

/**
 * Generate simple puzzle of Hexagonal type.
 * 
 */
Slitherlink* generatePuzzleSimple(std::ptrdiff_t size);

Slitherlink* generatePuzzleRandomSelection(std::ptrdiff_t size);

Slitherlink* generateFirstPuzzleInOrder(std::ptrdiff_t size);

Slitherlink* generateNextPuzzleInOrder(std::vector<std::ptrdiff_t>* face_indicators,
                                       std::ptrdiff_t size);
                                    
#endif // GENERATE_PUZZLE_H
