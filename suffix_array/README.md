[中文](README_zh.md) | English

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT) &ensp;

# SuffixArray Library

This project implements a suffix array based on the doubling algorithm, supporting efficient string processing and suitable for UTF-8 text. The core part is written in C++ and wrapped as a Python extension module via Cython.

## Features
- **Time Complexity**: $\mathcal{O}(n\log n)$
- **Space Complexity**: $\mathcal{O}(n)$ (with a large constant, be cautious with large datasets)
- **Multilingual Support**: Input must be UTF-8 encoded

## Implemented Functions
- `size()`: Returns the length of the text
- `get_id(suf_rank)`: Given a suffix rank (1-indexed), returns its position in the text (0-indexed)
- `get_suf(suf_rank)`: Given a suffix rank (1-indexed), returns the corresponding suffix
- `get_rank(suf_id)`: Given the left boundary position of a suffix (0-indexed), returns its lexicographical rank
- `get_count(pattern)`: Given a pattern string, returns the number of its occurrences in the text
- `get_prob(prompt)`: Given a text prompt, returns the probability distribution of the next character
- `get_branch_entropy(prompt)`: Given a text prompt, returns its right branch entropy
- `get_mutual_information(text)`: Given a text, returns its mutual information

## Installation & Build
It is recommended to use pip for local development installation:

```bash
pip install -e .
```

Or manually build the extension module:

```bash
python setup.py build_ext --inplace
```

The generated library file (e.g., `SuffixArray.*.so`) can be placed in your project directory and imported in Python code.

## Testing

`test.ipynb` provides simple test cases. If you encounter unexpected outputs, feel free to open an issue.

## Notes
- Currently, a linear-time construction algorithm is not used
- The space constant is large, not suitable for ultra-large datasets
- Test cases are limited, mainly for personal study and experimentation

## License
This project is licensed under the MIT License. Free to use and modify. 