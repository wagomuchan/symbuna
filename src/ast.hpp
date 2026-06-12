#ifndef SYMBUNA_AST_HPP
#define SYMBUNA_AST_HPP

#include <string>
#include <memory>
#include <numeric>   // std::gcd
#include <cstdlib>   // std::abs
#include <stdexcept>
#include <cctype>    // std::isdigit

namespace symbuna {

class Expr;
using ExprPtr = std::shared_ptr<Expr>;

// ノードの種類を識別する列挙型
enum class ExprType {
    INT, VAR, RATIONAL, NEG, ADD, MUL, DIV, POW, SQRT
};

// ── 基底クラス ──────────────────────────────────────
// すべての数式ノードはこのクラスを継承する。
// shared_ptr で管理し、木構造を安全に共有できるようにする。
class Expr {
public:
    virtual ~Expr() = default;

    // LaTeX形式の文字列を返す
    virtual std::string to_string() const = 0;

    // ノードの種類
    virtual ExprType type() const = 0;

    // 演算子の優先順位（括弧付けの判定に使用）
    //   Add = 1, Neg = 2, Mul/Div = 3, Pow = 4, Atom = 5
    virtual int precedence() const = 0;

    // 構造的な等価性比較（同類項の判定に使用）
    virtual bool equals(const ExprPtr& other) const = 0;

    // 数値リテラル（Int / Rational）であるか
    virtual bool is_numeric() const { return false; }

protected:
    // 子ノードの優先度が min_prec より低ければ \left( ... \right) で囲む
    static std::string parenthesize(const ExprPtr& child, int min_prec) {
        if (child->precedence() < min_prec) {
            return "\\left(" + child->to_string() + "\\right)";
        }
        return child->to_string();
    }
};

// スマートポインタのエイリアス（利便性のため再掲）
// using ExprPtr = std::shared_ptr<Expr>;  // 上で定義済み

// ── 整数ノード ──────────────────────────────────────
class Int : public Expr {
public:
    explicit Int(int value) : value_(value) {}

    int value() const { return value_; }

    ExprType type() const override { return ExprType::INT; }
    int precedence() const override { return 5; }
    bool is_numeric() const override { return true; }

    std::string to_string() const override {
        return std::to_string(value_);
    }

    bool equals(const ExprPtr& other) const override {
        return other->type() == ExprType::INT &&
               value_ == std::static_pointer_cast<Int>(other)->value_;
    }

private:
    int value_;
};

// ── 変数ノード ──────────────────────────────────────
class Var : public Expr {
public:
    explicit Var(const std::string& name) : name_(name) {}

    const std::string& name() const { return name_; }

    ExprType type() const override { return ExprType::VAR; }
    int precedence() const override { return 5; }

    std::string to_string() const override { return name_; }

    bool equals(const ExprPtr& other) const override {
        return other->type() == ExprType::VAR &&
               name_ == std::static_pointer_cast<Var>(other)->name_;
    }

private:
    std::string name_;
};

// ── 有理数（分数）ノード ────────────────────────────
// 分子・分母のペアで保持し、自動的に約分する。
// 分母は常に正に保つ。
class Rational : public Expr {
public:
    Rational(int num, int den) : num_(num), den_(den) {
        if (den_ == 0)
            throw std::invalid_argument(
                "エラー: 分母をゼロにすることはできません。");
        if (den_ < 0) { num_ = -num_; den_ = -den_; }
        int g = std::gcd(std::abs(num_), den_);
        num_ /= g;
        den_ /= g;
    }

    int num() const { return num_; }
    int den() const { return den_; }

    ExprType type() const override { return ExprType::RATIONAL; }
    int precedence() const override { return 5; }
    bool is_numeric() const override { return true; }

    std::string to_string() const override {
        if (den_ == 1) return std::to_string(num_);
        if (num_ < 0)
            return "-\\frac{" + std::to_string(-num_) +
                   "}{" + std::to_string(den_) + "}";
        return "\\frac{" + std::to_string(num_) +
               "}{" + std::to_string(den_) + "}";
    }

    bool equals(const ExprPtr& other) const override {
        if (other->type() != ExprType::RATIONAL) return false;
        auto r = std::static_pointer_cast<Rational>(other);
        return num_ == r->num_ && den_ == r->den_;
    }

private:
    int num_, den_;
};

// ── 符号反転ノード ──────────────────────────────────
// 単項マイナス。減算は Add(a, Neg(b)) で表現する。
class Neg : public Expr {
public:
    explicit Neg(ExprPtr operand) : operand_(std::move(operand)) {}

    const ExprPtr& operand() const { return operand_; }

    ExprType type() const override { return ExprType::NEG; }
    int precedence() const override { return 2; }

    std::string to_string() const override {
        return "-" + parenthesize(operand_, 2);
    }

    bool equals(const ExprPtr& other) const override {
        return other->type() == ExprType::NEG &&
               operand_->equals(
                   std::static_pointer_cast<Neg>(other)->operand_);
    }

private:
    ExprPtr operand_;
};

// ── 加算ノード ──────────────────────────────────────
class Add : public Expr {
public:
    Add(ExprPtr left, ExprPtr right)
        : left_(std::move(left)), right_(std::move(right)) {}

    const ExprPtr& left()  const { return left_; }
    const ExprPtr& right() const { return right_; }

    ExprType type() const override { return ExprType::ADD; }
    int precedence() const override { return 1; }

    std::string to_string() const override {
        std::string ls = left_->to_string();

        // 右辺が Neg(x) のとき "a - x" と表示
        if (right_->type() == ExprType::NEG) {
            auto neg = std::static_pointer_cast<Neg>(right_);
            return ls + " - " + parenthesize(neg->operand(), 2);
        }
        // 右辺が負の整数のとき "a - |n|"
        if (right_->type() == ExprType::INT) {
            int v = std::static_pointer_cast<Int>(right_)->value();
            if (v < 0) return ls + " - " + std::to_string(-v);
        }
        // 右辺が負の有理数のとき
        if (right_->type() == ExprType::RATIONAL) {
            auto r = std::static_pointer_cast<Rational>(right_);
            if (r->num() < 0) {
                if (r->den() == 1)
                    return ls + " - " + std::to_string(-r->num());
                return ls + " - \\frac{" + std::to_string(-r->num()) +
                       "}{" + std::to_string(r->den()) + "}";
            }
        }
        return ls + " + " + right_->to_string();
    }

    bool equals(const ExprPtr& other) const override {
        if (other->type() != ExprType::ADD) return false;
        auto a = std::static_pointer_cast<Add>(other);
        return left_->equals(a->left_) && right_->equals(a->right_);
    }

private:
    ExprPtr left_, right_;
};

// ── 乗算ノード ──────────────────────────────────────
class Mul : public Expr {
public:
    Mul(ExprPtr left, ExprPtr right)
        : left_(std::move(left)), right_(std::move(right)) {}

    const ExprPtr& left()  const { return left_; }
    const ExprPtr& right() const { return right_; }

    ExprType type() const override { return ExprType::MUL; }
    int precedence() const override { return 3; }

    std::string to_string() const override {
        // (-1) × x → "-x"
        if (left_->type() == ExprType::INT &&
            std::static_pointer_cast<Int>(left_)->value() == -1) {
            return "-" + parenthesize(right_, 3);
        }
        // 1 × x → "x"（簡略化前でも見やすく）
        if (left_->type() == ExprType::INT &&
            std::static_pointer_cast<Int>(left_)->value() == 1) {
            return parenthesize(right_, 3);
        }
        // x × 1 → "x"
        if (right_->type() == ExprType::INT &&
            std::static_pointer_cast<Int>(right_)->value() == 1) {
            return parenthesize(left_, 3);
        }

        std::string ls = parenthesize(left_, 3);
        std::string rs = parenthesize(right_, 3);

        // 数値 × 数値 → "a \cdot b"
        if (left_->is_numeric() && right_->is_numeric())
            return ls + " \\cdot " + rs;

        // 数字の衝突を防ぐ（"32" にならないように）
        if (!ls.empty() && !rs.empty() &&
            std::isdigit(static_cast<unsigned char>(ls.back())) &&
            std::isdigit(static_cast<unsigned char>(rs.front())))
            return ls + " \\cdot " + rs;

        // それ以外は並置（juxtaposition）: "5x", "xy" など
        return ls + rs;
    }

    bool equals(const ExprPtr& other) const override {
        if (other->type() != ExprType::MUL) return false;
        auto m = std::static_pointer_cast<Mul>(other);
        return left_->equals(m->left_) && right_->equals(m->right_);
    }

private:
    ExprPtr left_, right_;
};

// ── 除算（分数）ノード ──────────────────────────────
// 常に \frac{分子}{分母} として表示する。
class Div : public Expr {
public:
    Div(ExprPtr num, ExprPtr den)
        : num_(std::move(num)), den_(std::move(den)) {}

    const ExprPtr& num() const { return num_; }
    const ExprPtr& den() const { return den_; }

    ExprType type() const override { return ExprType::DIV; }
    int precedence() const override { return 3; }

    std::string to_string() const override {
        // \frac の中は括弧不要
        return "\\frac{" + num_->to_string() +
               "}{" + den_->to_string() + "}";
    }

    bool equals(const ExprPtr& other) const override {
        if (other->type() != ExprType::DIV) return false;
        auto d = std::static_pointer_cast<Div>(other);
        return num_->equals(d->num_) && den_->equals(d->den_);
    }

private:
    ExprPtr num_, den_;
};

// ── べき乗ノード ────────────────────────────────────
class Pow : public Expr {
public:
    Pow(ExprPtr base, ExprPtr exp)
        : base_(std::move(base)), exp_(std::move(exp)) {}

    const ExprPtr& base() const { return base_; }
    const ExprPtr& exp()  const { return exp_; }

    ExprType type() const override { return ExprType::POW; }
    int precedence() const override { return 4; }

    std::string to_string() const override {
        bool needs_parens = base_->precedence() < 4;

        // Pow(Pow(a,m), n) → 括弧が必要
        if (base_->type() == ExprType::POW)
            needs_parens = true;

        // 負の数の底 → 括弧が必要（(-2)^2 ≠ -2^2）
        if (base_->type() == ExprType::INT &&
            std::static_pointer_cast<Int>(base_)->value() < 0)
            needs_parens = true;
        if (base_->type() == ExprType::RATIONAL &&
            std::static_pointer_cast<Rational>(base_)->num() < 0)
            needs_parens = true;

        std::string bs = needs_parens
            ? "\\left(" + base_->to_string() + "\\right)"
            : base_->to_string();
        return bs + "^{" + exp_->to_string() + "}";
    }

    bool equals(const ExprPtr& other) const override {
        if (other->type() != ExprType::POW) return false;
        auto p = std::static_pointer_cast<Pow>(other);
        return base_->equals(p->base_) && exp_->equals(p->exp_);
    }

private:
    ExprPtr base_, exp_;
};

// ── 平方根ノード ────────────────────────────────────
class Sqrt : public Expr {
public:
    explicit Sqrt(ExprPtr operand) : operand_(std::move(operand)) {}

    const ExprPtr& operand() const { return operand_; }

    ExprType type() const override { return ExprType::SQRT; }
    int precedence() const override { return 5; }

    std::string to_string() const override {
        return "\\sqrt{" + operand_->to_string() + "}";
    }

    bool equals(const ExprPtr& other) const override {
        return other->type() == ExprType::SQRT &&
               operand_->equals(
                   std::static_pointer_cast<Sqrt>(other)->operand_);
    }

private:
    ExprPtr operand_;
};

} // namespace symbuna

#endif // SYMBUNA_AST_HPP
