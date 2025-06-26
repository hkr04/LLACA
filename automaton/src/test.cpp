#include "automaton.h"

int main() {
    automaton::Automaton automaton("../../data/dict/pku_dict.utf8");

    std::string text = "12345dfasdgas武汉市长江大桥";

    auto words = automaton.cut(text);

    for (const auto& word : words) {
        std::cout << word << " / ";
    }

    std::cout << std::endl;
    
    return 0;
}