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

static const size_t OFFSET = 4;
static const size_t SIZE = 1 << OFFSET;
static const size_t MASK = 0xf;
static const size_t ROOT = 0;
static const size_t INVALID_UTF8 = -1;

struct Node {
    uint64_t end = 0, end_utf8 = 0, prefix_sum = 0;
    double log_end = 0, log_prefix_sum = 0;
    size_t ch[SIZE] = {0};
    size_t id = ROOT, parent = ROOT, pre = ROOT, fail = ROOT;
    size_t length = 0;
    uint8_t data = 0;
    
    Node() = default;
    Node(size_t id, size_t parent = ROOT, uint8_t data = 0) : id(id), parent(parent), pre(parent), data(data) {}
};

class Automaton {
private:
    size_t _word_count; // Number of keywords
    size_t _node_count; // Node count
    size_t _cur_state; // Current state
    
    std::vector<Node> t; // Trie nodes
    void get_prefix_sum(); // Calculate word frequencies in sub-Tries for each node
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

    // Insert a valid UTF-8 keyword into the trie with frequency (if not specified, defaults to 1)
    void insert(const std::string& s, uint64_t freq = 1);

    // Returns the number of words in the automaton
    size_t word_count() const;

    // Get node by id
    Node get_node(size_t node_id) const;

    // Get the current state of the automaton
    size_t get_state() const;

    // Get the keyword associated with a node (traverses the path from the root to the node)
    std::string get_keyword(size_t node_id, size_t stop_id = ROOT);

    // Returns border nodes
    std::vector<Node> get_borders(size_t node_id);

    // Returns all child nodes
    std::vector<Node> get_children(size_t node_id);

    // Return all leaf nodes
    std::vector<Node> get_leaves(size_t node_id);

    // Returns keyword matches for a given UTF-8 string
    std::vector<std::string> get_keywords(const std::string& s);

    // Transition function, accepted a UTF-8 string
    Node trans_string(const std::string& s);

    // Transition function, accepts a single byte
    Node trans_byte(uint8_t byte);

    // Reset the automaton to a new state (defaults to root)
    void reset(size_t new_state = ROOT);

    // Build the automaton from the loaded dictionary
    void build();

    // Build the automaton from a dictionary file, with each line containing a word and its frequency
    void build(const std::string& dict_path);

    // Build the automaton from multiple dictionary files
    void build(const std::vector<std::string>& dict_paths);

    // Load a dictionary file into the automaton
    void load_dict(const std::string& dict_path);
};

}

#endif // TRIE_GRAM_AUTOMATON_H

