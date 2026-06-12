#include <pybind11/pybind11.h>
#include "ast.hpp"

namespace py = pybind11;

PYBIND11_MODULE(_symbuna_core, m) {
    m.doc() = "SymBuna: 高校数Ⅲレベルの数式処理ライブラリ (C++ core)";

    // ── 基底クラス Expr ─────────────────────────────
    py::class_<symbuna::Expr, std::shared_ptr<symbuna::Expr>>(m, "Expr")
        .def("to_string", &symbuna::Expr::to_string,
             "LaTeX形式の文字列を返す");

    // ── 整数ノード Int ──────────────────────────────
    py::class_<symbuna::Int, symbuna::Expr,
               std::shared_ptr<symbuna::Int>>(m, "Int")
        .def(py::init<int>(), py::arg("value"),
             "整数ノードを作成する")
        .def("value", &symbuna::Int::value,
             "保持している整数値を返す")
        .def("to_string", &symbuna::Int::to_string);

    // ── 変数ノード Var ──────────────────────────────
    py::class_<symbuna::Var, symbuna::Expr,
               std::shared_ptr<symbuna::Var>>(m, "Var")
        .def(py::init<const std::string&>(), py::arg("name"),
             "変数ノードを作成する")
        .def("name", &symbuna::Var::name,
             "変数名を返す")
        .def("to_string", &symbuna::Var::to_string);
}
