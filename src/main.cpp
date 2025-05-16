#include "model/model_CPP/api/slitherlink.hpp"
#include "generate/api/generate_puzzle.hpp"
#include "solve/api/solver.hpp"
#include "utilities/api/trace_lib.hpp"

int main(){
    // Slitherlink puzzle = Slitherlink("test.txt");
    // puzzle.savePuzzle("test_output.txt");
    Slitherlink* puzzle = generatePuzzleSimple(6);
    // Slitherlink* puzzle = new Slitherlink("generated_1.txt");
    puzzle->savePuzzle("generated_1.txt");

    puzzle->clearSolution();

    Solver* solver = new Solver();
    std::vector<Slitherlink*> slitherlink_solution;
    solver->solvePuzzle(puzzle, slitherlink_solution);
    for (std::ptrdiff_t i = 0; i < (std::ptrdiff_t)slitherlink_solution.size(); ++i) {
        slitherlink_solution[i]->savePuzzle("generated_solution" + std::to_string(i) + ".txt");
        LOG("Puzzle ", i, " solved");
        delete slitherlink_solution[i];
    }

    delete puzzle;
}