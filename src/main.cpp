#include "model/model_CPP/api/slitherlink.hpp"
#include "generate/api/generate_puzzle.hpp"

int main(){
    // Slitherlink puzzle = Slitherlink("test.txt");
    // puzzle.savePuzzle("test_output.txt");
    Slitherlink* puzzle = new Slitherlink(5);
    puzzle->savePuzzle("generated_1.txt");
    delete puzzle;
}