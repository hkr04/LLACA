from setuptools import setup, Extension
from Cython.Build import cythonize

ext_modules = [
    Extension(
        name="automaton.Automaton",
        sources=["automaton/automaton.pyx", "automaton/src/automaton.cpp"],
        language="c++",
        include_dirs=["."],
        extra_compile_args=["-std=c++11", "-O2"],
    ),
    Extension(
        name="suffix_array.SuffixArray",
        sources=["suffix_array/suffix_array.pyx", "suffix_array/src/suffix_array.cpp"],
        language="c++",
        include_dirs=["."],
        extra_compile_args=["-std=c++11", "-O2"]
    )
]

setup(
    ext_modules=cythonize(ext_modules)
)
