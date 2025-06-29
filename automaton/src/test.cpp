// g++ test.cpp automaton.cpp -std=c++17

#include "automaton.h"

int main() {
    automaton::Automaton automaton("../../data/dict/pku_dict.utf8");

    std::string text = "武汉市长江大桥";

    auto words = automaton.cut(text);

    for (const auto& word : words) {
        std::cout << word << " / ";
    }

    std::cout << std::endl;

    words = automaton.cut(text, true); // cut all

    std::cout << "================ cut all ================" << std::endl;

    for (const auto& word : words) {
        std::cout << word << " / ";
    }

    std::cout << std::endl;

    std::cout << "Memory usage: " << automaton.memory_usage() << " bytes" << std::endl;
    
    return 0;
}