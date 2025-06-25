#include "automaton.h"

namespace automaton {

// Function to count the number of UTF-8 characters in a string
// Returns INVALID_UTF8 (-1) if the string is not valid UTF-8
static size_t count_utf8_chars(const std::string& s) {
    size_t i = 0;
    size_t count = 0;
    const size_t len = s.size();

    while (i < len) {
        uint8_t byte = static_cast<uint8_t>(s[i]);

        size_t char_len = 0;
        if ((byte & 0x80) == 0x00) {
            // 1-byte ASCII: 0xxxxxxx
            char_len = 1;
        } else if ((byte & 0xE0) == 0xC0) {
            // 2-byte: 110xxxxx 10xxxxxx
            char_len = 2;
        } else if ((byte & 0xF0) == 0xE0) {
            // 3-byte: 1110xxxx 10xxxxxx 10xxxxxx
            char_len = 3;
        } else if ((byte & 0xF8) == 0xF0) {
            // 4-byte: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
            char_len = 4;
        } else {
            return -1; // Invalid first byte
        }

        if (i + char_len > len) {
            return -1; // Not enough bytes
        }

        // Check continuation bytes: must be of form 10xxxxxx
        for (size_t j = 1; j < char_len; ++j) {
            if ((static_cast<uint8_t>(s[i + j]) & 0xC0) != 0x80) {
                return -1; // Invalid continuation byte
            }
        }

        i += char_len;
        ++count;
    }

    return count;
}

void Automaton::insert(const std::string& s, uint64_t freq) {
    auto length = count_utf8_chars(s);

    if (length == INVALID_UTF8) {
        throw std::invalid_argument("Invalid UTF-8 string: " + s);
    }

    size_t u = ROOT;
    size_t char_len = 0;

    for (uint8_t byte : s) {
        uint8_t half_byte = byte >> OFFSET; // Upper 4 bits

        if (!t[u].ch[half_byte] || get_node(t[u].ch[half_byte]).parent != u) {
            t.emplace_back(++_node_count, u, half_byte);
            t[u].ch[half_byte] = _node_count;
        }

        u = t[u].ch[half_byte]; // Move to the next state
        
        half_byte = byte & MASK; // Lower 4 bits

        if (!t[u].ch[half_byte] || get_node(t[u].ch[half_byte]).parent != u) {
            t.emplace_back(++_node_count, u, half_byte);
            t[u].ch[half_byte] = _node_count;
        }

        u = t[u].ch[half_byte]; // Move to the next state
        
        if (char_len == 0) {
            if ((byte & 0x80) == 0x00) {
                // 1-byte ASCII: 0xxxxxxx
                char_len = 1;
            } else if ((byte & 0xE0) == 0xC0) {
                // 2-byte: 110xxxxx 10xxxxxx
                char_len = 2;
            } else if ((byte & 0xF0) == 0xE0) {
                // 3-byte: 1110xxxx 10xxxxxx 10xxxxxx
                char_len = 3;
            } else if ((byte & 0xF8) == 0xF0) {
                // 4-byte: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
                char_len = 4;
            }
        }

        char_len--;

        if (char_len == 0) {
            t[u].end_utf8 += freq; // Increment the UTF-8 end count
        }
    }

    if (t[u].end == 0) { // New keyword
        _word_count++;
    }
        
    t[u].end += freq;
    t[u].length = length;
}

void Automaton::get_prefix_sum() {
    // Initialize prefix sum
    for (size_t i = 0; i <= _node_count; i++) {
        t[i].prefix_sum = t[i].end;
    }

    // Update the Trie from bottom to top
    for (size_t i = _node_count; i >= 0; i--) {
        t[t[i].parent].prefix_sum += t[i].prefix_sum;
        t[i].log_end = log2(t[i].end);
        t[i].log_prefix_sum = log2(t[i].prefix_sum);
        if (i == 0) break; // Avoid underflow
    }
}

void Automaton::get_fail() {
    std::queue<size_t> q;

    for (size_t i = 0; i < SIZE; i++) {
        size_t v = t[ROOT].ch[i];
        if (v && t[v].parent == ROOT) {
            q.push(v);
        } 
    }

    while (!q.empty()) {
        size_t u = q.front();
        q.pop();
        for (size_t i = 0; i < SIZE; i++) {
            size_t v = t[u].ch[i];
            if (v && t[v].parent == u) {
                t[v].fail = t[t[u].fail].ch[i];
                q.push(v);
            } else {
                t[u].ch[i] = t[t[u].fail].ch[i];
            }
        }
    }
}

void Automaton::load_dict(const std::string& dict_path) {
    std::ifstream fin;
    std::string line;
    std::string keyword;
    uint64_t freq;

    fin.open(dict_path, std::ifstream::in);
    if (!fin.is_open()) {
        throw std::runtime_error("Failed to open dictionary file: " + dict_path);
    }
    
    while (std::getline(fin, line)) {
        // Each line contains a keyword, frequency (default 1 if not present), and part of speech (optional), separated by spaces
        std::istringstream iss(line);
        iss >> keyword;
        if (!(iss >> freq)) {
            freq = 1;
        }
        insert(keyword, freq);
    }
}

void Automaton::build() {
    get_prefix_sum();

    for (size_t i = 1; i <= _node_count; i++) {
        size_t pre = t[i].pre;
        while (pre != ROOT && t[pre].end == 0) { // Path compression, point to the last end state
            pre = t[pre].pre;
        }
        t[i].pre = pre;
    }

    get_fail();

    for (size_t i = 1; i <= _node_count; i++) {
        size_t pre = t[i].fail;
        while (pre != ROOT && t[pre].end_utf8 == 0) { // Path compression, point to the last valid utf-8 state
            pre = t[pre].fail;
        }
        t[i].fail = pre;
    }
}

void Automaton::build(const std::string& dict_path) {
    load_dict(dict_path);
    build();
}

void Automaton::build(const std::vector<std::string>& dict_paths) {
    for (const auto& dict_path : dict_paths) {
        load_dict(dict_path);
    }
    build();
}

Automaton::Automaton() {
    _word_count = 0;
    _node_count = 0;
    _cur_state = ROOT;
    t.emplace_back(ROOT);
}

Automaton::Automaton(const std::string& dict_path) {
    _word_count = 0;
    _node_count = 0;
    _cur_state = ROOT;
    t.emplace_back(ROOT);
    build(dict_path);
}

Automaton::Automaton(const std::vector<std::string>& dict_paths) {
    _word_count = 0;
    _node_count = 0;
    _cur_state = ROOT;
    t.emplace_back(ROOT);
    build(dict_paths);
}

size_t Automaton::word_count() const {
    return _word_count;
}

Node Automaton::get_node(size_t node_id) const {
    if (node_id >= t.size()) {
        throw std::out_of_range("Node ID out of range");
    }
    return t[node_id];
}

std::vector<Node> Automaton::get_borders(size_t node_id) {
    std::vector<Node> borders;
    Node unode = get_node(node_id);
    while (unode.id != ROOT) {
        borders.push_back(unode);
        unode = t[unode.fail];
    }
    return borders;
}

std::vector<Node> Automaton::get_children(size_t node_id) {
    if (node_id >= t.size()) {
        throw std::out_of_range("Node ID out of range");
    }
    std::vector<Node> children;
    for (size_t i = 0; i < SIZE; i++) {
        if (t[node_id].ch[i] != ROOT && t[t[node_id].ch[i]].parent == node_id) {
            children.push_back(get_node(t[node_id].ch[i]));
        }
    }
    return children;
}

std::vector<Node> Automaton::get_leaves(size_t node_id) {
    if (node_id >= t.size()) {
        throw std::out_of_range("Node ID out of range");
    }

    std::queue<size_t> q;
    std::vector<Node> leaves;

    q.push(node_id);

    while (!q.empty()) {
        size_t u = q.front();

        q.pop();

        bool is_leaf = true;

        for (size_t i = 0; i < SIZE; i++) {
            if (t[u].ch[i] != ROOT && t[t[u].ch[i]].parent == u) {
                q.push(t[u].ch[i]);
                is_leaf = false;
            }
        }

        if (is_leaf) {
            leaves.push_back(get_node(u));
        }
    }

    return leaves;
}

std::string Automaton::get_keyword(size_t node_id, size_t stop_id) {
    Node unode = get_node(node_id);

    if (unode.id != ROOT && unode.end == 0) { // Find the last recognized state
        unode = t[unode.pre];
    }

    std::string keyword;
    bool is_lower = true;

    while (unode.id != stop_id && unode.id != ROOT) {
        if (is_lower) {
            keyword += static_cast<char>(unode.data);
        } else {
            keyword.back() |= unode.data << OFFSET;
        }
        unode = t[unode.parent]; // Move to the parent node
        is_lower = !is_lower; // Toggle case
    }

    std::reverse(keyword.begin(), keyword.end()); // Reverse the string to get the correct order

    return keyword;
}

std::vector<std::string> Automaton::get_keywords(const std::string& s) {
    size_t pre_state = _cur_state;

    Node u;
    std::vector<Node> borders;
    std::vector<std::string> keywords;

    reset(ROOT); // Reset to root state
    for (uint8_t byte : s) {
        u = trans_byte(byte);
        borders = get_borders(u.id);
        for (const auto& node : borders) {
            if (node.end == 0) {
                continue; // Skip nodes that are not end nodes
            }
            keywords.push_back(get_keyword(node.id));
        }
    }
    reset(pre_state); // Restore previous state

    // Remove duplicates
    std::sort(keywords.begin(), keywords.end());
    auto it = std::unique(keywords.begin(), keywords.end());
    keywords.erase(it, keywords.end());

    return keywords;
}

size_t Automaton::get_state() const {
    return _cur_state;
}

Node Automaton::trans_string(const std::string& s) {
    size_t u = _cur_state;
    for (uint8_t byte : s) {
        uint8_t half_byte = byte >> OFFSET;
        u = t[u].ch[half_byte];
        half_byte = byte & MASK;
        u = t[u].ch[half_byte];
    }
    _cur_state = u;
    return t[u];
}

Node Automaton::trans_byte(uint8_t byte) {
    size_t u = _cur_state;
    uint8_t half_byte = byte >> OFFSET;
    u = t[u].ch[half_byte];
    half_byte = byte & MASK;
    u = t[u].ch[half_byte];
    _cur_state = u;
    return t[u];
}

void Automaton::reset(size_t new_state) {
    _cur_state = new_state;
}

} // namespace automaton
