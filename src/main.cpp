#include "model/model_CPP/api/slitherlink_generic.hpp"


int main(){
    SlitherlinkGeneric puzzle = SlitherlinkGeneric("test.txt");
    puzzle.savePuzzle("test_output.txt");
}