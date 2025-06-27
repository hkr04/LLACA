#ifndef TRIE_GRAM_AUTOMATON_H
#define TRIE_GRAM_AUTOMATON_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <queue>
#include <cmath>
#include <algorithm>

namespace automaton {

static const uint8_t OFFSET = 4;
static const uint8_t SIZE = 1 << OFFSET;
static const uint8_t MASK = 0xf;
static const uint8_t INVALID_UTF8 = -1;
static const uint32_t ROOT = 0;
static const uint32_t MAX_FREQ = 1 << 24;
static const uint32_t MAX_UTF8_LEN = 1 << 8;
static const uint32_t INIT_SIZE = 2048;


struct Node {
    uint32_t length : 8;
    uint32_t end : 24;
    uint32_t ch[SIZE];
    uint32_t id, parent, pre, fail;
    float log_end, log_trie_sum;
    uint64_t trie_sum;

    Node(uint32_t id = ROOT, uint32_t parent = ROOT) 
    : length(0), end(0),
      id(id), parent(parent), pre(parent), fail(ROOT),
      log_end(0), log_trie_sum(0),
      trie_sum(0) {
        std::fill(ch, ch + SIZE, 0);
    }
};

class Automaton {
private:
    uint32_t _word_count; // Number of keywords
    uint32_t _node_count; // Node count
    uint32_t _cur_state; // Current state
    
    std::vector<Node> t; // Trie nodes
    void get_trie_sum(); // Calculate word frequencies in sub-Tries for each node
    void get_fail(); // Calculate the fail pointer for each node

public:
    // Constructor
    Automaton();

    // Constructor with dictionary path
    Automaton(const std::string& dict_paths);

    // Constructor with dictionary paths
    Automaton(const std::vector<std::string>& dict_paths);

    // Destructor
    ~Automaton() = default;

    // Get the memory usage of the automaton (bytes)
    size_t memory_usage() const;

    // Insert a valid UTF-8 keyword into the trie with frequency (if not specified, defaults to 1)
    void insert(const std::string& s, uint32_t freq = 1);

    // Returns the number of words in the automaton
    uint32_t word_count() const;

    // Get node by id
    Node get_node(uint32_t node_id) const;

    // Get the current state of the automaton
    uint32_t get_state() const;

    // Returns border nodes
    std::vector<Node> get_borders(uint32_t node_id);

    // Transition function, accepted a UTF-8 string
    Node trans_string(const std::string& s);

    // Transition function, accepts a single byte
    Node trans_byte(uint8_t byte);

    // Reset the automaton to a new state (defaults to root)
    void reset(uint32_t new_state = ROOT);

    // Build the automaton from the loaded dictionary
    void build();

    // Build the automaton from a dictionary file, with each line containing a word and its frequency
    void build(const std::string& dict_path);

    // Build the automaton from multiple dictionary files
    void build(const std::vector<std::string>& dict_paths);

    // Load a dictionary file into the automaton
    void load_dict(const std::string& dict_path);

    // Cut a text into words
    std::vector<std::string> cut(const std::string& text, bool cut_all = false);
};

}

#endif // TRIE_GRAM_AUTOMATON_H

