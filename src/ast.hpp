#ifndef SYMBUNA_AST_HPP
#define SYMBUNA_AST_HPP

#include <string>
#include <memory>

namespace symbuna {

// ── 基底クラス ──────────────────────────────────────
// すべての数式ノードはこのクラスを継承する。
// shared_ptr で管理し、木構造を安全に共有できるようにする。
class Expr {
public:
    virtual ~Expr() = default;

    // LaTeX形式の文字列を返す
    virtual std::string to_string() const = 0;
};

// スマートポインタのエイリアス
using ExprPtr = std::shared_ptr<Expr>;

// ── 整数ノード ──────────────────────────────────────
class Int : public Expr {
public:
    explicit Int(int value) : value_(value) {}

    int value() const { return value_; }

    std::string to_string() const override {
        return std::to_string(value_);
    }

private:
    int value_;
};

// ── 変数ノード ──────────────────────────────────────
class Var : public Expr {
public:
    explicit Var(const std::string& name) : name_(name) {}

    const std::string& name() const { return name_; }

    std::string to_string() const override {
        return name_;
    }

private:
    std::string name_;
};

} // namespace symbuna

#endif // SYMBUNA_AST_HPP
