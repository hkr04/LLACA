# LLACA: Large Language Models-Inspired Aho-Corasick Automaton

![LLACA](/assets/LLACA.png)

## Installation & Build
It is recommended to use pip for local development installation:

```bash
pip install -e .
```

Or manually build the extension module:

```bash
python setup.py build_ext --inplace
```

The generated library file (e.g., `LLACA.*.so`) can be placed in your project directory and imported in Python code.

## Word Segmentation

Example:

```python
import LLACA

ac = LLACA.Automaton("/path/to/dict") # e.g. "./data/dict/pku_dict.utf8"

# Segment text to list of words
LLACA.cut(ac, text="武汉市长江大桥") # ['武汉市', '长江', '大桥']

# Segment text with specified delimeter
LLACA.cut(ac, text="武汉市长江大桥", delim="/") # '武汉市/长江/大桥'

# Segment text of <input_path> to output_path
LLACA.cutf(ac, input_path="/path/to/raw/text", output_path="/path/to/segmented/output")
```

**⚡ 10x Faster**: New C++ interface now available! Replace `cut` with `cut_cpp` for dramatically improved performance. Note: Simplified preprocessing in C++ version results in minor differences from Python implementation.

## 🌟 Get Inspiration from Large Language Models

A more convenient API is on the way!

For now, you can use `LLACA.llm_cut` to get word lists and convert to segmented texts like `data/gold/*_gold.utf8`, then use `LLAMA.build_dict` to convert to dictionaries like `data/dict/*_dict.utf8`.

## Benchmark

If you are interested in the performance based on official dictionaries:

```bash
python utils/benchmark.py
```

Then you can get a result like this:

```
====================== Benchmark Summary ======================
Dataset         F1 Score (%)    Build Time (s)  Cut Time (s)   
as              90.9            0.227           0.836          
cityu           88.6            0.087           0.250          
msr             93.7            0.188           0.683          
pku             90.4            0.084           0.620          
kwdlc           91.0            0.030           0.249          
ud_ja           89.5            0.024           0.081          
best            93.6            0.080           0.407           
```

## Run Demo

Example:

```bash
python demo.py --model qwen-max --base-url "https://dashscope.aliyuncs.com/compatible-mode/v1" --api-key $DASHSCOPE_API_KEY
```

## Run Tests
**Note**: Please install `jieba` for testing!

Example:

```bash
python test.py --model qwen-max --base-url "https://dashscope.aliyuncs.com/compatible-mode/v1" --api-key $DASHSCOPE_API_KEY
```

## Citation

```bib
@misc{zhang2025segment,
      title={Segment First or Comprehend First? Explore the Limit of Unsupervised Word Segmentation with Large Language Models}, 
      author={Zihong Zhang and Liqi He and Zuchao Li and Lefei Zhang and Hai Zhao and Bo Du},
      year={2025},
      eprint={2505.19631},
      archivePrefix={arXiv},
      primaryClass={cs.CL},
      url={https://arxiv.org/abs/2505.19631}, 
}
```