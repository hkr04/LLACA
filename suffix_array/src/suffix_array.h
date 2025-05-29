#ifndef SUFFIX_ARRAY_H
#define SUFFIX_ARRAY_H

#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <iostream>
#include <cmath>

namespace suffix_array {

static const uint64_t SIZE = (1 << 8) - 1; // One byte

class SuffixArray {
private:
    // sa[i]: The i-th suffix rank
    // id[i]: Similar to sa[i]
    // rk[i]: The rank of the i-th suffix
    std::string s;
    std::vector<uint64_t> sa, oldrk, rk;
    bool cmp(uint64_t x, uint64_t y, uint64_t w);
    bool cmp(uint64_t pos, const std::string &t);
    bool cmp(const std::string &t, uint64_t pos);
    uint64_t lower_bound(const std::string &t);
    uint64_t upper_bound(const std::string &t);
    static uint64_t utf8_get_len(uint8_t byte);
public:
    SuffixArray() = default;
    SuffixArray(const std::string& s); 
    uint64_t size();
    uint64_t get_id(uint64_t suf_rank); // 1-index
    std::string get_suf(uint64_t suf_rank); // 1-index
    uint64_t get_rank(uint64_t suf_id); // 0-index
    uint64_t get_count(const std::string& t);
    std::vector<std::pair<std::string, double>> get_prob(const std::string& t);
    double get_branch_entropy(const std::string& t);
    double get_mutual_information(const std::string& t);
};

} // namespace suffix_array

#endif // SUFFIX_ARRAY_H
