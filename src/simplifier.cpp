#include "simplifier.hpp"
#include <cmath>
#include <map>
#include <vector>
#include <algorithm>

namespace symbuna {

// ヘルパー：ノードが特定の整数かチェック
bool is_int(const ExprPtr& e, int v) {
    if (e->type() == ExprType::INT) {
        return std::static_pointer_cast<Int>(e)->value() == v;
    }
    return false;
}

// ヘルパー：2つの整数の最大公約数
int gcd(int a, int b) {
    return std::gcd(std::abs(a), std::abs(b));
}

// ヘルパー：累乗を計算 (整数)
int int_pow(int base, int exp) {
    if (exp < 0) return 0; // 今回は整数の範囲なので、負の累乗は分数になるため別途処理
    int res = 1;
    while (exp > 0) {
        if (exp % 2 == 1) res *= base;
        base *= base;
        exp /= 2;
    }
    return res;
}

// ヘルパー：平方根の簡略化 (a√b の形にする)
// 戻り値は {a, b}。例えば sqrt(12) なら {2, 3}
std::pair<int, int> simplify_sqrt(int n) {
    if (n < 0) throw std::invalid_argument("負の数の平方根はサポートされていません。");
    if (n == 0) return {0, 1};
    int outside = 1;
    for (int i = 2; i * i <= n; ++i) {
        while (n % (i * i) == 0) {
            outside *= i;
            n /= (i * i);
        }
    }
    return {outside, n};
}

// =========================================================================
// 簡略化関数のメイン実装
// =========================================================================
ExprPtr simplify(ExprPtr expr) {
    if (!expr) return nullptr;

    // ボトムアップで子ノードを先に簡略化する
    switch (expr->type()) {
        case ExprType::INT:
        case ExprType::VAR:
        case ExprType::RATIONAL:
            return expr; // 末端ノードはそのまま

        case ExprType::NEG: {
            auto n = std::static_pointer_cast<Neg>(expr);
            auto op = simplify(n->operand());

            // -(-x) -> x
            if (op->type() == ExprType::NEG) {
                return std::static_pointer_cast<Neg>(op)->operand();
            }
            // -(3) -> -3 (整数や分数の場合)
            if (op->type() == ExprType::INT) {
                return std::make_shared<Int>(-std::static_pointer_cast<Int>(op)->value());
            }
            if (op->type() == ExprType::RATIONAL) {
                auto r = std::static_pointer_cast<Rational>(op);
                return std::make_shared<Rational>(-r->num(), r->den());
            }

            return std::make_shared<Neg>(op);
        }

        case ExprType::ADD: {
            auto a = std::static_pointer_cast<Add>(expr);
            auto l = simplify(a->left());
            auto r = simplify(a->right());

            // 0 + x -> x, x + 0 -> x
            if (is_int(l, 0)) return r;
            if (is_int(r, 0)) return l;

            // 定数同士の加算 (Int + Int)
            if (l->type() == ExprType::INT && r->type() == ExprType::INT) {
                int lv = std::static_pointer_cast<Int>(l)->value();
                int rv = std::static_pointer_cast<Int>(r)->value();
                return std::make_shared<Int>(lv + rv);
            }
            // 分数同士などの加算は後ほど強化

            // 同類項のまとめ (簡単なもののみ： x + x -> 2x)
            if (l->equals(r)) {
                return std::make_shared<Mul>(std::make_shared<Int>(2), l);
            }
            // (a * x) + x -> (a+1) * x などは今後のフェーズで拡張可能

            return std::make_shared<Add>(l, r);
        }

        case ExprType::MUL: {
            auto m = std::static_pointer_cast<Mul>(expr);
            auto l = simplify(m->left());
            auto r = simplify(m->right());

            // 0 * x -> 0, x * 0 -> 0
            if (is_int(l, 0) || is_int(r, 0)) return std::make_shared<Int>(0);
            // 1 * x -> x, x * 1 -> x
            if (is_int(l, 1)) return r;
            if (is_int(r, 1)) return l;

            // 定数同士の乗算
            if (l->type() == ExprType::INT && r->type() == ExprType::INT) {
                int lv = std::static_pointer_cast<Int>(l)->value();
                int rv = std::static_pointer_cast<Int>(r)->value();
                return std::make_shared<Int>(lv * rv);
            }

            return std::make_shared<Mul>(l, r);
        }

        case ExprType::DIV: {
            auto d = std::static_pointer_cast<Div>(expr);
            auto n = simplify(d->num());
            auto dn = simplify(d->den());

            if (is_int(dn, 0)) throw std::invalid_argument("ゼロ除算エラー");

            // 0 / x -> 0
            if (is_int(n, 0)) return std::make_shared<Int>(0);

            // 定数 / 定数 -> Rational
            if (n->type() == ExprType::INT && dn->type() == ExprType::INT) {
                int nv = std::static_pointer_cast<Int>(n)->value();
                int dv = std::static_pointer_cast<Int>(dn)->value();
                if (nv % dv == 0) {
                    return std::make_shared<Int>(nv / dv);
                } else {
                    return std::make_shared<Rational>(nv, dv);
                }
            }
            
            // a / a -> 1
            if (n->equals(dn)) return std::make_shared<Int>(1);

            // 分母の有理化: a / sqrt(b)
            if (dn->type() == ExprType::SQRT) {
                auto sqrt_dn = std::static_pointer_cast<Sqrt>(dn);
                if (sqrt_dn->operand()->type() == ExprType::INT) {
                    int b = std::static_pointer_cast<Int>(sqrt_dn->operand())->value();
                    // a*sqrt(b) / b
                    auto new_num = simplify(std::make_shared<Mul>(n, dn));
                    auto new_den = std::make_shared<Int>(b);
                    return simplify(std::make_shared<Div>(new_num, new_den));
                }
            }

            return std::make_shared<Div>(n, dn);
        }

        case ExprType::POW: {
            auto p = std::static_pointer_cast<Pow>(expr);
            auto base = simplify(p->base());
            auto exp = simplify(p->exp());

            // x^0 -> 1
            if (is_int(exp, 0)) return std::make_shared<Int>(1);
            // x^1 -> x
            if (is_int(exp, 1)) return base;
            // 0^x -> 0, 1^x -> 1
            if (is_int(base, 0) || is_int(base, 1)) return base;

            // 定数^定数
            if (base->type() == ExprType::INT && exp->type() == ExprType::INT) {
                int b = std::static_pointer_cast<Int>(base)->value();
                int e = std::static_pointer_cast<Int>(exp)->value();
                if (e >= 0) {
                    return std::make_shared<Int>(int_pow(b, e));
                } else {
                    // 負のべき乗 -> 1 / (b^|e|) -> 分数へ
                    return std::make_shared<Rational>(1, int_pow(b, -e));
                }
            }

            return std::make_shared<Pow>(base, exp);
        }

        case ExprType::SQRT: {
            auto s = std::static_pointer_cast<Sqrt>(expr);
            auto op = simplify(s->operand());

            // sqrt(0) -> 0, sqrt(1) -> 1
            if (is_int(op, 0)) return std::make_shared<Int>(0);
            if (is_int(op, 1)) return std::make_shared<Int>(1);

            // 定数の平方根の簡略化
            if (op->type() == ExprType::INT) {
                int v = std::static_pointer_cast<Int>(op)->value();
                auto res = simplify_sqrt(v);
                if (res.second == 1) { // 完全にルートが外れた
                    return std::make_shared<Int>(res.first);
                } else if (res.first > 1) { // a*sqrt(b) の形になる
                    return std::make_shared<Mul>(
                        std::make_shared<Int>(res.first),
                        std::make_shared<Sqrt>(std::make_shared<Int>(res.second))
                    );
                }
            }

            return std::make_shared<Sqrt>(op);
        }

        case ExprType::CBRT: {
            auto c = std::static_pointer_cast<Cbrt>(expr);
            auto op = simplify(c->operand());

            // cbrt(0) -> 0, cbrt(1) -> 1
            if (is_int(op, 0)) return std::make_shared<Int>(0);
            if (is_int(op, 1)) return std::make_shared<Int>(1);

            // cbrt(x^3) -> x (厳密にはxが実数の場合だがここでは簡易的に)
            if (op->type() == ExprType::POW) {
                auto p = std::static_pointer_cast<Pow>(op);
                if (is_int(p->exp(), 3)) return p->base();
            }

            // cbrt(定数) の計算 (例: cbrt(8) -> 2)
            if (op->type() == ExprType::INT) {
                int v = std::static_pointer_cast<Int>(op)->value();
                int root = std::round(std::cbrt(v));
                if (root * root * root == v) {
                    return std::make_shared<Int>(root);
                }
            }

            return std::make_shared<Cbrt>(op);
        }

        case ExprType::LOG: {
            auto l = std::static_pointer_cast<Log>(expr);
            auto op = simplify(l->operand());

            // ln(1) -> 0
            if (is_int(op, 1)) return std::make_shared<Int>(0);
            
            // ln(e) -> 1
            if (op->type() == ExprType::E) return std::make_shared<Int>(1);

            // ln(e^x) -> x
            if (op->type() == ExprType::POW) {
                auto p = std::static_pointer_cast<Pow>(op);
                if (p->base()->type() == ExprType::E) {
                    return p->exp();
                }
            }

            return std::make_shared<Log>(op);
        }

        case ExprType::SIN:
        case ExprType::COS:
        case ExprType::TAN: {
            auto type = expr->type();
            ExprPtr op;
            if (type == ExprType::SIN) op = std::static_pointer_cast<Sin>(expr)->operand();
            else if (type == ExprType::COS) op = std::static_pointer_cast<Cos>(expr)->operand();
            else op = std::static_pointer_cast<Tan>(expr)->operand();
            
            op = simplify(op);

            // sin(0)->0, cos(0)->1, tan(0)->0
            if (is_int(op, 0)) {
                if (type == ExprType::COS) return std::make_shared<Int>(1);
                return std::make_shared<Int>(0);
            }

            // 特殊角の処理 (pi/4, pi/2 など)
            // ここでは簡易的に Div(Pi, Int(d)) または Div(Mul(Int(n), Pi), Int(d)) をチェック
            int n = 1, d = 1;
            bool is_pi_mul = false;

            if (op->type() == ExprType::PI) {
                is_pi_mul = true; n = 1; d = 1;
            } else if (op->type() == ExprType::DIV) {
                auto div = std::static_pointer_cast<Div>(op);
                auto num = div->num();
                auto den = div->den();
                if (den->type() == ExprType::INT) {
                    d = std::static_pointer_cast<Int>(den)->value();
                    if (num->type() == ExprType::PI) {
                        is_pi_mul = true; n = 1;
                    } else if (num->type() == ExprType::MUL) {
                        auto m = std::static_pointer_cast<Mul>(num);
                        if (m->left()->type() == ExprType::INT && m->right()->type() == ExprType::PI) {
                            is_pi_mul = true;
                            n = std::static_pointer_cast<Int>(m->left())->value();
                        }
                    }
                }
            }

            if (is_pi_mul) {
                // n*pi / d の形になっている。角度を主値に直すなどの厳密な処理は省略し、典型的な値のみ。
                // 優先度C: sin(pi/4) -> sqrt(2)/2
                if (n == 1 && d == 4) {
                    if (type == ExprType::SIN || type == ExprType::COS) {
                        // sqrt(2) / 2
                        return std::make_shared<Div>(
                            std::make_shared<Sqrt>(std::make_shared<Int>(2)),
                            std::make_shared<Int>(2)
                        );
                    } else if (type == ExprType::TAN) {
                        return std::make_shared<Int>(1);
                    }
                }
                if (n == 1 && d == 1) { // pi
                    if (type == ExprType::SIN) return std::make_shared<Int>(0);
                    if (type == ExprType::COS) return std::make_shared<Int>(-1);
                    if (type == ExprType::TAN) return std::make_shared<Int>(0);
                }
                if (n == 1 && d == 2) { // pi/2
                    if (type == ExprType::SIN) return std::make_shared<Int>(1);
                    if (type == ExprType::COS) return std::make_shared<Int>(0);
                    if (type == ExprType::TAN) throw std::invalid_argument("tan(pi/2) は定義されていません");
                }
            }

            if (type == ExprType::SIN) return std::make_shared<Sin>(op);
            if (type == ExprType::COS) return std::make_shared<Cos>(op);
            return std::make_shared<Tan>(op);
        }

        default:
            return expr;
    }
}

struct Monomial {
    int coeff;
    std::vector<std::pair<ExprPtr, int>> vars;
};

Monomial parse_monomial(ExprPtr expr) {
    if (expr->type() == ExprType::INT) {
        return {std::static_pointer_cast<Int>(expr)->value(), {}};
    }
    if (expr->type() == ExprType::NEG) {
        Monomial m = parse_monomial(std::static_pointer_cast<Neg>(expr)->operand());
        m.coeff = -m.coeff;
        return m;
    }
    if (expr->type() == ExprType::MUL) {
        Monomial m1 = parse_monomial(std::static_pointer_cast<Mul>(expr)->left());
        Monomial m2 = parse_monomial(std::static_pointer_cast<Mul>(expr)->right());
        
        Monomial res;
        res.coeff = m1.coeff * m2.coeff;
        res.vars = m1.vars;
        
        for (auto& v2 : m2.vars) {
            bool found = false;
            for (auto& v1 : res.vars) {
                if (v1.first->equals(v2.first)) {
                    v1.second += v2.second;
                    found = true;
                    break;
                }
            }
            if (!found) {
                res.vars.push_back(v2);
            }
        }
        return res;
    }
    if (expr->type() == ExprType::POW) {
        auto p = std::static_pointer_cast<Pow>(expr);
        if (p->exp()->type() == ExprType::INT) {
            int e = std::static_pointer_cast<Int>(p->exp())->value();
            return {1, {{p->base(), e}}};
        }
    }
    return {1, {{expr, 1}}};
}

ExprPtr build_monomial(const Monomial& m) {
    ExprPtr res = nullptr;
    
    std::vector<std::pair<ExprPtr, int>> sorted_vars = m.vars;
    std::sort(sorted_vars.begin(), sorted_vars.end(), [](const auto& a, const auto& b) {
        return a.first->to_string() > b.first->to_string();
    });
    
    for (auto& v : sorted_vars) {
        if (v.second == 0) continue;
        ExprPtr part;
        if (v.second == 1) {
            part = v.first;
        } else {
            part = std::make_shared<Pow>(v.first, std::make_shared<Int>(v.second));
        }
        
        if (!res) {
            res = part;
        } else {
            res = std::make_shared<Mul>(res, part);
        }
    }
    
    if (m.coeff == 0) return std::make_shared<Int>(0);
    if (!res) return std::make_shared<Int>(m.coeff);
    if (m.coeff == 1) return res;
    if (m.coeff == -1) return std::make_shared<Neg>(res);
    
    return std::make_shared<Mul>(std::make_shared<Int>(m.coeff), res);
}

void flatten_add(ExprPtr expr, std::vector<Monomial>& terms) {
    if (!expr) return;
    if (expr->type() == ExprType::ADD) {
        auto a = std::static_pointer_cast<Add>(expr);
        flatten_add(a->left(), terms);
        flatten_add(a->right(), terms);
    } else {
        terms.push_back(parse_monomial(expr));
    }
}

ExprPtr collect_terms(ExprPtr expr) {
    std::vector<Monomial> terms;
    flatten_add(expr, terms);
    
    std::map<std::string, Monomial> term_map;
    
    for (auto& m : terms) {
        if (m.coeff == 0) continue;
        
        Monomial var_only = m;
        var_only.coeff = 1;
        std::string key = build_monomial(var_only)->to_string();
        
        if (term_map.find(key) == term_map.end()) {
            term_map[key] = m;
        } else {
            term_map[key].coeff += m.coeff;
        }
    }
    
    std::vector<std::pair<std::string, Monomial>> sorted_terms(term_map.begin(), term_map.end());
    
    std::sort(sorted_terms.begin(), sorted_terms.end(), [](const auto& a, const auto& b) {
        if (a.first == "1" && b.first != "1") return false;
        if (b.first == "1" && a.first != "1") return true;
        return a.first > b.first;
    });

    ExprPtr result = nullptr;
    for (auto& kv : sorted_terms) {
        if (kv.second.coeff == 0) continue;
        ExprPtr term_expr = build_monomial(kv.second);
        
        if (!result) {
            result = term_expr;
        } else {
            result = std::make_shared<Add>(result, term_expr);
        }
    }
    
    if (!result) return std::make_shared<Int>(0);
    return result;
}

ExprPtr distribute_mul(ExprPtr l, ExprPtr r) {
    if (l->type() == ExprType::ADD) {
        auto a = std::static_pointer_cast<Add>(l);
        return std::make_shared<Add>(
            distribute_mul(a->left(), r),
            distribute_mul(a->right(), r)
        );
    }
    if (r->type() == ExprType::ADD) {
        auto a = std::static_pointer_cast<Add>(r);
        return std::make_shared<Add>(
            distribute_mul(l, a->left()),
            distribute_mul(l, a->right())
        );
    }
    return std::make_shared<Mul>(l, r);
}

ExprPtr distribute_pow(ExprPtr base, int exp) {
    if (exp == 0) return std::make_shared<Int>(1);
    if (exp == 1) return base;
    
    ExprPtr half = distribute_pow(base, exp / 2);
    ExprPtr res = collect_terms(distribute_mul(half, half));
    if (exp % 2 == 1) {
        res = collect_terms(distribute_mul(res, base));
    }
    return res;
}

ExprPtr expand(ExprPtr expr) {
    if (!expr) return nullptr;
    
    expr = simplify(expr);
    
    switch (expr->type()) {
        case ExprType::ADD: {
            auto a = std::static_pointer_cast<Add>(expr);
            auto l = expand(a->left());
            auto r = expand(a->right());
            return collect_terms(std::make_shared<Add>(l, r));
        }
        case ExprType::MUL: {
            auto m = std::static_pointer_cast<Mul>(expr);
            auto l = expand(m->left());
            auto r = expand(m->right());
            auto res = distribute_mul(l, r);
            return collect_terms(res);
        }
        case ExprType::POW: {
            auto p = std::static_pointer_cast<Pow>(expr);
            auto base = expand(p->base());
            if (p->exp()->type() == ExprType::INT) {
                int e = std::static_pointer_cast<Int>(p->exp())->value();
                if (e >= 0) {
                    auto res = distribute_pow(base, e);
                    return collect_terms(res);
                }
            }
            return std::make_shared<Pow>(base, p->exp());
        }
        case ExprType::NEG: {
            auto n = std::static_pointer_cast<Neg>(expr);
            return collect_terms(std::make_shared<Neg>(expand(n->operand())));
        }
        case ExprType::SQRT: {
            auto s = std::static_pointer_cast<Sqrt>(expr);
            return std::make_shared<Sqrt>(expand(s->operand()));
        }
        case ExprType::CBRT: {
            auto c = std::static_pointer_cast<Cbrt>(expr);
            return std::make_shared<Cbrt>(expand(c->operand()));
        }
        case ExprType::SIN: {
            auto s = std::static_pointer_cast<Sin>(expr);
            return std::make_shared<Sin>(expand(s->operand()));
        }
        case ExprType::COS: {
            auto c = std::static_pointer_cast<Cos>(expr);
            return std::make_shared<Cos>(expand(c->operand()));
        }
        case ExprType::TAN: {
            auto t = std::static_pointer_cast<Tan>(expr);
            return std::make_shared<Tan>(expand(t->operand()));
        }
        case ExprType::LOG: {
            auto l = std::static_pointer_cast<Log>(expr);
            return std::make_shared<Log>(expand(l->operand()));
        }
        default:
            return expr;
    }
}

} // namespace symbuna
