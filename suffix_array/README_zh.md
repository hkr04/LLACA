[English](README.md) | 中文

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT) &ensp;

# SuffixArray 后缀数组库

本项目实现了基于倍增算法的后缀数组，支持高效的字符串处理，适用于 UTF-8 文本。核心部分采用 C++ 编写，并通过 Cython 封装为 Python 扩展模块。

## 特性
- **时间复杂度**：$\mathcal{O}(n\log n)$
- **空间复杂度**：$\mathcal{O}(n)$（常数较大，处理大数据集时需注意）
- **多语言支持**：输入需为 UTF-8 编码

## 已实现功能
- `size()`：返回文本串长度
- `get_id(suf_rank)`：输入后缀排名（1-index），返回其在文本串中的位置（0-index）
- `get_suf(suf_rank)`：输入后缀排名（1-index），返回对应后缀
- `get_rank(suf_id)`：输入后缀左边界位置（0-index），返回对应字典序排名
- `get_count(pattern)`：输入模式串，返回其在文本串中出现的次数
- `get_prob(prompt)`：输入一段文本，返回其下一个字的概率分布
- `get_branch_entropy(prompt)`：输入一段文本，返回其右分支熵
- `get_mutual_information(text)`：输入一段文本，返回其互信息大小

## 安装与构建
推荐使用 pip 进行本地开发模式安装：

```bash
pip install -e .
```

或手动编译扩展模块：

```bash
python setup.py build_ext --inplace
```

将生成的库文件（如 `SuffixArray.*.so`）放置于你的项目目录下，即可在 Python 代码中导入。

## 测试

`test.ipynb`给出了简单的测试用例，如有出现不符合预期的输出欢迎在 issue 区提出。

## 注意事项
- 当前未采用线性时间构造算法
- 空间常数较大，不适合超大规模数据集
- 测试用例有限，主要供个人学习与实验使用

## 许可证
本项目采用 MIT 许可证，欢迎自由使用与修改。