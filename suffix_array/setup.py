from setuptools import setup, Extension
from Cython.Build import cythonize

ext_modules = [
    Extension(
        name="SuffixArray",
        sources=["suffix_array.pyx", "src/suffix_array.cpp"],
        language="c++",
        include_dirs=["."],
        extra_compile_args=["-std=c++11", "-O2"],
    )
]

setup(
    ext_modules=cythonize(ext_modules)
)