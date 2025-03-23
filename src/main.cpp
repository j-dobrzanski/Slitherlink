#include "model/model_CPP/api/slitherlink.hpp"

int main(){
    Slitherlink puzzle = Slitherlink("test.txt");
    puzzle.savePuzzle("test_output.txt");
}