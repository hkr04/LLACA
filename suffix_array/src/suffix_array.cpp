#include "suffix_array.h"

namespace suffix_array {

bool SuffixArray::cmp(uint64_t x, uint64_t y, uint64_t w) {
    return oldrk[x] == oldrk[y] && oldrk[x + w] == oldrk[y + w];
}

// Compare s[pos:] and t, return true if s[pos:] < t, otherwise return false
bool SuffixArray::cmp(uint64_t pos, const std::string& t) { 
    for (uint64_t i = 0; i < t.size(); i++) {
        if (pos + i >= s.size()) {
            return true;
        }
        if (s[pos + i] != t[i]) {
            return (uint8_t)s[pos + i] < (uint8_t)t[i];
        }
    }
    return false;
}

// Compare t and s[pos:], return true if t < s[pos:], otherwise return false
bool SuffixArray::cmp(const std::string& t, uint64_t pos) { 
    for (uint64_t i = 0; i < t.size(); i++) {
        if (pos + i >= s.size()) {
            return false;
        }
        if (t[i] != s[pos + i]) {
            return (uint8_t)t[i] < (uint8_t)s[pos + i];
        }
    }
    return false;
}

uint64_t SuffixArray::utf8_get_len(uint8_t byte) {
    if ((byte & 0x80) == 0x00) { // Single byte, 0xxxxxxx
        return 1;
    } else if ((byte & 0xE0) == 0xC0) { // Two bytes, 110xxxxx
        return 2;
    } else if ((byte & 0xF0) == 0xE0) { // Three bytes, 1110xxxx
        return 3;
    } else if ((byte & 0xF8) == 0xF0) { // Four bytes, 11110xxx
        return 4;
    } else { // byte is not a valid UTF-8 start byte
        return 0;
    }
}

SuffixArray::SuffixArray(const std::string& s) : s(s) {
    uint64_t n = s.size(), m = SIZE;
    std::vector<uint64_t> cnt(std::max(n, m) + 1), key(n + 1), id(n + 1);
    sa.resize(n + 1), oldrk.resize(2 * n + 1), rk.resize(2 * n + 1);

    // Initialize for length 1
    for (uint64_t i = 1; i <= n; i++) {
        cnt[rk[i] = (uint8_t)s[i-1]]++;
    }
    for (uint64_t i = 1; i <= m; i++) {
        cnt[i] += cnt[i-1];
    }
    for (uint64_t i = n; i >= 1; i--) {
        sa[cnt[rk[i]]--] = i;
    }
    
    uint64_t p = 0; // Number of distinct suffixes
    for (uint64_t len = 1; len <= n; len <<= 1, m = p) {
        // Sort suffixes of length = 2 * len
        // The first key: rk[i], the second key: rk[i+len]
        // If i + len > n, they are the smallest, corresponding to the rank interval [1, len] in the first round of sorting
        // If sa[i] > len, its corresponding suffix sa[i]-len should be placed after the second key
        p = 0;
        for (uint64_t i = n; i > n - len; i--) { // The part that is not more than len
            id[++p] = i;
        }
        for (uint64_t i = 1; i <= n; i++) {
            if (sa[i] > len)
                id[++p] = sa[i] - len;
        }

        // Sort the second key
        // Also store the first key of the suffix with rank i after sorting the second key
        fill(cnt.begin(), cnt.end(), 0);
        for (uint64_t i = 1; i <= n; i++) {
            cnt[key[i] = rk[id[i]]]++;
        }
        for (uint64_t i = 1; i <= m; i++) {
            cnt[i] += cnt[i-1];
        }
        for (uint64_t i = n; i >= 1; i--) {
            sa[cnt[key[i]]--] = id[i];
        }

        // Sort the first key
        copy(rk.begin(), rk.end(), oldrk.begin());
        p = 0;
        for (uint64_t i = 1; i <= n; i++) {
            rk[sa[i]] = (cmp(sa[i-1], sa[i], len)? p: ++p);
        }
        if (p == n) { // At most n distinct suffixes
            break;
        }
    }

    // Convert to the rank in all utf-8 start points
    copy(rk.begin(), rk.end(), oldrk.begin());
    std::vector<uint64_t> real_rk;
    id.clear();
    for (uint64_t i = 0, len = 0; i < n; i += len) {
        len = utf8_get_len(s[i]);
        real_rk.push_back(rk[i + 1]);
        id.push_back(i + 1);
    }
    std::sort(real_rk.begin(), real_rk.end());
    rk.resize(id.size());
    sa.resize(id.size() + 1);
    for (uint64_t i = 0; i < id.size(); i++) {
        rk[i] = std::lower_bound(real_rk.begin(), real_rk.end(), oldrk[id[i]])
                - real_rk.begin()
                + 1;
        sa[rk[i]] = id[i] - 1;
    }
}

uint64_t SuffixArray::size() {
    return rk.size();
}

uint64_t SuffixArray::get_id(uint64_t suf_rank) { // 1-index
    return sa[suf_rank];
}

std::string SuffixArray::get_suf(uint64_t suf_rank) { // 1-index
    return s.substr(sa[suf_rank]);
}

uint64_t SuffixArray::get_rank(uint64_t suf_id) { // 0-index
    return rk[suf_id];
}

uint64_t SuffixArray::lower_bound(const std::string& t) {
    uint64_t l = 1, r = size() + 1; // rank
    while (l < r) {
        uint64_t mid = (l + r) >> 1;
        if (mid != size() + 1 && cmp(get_id(mid), t))
            l = mid + 1;
        else
            r = mid;
    }
    return l;
}

uint64_t SuffixArray::upper_bound(const std::string& t) {
    uint64_t l = 1, r = size() + 1; // rank
    while (l < r) {
        uint64_t mid = (l + r) >> 1;
        if (mid == size() + 1 || cmp(t, get_id(mid)))
            r = mid;
        else
            l = mid + 1;
    }
    return l;
}

uint64_t SuffixArray::get_count(const std::string& t) {
    return upper_bound(t) - lower_bound(t);
}

std::vector<std::pair<std::string, double>> SuffixArray::get_prob(const std::string &t) {
    std::vector<std::pair<std::string, double>> prob;
    uint64_t l = lower_bound(t), r = upper_bound(t); // rank [l, r)
    uint64_t p = l;
    if (get_id(p) == s.size() - t.size()) { // Skip the case where the suffix is t
        p++;
    }

    // Use doubling method
    while (p < r) {
        uint64_t L = p, R = r; 
        uint64_t len = utf8_get_len(s[get_id(p) + t.size()]); // Ensure the next token after truncation is a valid utf8 character
        std::string sub = s.substr(get_id(p), t.size() + len);
        while (L < R) { // L is the first suffix greater than the current substring
            uint64_t mid = (L + R) >> 1;
            if (mid == r || cmp(sub, get_id(mid))) // The current substring < the enumerated suffix
                R = mid;
            else // The current substring >= the enumerated suffix, not enough
                L = mid + 1;
        }
        // sub may be an empty string
        prob.emplace_back(sub.substr(t.size()), 1.0 * (L - p) / (r - l));
        p = L;
    }

    if (prob.empty()) {
        prob.emplace_back("[UNK]", 0);
    }
        
    return prob;
}
double SuffixArray::get_branch_entropy(const std::string &t) {
    double be = 0;
    auto prob_list = get_prob(t);
    for (const auto& p : prob_list) {
        double prob = p.second;
        be += -prob * log(prob + 1e-20);
    }
    return be;
}
double SuffixArray::get_mutual_information(const std::string &t) {
    if (t.size() <= 1) {
        return 0;
    }
    uint64_t count = get_count(t);
    if (count == 0) {
        return 0;
    }
    uint64_t total = size(); // The total number of characters
    double log_count = log(count), log_total = log(total);
    double pmi = 1e60;
    for (uint64_t i = utf8_get_len(t[0]); i < t.size(); i += utf8_get_len(t[i])) {
        pmi = std::min(pmi, log_total + log_count - log(get_count(t.substr(0, i))) - log(get_count(t.substr(i))));
    }
    return pmi;
}

} // namespace suffix_array
