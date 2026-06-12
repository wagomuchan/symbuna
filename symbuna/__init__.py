"""
SymBuna ── 高校数Ⅲレベルの数式処理ライブラリ
"""

from _symbuna_core import Expr, Int, Var, Rational, Neg, Add, Mul, Div, Pow, Sqrt, Pi, E, Cbrt, Sin, Cos, Tan, Log, simplify
from .parser import parse, ParseError

__all__ = [
    "Expr", "Int", "Var", "Rational",
    "Neg", "Add", "Mul", "Div", "Pow", "Sqrt",
    "Pi", "E", "Cbrt", "Sin", "Cos", "Tan", "Log",
    "simplify",
    "parse", "ParseError"
]

# Jupyter Notebook 用のリッチ表示（LaTeX形式）
def _expr_repr_latex_(self):
    return f"$${self.to_string()}$$"

# print() された時のための表示（基本はLaTeX文字列）
def _expr_str_(self):
    return self.to_string()

Expr._repr_latex_ = _expr_repr_latex_
Expr.__str__ = _expr_str_
