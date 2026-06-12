#include <pybind11/pybind11.h>
#include "ast.hpp"
#include "simplifier.hpp"

namespace py = pybind11;

PYBIND11_MODULE(_symbuna_core, m) {
    m.doc() = "SymBuna: 高校数Ⅲレベルの数式処理ライブラリ (C++ core)";

    // ── 基底クラス Expr ─────────────────────────────
    py::class_<symbuna::Expr, std::shared_ptr<symbuna::Expr>>(m, "Expr")
        .def("to_string", &symbuna::Expr::to_string,
             "LaTeX形式の文字列を返す");

    // ── 整数ノード Int ──────────────────────────────
    py::class_<symbuna::Int, symbuna::Expr, std::shared_ptr<symbuna::Int>>(m, "Int")
        .def(py::init<int>(), py::arg("value"))
        .def("value", &symbuna::Int::value);

    // ── 変数ノード Var ──────────────────────────────
    py::class_<symbuna::Var, symbuna::Expr, std::shared_ptr<symbuna::Var>>(m, "Var")
        .def(py::init<const std::string&>(), py::arg("name"))
        .def("name", &symbuna::Var::name);

    // ── 有理数ノード Rational ───────────────────────
    py::class_<symbuna::Rational, symbuna::Expr, std::shared_ptr<symbuna::Rational>>(m, "Rational")
        .def(py::init<int, int>(), py::arg("num"), py::arg("den"))
        .def("num", &symbuna::Rational::num)
        .def("den", &symbuna::Rational::den);

    // ── 符号反転ノード Neg ──────────────────────────
    py::class_<symbuna::Neg, symbuna::Expr, std::shared_ptr<symbuna::Neg>>(m, "Neg")
        .def(py::init<std::shared_ptr<symbuna::Expr>>(), py::arg("operand"))
        .def("operand", &symbuna::Neg::operand);

    // ── 加算ノード Add ──────────────────────────────
    py::class_<symbuna::Add, symbuna::Expr, std::shared_ptr<symbuna::Add>>(m, "Add")
        .def(py::init<std::shared_ptr<symbuna::Expr>, std::shared_ptr<symbuna::Expr>>(), py::arg("left"), py::arg("right"))
        .def("left", &symbuna::Add::left)
        .def("right", &symbuna::Add::right);

    // ── 乗算ノード Mul ──────────────────────────────
    py::class_<symbuna::Mul, symbuna::Expr, std::shared_ptr<symbuna::Mul>>(m, "Mul")
        .def(py::init<std::shared_ptr<symbuna::Expr>, std::shared_ptr<symbuna::Expr>>(), py::arg("left"), py::arg("right"))
        .def("left", &symbuna::Mul::left)
        .def("right", &symbuna::Mul::right);

    // ── 除算ノード Div ──────────────────────────────
    py::class_<symbuna::Div, symbuna::Expr, std::shared_ptr<symbuna::Div>>(m, "Div")
        .def(py::init<std::shared_ptr<symbuna::Expr>, std::shared_ptr<symbuna::Expr>>(), py::arg("num"), py::arg("den"))
        .def("num", &symbuna::Div::num)
        .def("den", &symbuna::Div::den);

    // ── べき乗ノード Pow ──────────────────────────────
    py::class_<symbuna::Pow, symbuna::Expr, std::shared_ptr<symbuna::Pow>>(m, "Pow")
        .def(py::init<std::shared_ptr<symbuna::Expr>, std::shared_ptr<symbuna::Expr>>(), py::arg("base"), py::arg("exp"))
        .def("base", &symbuna::Pow::base)
        .def("exp", &symbuna::Pow::exp);

    // ── 平方根ノード Sqrt ──────────────────────────────
    py::class_<symbuna::Sqrt, symbuna::Expr, std::shared_ptr<symbuna::Sqrt>>(m, "Sqrt")
        .def(py::init<std::shared_ptr<symbuna::Expr>>(), py::arg("operand"))
        .def("operand", &symbuna::Sqrt::operand);

    // ── 簡略化関数 simplify ───────────────────────────
    m.def("simplify", &symbuna::simplify, "数式を簡略化する", py::arg("expr"));
}
