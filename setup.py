"""
SymBuna ── ビルド設定
pip install -e .  でインストール可能
"""

from setuptools import setup, Extension, find_packages
from setuptools.command.build_ext import build_ext
import pybind11
import sys
import os


class BuildExt(build_ext):
    """pybind11 用のカスタムビルドコマンド"""

    def build_extensions(self):
        # コンパイラに応じたフラグの設定
        compiler_type = self.compiler.compiler_type
        for ext in self.extensions:
            if compiler_type == "msvc":
                ext.extra_compile_args = ["/std:c++17", "/EHsc"]
            else:
                ext.extra_compile_args = ["-std=c++17", "-fvisibility=hidden"]
        build_ext.build_extensions(self)


ext_modules = [
    Extension(
        "_symbuna_core",
        sources=["src/main.cpp", "src/simplifier.cpp"],
        include_dirs=[
            pybind11.get_include(),
            pybind11.get_include(user=True),
            "src",
        ],
        language="c++",
    ),
]

setup(
    name="symbuna",
    version="0.2.1",
    author="SymBuna Developers",
    description="高校数Ⅲレベルの数式処理ライブラリ",
    long_description=open("readme.md", encoding="utf-8").read(),
    long_description_content_type="text/markdown",
    packages=find_packages(),
    ext_modules=ext_modules,
    cmdclass={"build_ext": BuildExt},
    python_requires=">=3.8",
    install_requires=["pybind11>=2.10"],
)
