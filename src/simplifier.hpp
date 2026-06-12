#ifndef SYMBUNA_SIMPLIFIER_HPP
#define SYMBUNA_SIMPLIFIER_HPP

#include "ast.hpp"

namespace symbuna {

// 数式を簡略化する（再帰的にボトムアップで適用）
//
// 適用される主なルール:
//   - 定数畳み込み（2 + 3 → 5, 1/2 + 1/3 → 5/6）
//   - 恒等元の除去（x + 0 → x, x × 1 → x）
//   - 二重否定の除去（--x → x）
//   - 同類項のまとめ（2x + 3x → 5x）
//   - 平方根の整理（√12 → 2√3）
//   - 分母の有理化（1/√2 → √2/2）
//   - べき乗の計算（2^3 → 8, x^0 → 1）
ExprPtr simplify(ExprPtr expr);
ExprPtr expand(ExprPtr expr);

} // namespace symbuna

#endif // SYMBUNA_SIMPLIFIER_HPP
