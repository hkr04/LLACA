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
            return INVALID_UTF8;
        }

        i += char_len;
        ++count;
    }

    return count;
}

size_t Automaton::memory_usage() const {
    size_t total = 0;
    total += sizeof(*this);
    total += t.capacity() * sizeof(Node);
    return total;
}

void Automaton::insert(const std::string& s, uint32_t freq) {
    auto length = count_utf8_chars(s);

    if (length == INVALID_UTF8) {
        throw std::invalid_argument("Invalid UTF-8 string: " + s);
    }

    auto u = ROOT;

    for (uint8_t byte : s) {
        uint8_t half_byte = byte >> OFFSET; // Upper 4 bits

        if (!t[u].ch[half_byte] || t[t[u].ch[half_byte]].parent != u) {
            t.emplace_back(++_node_count, u);
            t[u].ch[half_byte] = _node_count;
        }

        u = t[u].ch[half_byte]; // Move to the next state
        
        half_byte = byte & MASK; // Lower 4 bits

        if (!t[u].ch[half_byte] || t[t[u].ch[half_byte]].parent != u) {
            t.emplace_back(++_node_count, u);
            t[u].ch[half_byte] = _node_count;
        }

        u = t[u].ch[half_byte]; // Move to the next state
    }

    if (t[u].end == 0) { // New keyword
        _word_count++;
    }

    if (1ll * t[u].end + freq > MAX_FREQ) {
        throw std::overflow_error("Frequency overflow");
    }

    if (length > MAX_UTF8_LEN) {
        throw std::overflow_error("UTF-8 length overflow");
    }

    t[u].end += freq;
    t[u].length = length;
}

void Automaton::get_trie_sum() {
    // Initialize prefix sum
    for (auto& node : t) {
        node.trie_sum = node.end;
    }

    // Update the Trie from bottom to top
    for (auto i = _node_count; i >= 0; i--) {
        t[t[i].parent].trie_sum += t[i].trie_sum;
        t[i].log_end = log2(t[i].end);
        t[i].log_trie_sum = log2(t[i].trie_sum);
        if (i == 0) break; // Avoid underflow
    }
}

void Automaton::get_fail() {
    std::queue<uint32_t> q;

    for (uint32_t i = 0; i < SIZE; i++) {
        auto v = t[ROOT].ch[i];
        if (v && t[v].parent == ROOT) {
            q.push(v);
        } 
    }

    while (!q.empty()) {
        auto u = q.front();
        q.pop();
        for (uint32_t i = 0; i < SIZE; i++) {
            auto v = t[u].ch[i];
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
    uint32_t freq;

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
    // Shrink the vector to fit the actual size
    t.shrink_to_fit();

    get_trie_sum();

    for (uint32_t i = 1; i <= _node_count; i++) {
        auto pre = t[i].pre;
        while (pre != ROOT && t[pre].end == 0) { // Path compression, point to the last end state
            pre = t[pre].pre;
        }
        t[i].pre = pre;
    }

    get_fail();

    for (uint32_t i = 1; i <= _node_count; i++) {
        auto pre = t[i].fail;
        while (pre != ROOT && t[pre].end == 0) { // Path compression, point to the last end state
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
    t.reserve(INIT_SIZE);
    t.emplace_back(ROOT);
}

Automaton::Automaton(const std::string& dict_path) {
    _word_count = 0;
    _node_count = 0;
    _cur_state = ROOT;
    t.reserve(INIT_SIZE);
    t.emplace_back(ROOT);
    build(dict_path);
}

Automaton::Automaton(const std::vector<std::string>& dict_paths) {
    _word_count = 0;
    _node_count = 0;
    _cur_state = ROOT;
    t.reserve(INIT_SIZE);
    t.emplace_back(ROOT);
    build(dict_paths);
}

uint32_t Automaton::word_count() const {
    return _word_count;
}

Node Automaton::get_node(uint32_t node_id) const {
    if (node_id >= t.size()) {
        throw std::out_of_range("Node ID out of range");
    }
    return t[node_id];
}

std::vector<Node> Automaton::get_borders(uint32_t node_id) {
    std::vector<Node> borders;
    Node unode = get_node(node_id);
    while (unode.id != ROOT) {
        borders.push_back(unode);
        unode = t[unode.fail];
    }
    return borders;
}

uint32_t Automaton::get_state() const {
    return _cur_state;
}

Node Automaton::trans_string(const std::string& s) {
    auto u = _cur_state;
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
    auto u = _cur_state;
    uint8_t half_byte = byte >> OFFSET;
    u = t[u].ch[half_byte];
    half_byte = byte & MASK;
    u = t[u].ch[half_byte];
    _cur_state = u;
    return t[u];
}

void Automaton::reset(uint32_t new_state) {
    _cur_state = new_state;
}

std::vector<std::string> Automaton::cut(const std::string& text, bool cut_all) {
    if (text.empty()) {
        return {};
    }

    auto n = text.size();
    auto min_prob = -get_node(0).log_trie_sum;

    std::vector<float> max_prob; // char
    std::vector<int> utf8_start; // byte
    std::vector<int> pre; // char
    std::vector<std::string> words;

    int i = 0, j = 0; // byte, char

    auto pre_state = _cur_state;

    _cur_state = ROOT;
    
    // TODO: Handle full-width numbers and alphabets
    int num_start = -1, alpha_start = -1; // char, char

    auto collect_word = [&words, &text](int utf8_start, int utf8_len) {
        words.push_back(text.substr(utf8_start, utf8_len));
    };
    
    while (i < n) {
        uint8_t byte = static_cast<uint8_t>(text[i]);
        uint8_t char_len = 0;
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
            _cur_state = pre_state; // Restore previous state
            throw std::invalid_argument("Invalid UTF-8 string: " + text);
        }

        if (i + char_len > n) {
            _cur_state = pre_state; // Restore previous state
            throw std::invalid_argument("Invalid UTF-8 string: " + text);
        }
        
        for (uint8_t k = 0; k < char_len; k++) {
            trans_byte(static_cast<uint8_t>(text[i + k]));
        }

        max_prob.push_back(min_prob);
        pre.push_back(j - 1);
        utf8_start.push_back(i);

        // Handle numbers
        if (char_len == 1 && isdigit(text[i])) {
            if (num_start == -1) { // First number
                num_start = j;
            } else { // Not the first number
                // [0, num_start) [num_start, j) vs [0, j - 1) [j - 1, j)
                // NOTE: min_prob is negative
                if (max_prob[num_start - 1] + min_prob / 2 > max_prob.back() + min_prob) {
                    max_prob.back() = max_prob[num_start - 1] + min_prob / 2;
                    pre.back() = num_start - 1;
                }
            }
        } else { // Clear num_start
            num_start = -1;
        }

        // Handle alphabets
        if (char_len == 1 && isalpha(text[i])) {
            if (alpha_start == -1) { // First alpha
                alpha_start = j;
            } else { // Not the first alpha
                // [0, alpha_start) [alpha_start, j) vs [0, j - 1) [j - 1, j)
                // NOTE: min_prob is negative
                if (max_prob[alpha_start - 1] + min_prob / 2 > max_prob.back() + min_prob) {
                    max_prob.back() = max_prob[alpha_start - 1] + min_prob / 2;
                    pre.back() = alpha_start - 1;
                }
            }
        } else { // Clear alpha_start
            alpha_start = -1;
        }

        if (cut_all) {
            collect_word(utf8_start[pre.back() + 1], utf8_start.back() + char_len - utf8_start[pre.back() + 1]);
        }

        auto borders = get_borders(_cur_state);

        for (const auto& border : borders) {
            if (border.end == 0) {
                continue;
            }
            auto pre_node = get_node(border.pre);
            auto len_border = border.length;
            auto log_cnt_pre = pre_node.log_trie_sum;
            auto log_cnt_border = border.log_end;
            auto prob = log_cnt_border - log_cnt_pre;
            if (cut_all && len_border != 1) {
                collect_word(utf8_start[j - len_border + 1], utf8_start.back() + char_len - utf8_start[j - len_border + 1]);
            }
            if (prob > max_prob.back()) {
                max_prob.back() = prob;
                pre.back() = j - len_border;
            }
        }

        i += char_len, j++;
    }

    _cur_state = pre_state; // Restore previous state

    if (cut_all) {
        return words;
    }

    utf8_start.push_back(n);

    // Trace back to get the words
    j--;
    while (j >= 0) {
        collect_word(utf8_start[pre[j] + 1], utf8_start[j + 1] - utf8_start[pre[j] + 1]);
        j = pre[j];
    }

    std::reverse(words.begin(), words.end());

    return words;
}

} // namespace automaton
