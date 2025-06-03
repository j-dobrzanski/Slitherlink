#include "model/model_CPP/api/slitherlink.hpp"
#include "generate/api/generate_puzzle.hpp"
#include "solve/api/solver.hpp"
#include "utilities/api/trace_lib.hpp"

void generateAllPuzlesOfSize(std::ptrdiff_t size) {
    Slitherlink* slitherlink = generateFirstPuzzleInOrder(size);
    std::vector<std::ptrdiff_t> face_indicators(slitherlink->no_of_faces, FACE_REJECTED);
    delete slitherlink;
    slitherlink = generateNextPuzzleInOrder(&face_indicators, size);
    std::ptrdiff_t id = 0;
    do {
        slitherlink->savePuzzle("gen/puzzles/size" + std::to_string(size) + "/generated_" + std::to_string(id) + ".txt");
        delete slitherlink;
        slitherlink = generateNextPuzzleInOrder(&face_indicators, size);
        id++;
    } while (slitherlink != nullptr);
}

int main(){
    // Slitherlink puzzle = Slitherlink("test.txt");
    // puzzle.savePuzzle("test_output.txt");
    // Slitherlink* puzzle = generatePuzzleSimple(5);
    // Slitherlink* puzzle = new Slitherlink("generated_1.txt");
    // puzzle->savePuzzle("generated_1.txt");

    // puzzle->clearSolution();

    Solver* solver = new Solver();
    // std::vector<Slitherlink*> slitherlink_solution;
    // solver->solvePuzzle(puzzle, &slitherlink_solution);
    // for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)slitherlink_solution.size(); ++i) {
    //     slitherlink_solution[i]->savePuzzle("generated_solution" + std::to_string(i) + ".txt");
    //     LOG_DEBUG("Puzzle ", i, " solved");
    //     delete slitherlink_solution[i];
    // }

    // delete solver;
    // delete puzzle;

    // solver->generateAndSaveRules("generated_rules_size_2.txt", 2);
    // solver->test();
    // solver->generateAndSaveRules("gen/raw_rules/generated_rules.txt", 2);
    // solver->refineAndSaveRules("gen/raw_rules/generated_rules.txt", "gen/refined_rules/generated_rules.txt", 2);
    
    solver->loadRules("gen/refined_rules/generated_rules.txt", 2);
    // Slitherlink* puzzle = new Slitherlink("generated_random.txt");
    Slitherlink* puzzle = generatePuzzleRandomSelection(8);
    puzzle->savePuzzle("generated_random.txt");
    std::vector<Slitherlink*> slitherlink_solution;
    statistics slitherlink_stats;
    solver->solvePuzzle(puzzle, true, true, &slitherlink_solution, &slitherlink_stats);
    slitherlink_stats.print();
    delete solver;

    if (slitherlink_solution.size() > 1) {
        for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)slitherlink_solution.size(); ++i) {
            slitherlink_solution[i]->savePuzzle("solution_" + std::to_string(i) + ".txt");
        }
    }

    // generateAllPuzlesOfSize(3);

    // Slitherlink* puzzle = generatePuzzleRandomSelection(4);
    // puzzle->savePuzzle("generated_random.txt");
    for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)slitherlink_solution.size(); ++i) {
        delete slitherlink_solution[i];
    }
    delete puzzle;
    return 0;
}