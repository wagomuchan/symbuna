"""
フェーズ1 & 2 テスト: 演算子ノードと簡略化の動作確認
"""

import symbuna
from symbuna import Int, Var, Rational, Neg, Add, Mul, Div, Pow, Sqrt, simplify


def test_phase1_nodes():
    """フェーズ1の基本ノードテスト"""
    n = Int(5)
    assert n.to_string() == "5"
    x = Var("x")
    assert x.to_string() == "x"


def test_phase2_operators():
    """フェーズ2: 演算子ノードの生成とLaTeX出力テスト"""
    x = Var("x")
    y = Var("y")
    
    # x + y
    add = Add(x, y)
    assert add.to_string() == "x + y"
    
    # x - y -> Add(x, Neg(y))
    sub = Add(x, Neg(y))
    assert sub.to_string() == "x - y"
    
    # x * y
    mul = Mul(x, y)
    assert mul.to_string() == "xy"
    
    # 2 * x
    mul2 = Mul(Int(2), x)
    assert mul2.to_string() == "2x"
    
    # x / y
    div = Div(x, y)
    assert div.to_string() == "\\frac{x}{y}"
    
    # x^2
    pow_node = Pow(x, Int(2))
    assert pow_node.to_string() == "x^{2}"
    
    # sqrt(x)
    sqrt_node = Sqrt(x)
    assert sqrt_node.to_string() == "\\sqrt{x}"


def test_phase2_rational():
    """有理数の自動約分テスト"""
    # 2/4 -> 1/2
    r1 = Rational(2, 4)
    assert r1.num() == 1
    assert r1.den() == 2
    assert r1.to_string() == "\\frac{1}{2}"
    
    # 負の分数: 3/-6 -> -1/2
    r2 = Rational(3, -6)
    assert r2.num() == -1
    assert r2.den() == 2
    assert r2.to_string() == "-\\frac{1}{2}"


def test_phase2_simplify():
    """簡略化エンジンのテスト"""
    # 1. 定数畳み込み: 2 + 3 -> 5
    expr1 = Add(Int(2), Int(3))
    sim1 = simplify(expr1)
    assert isinstance(sim1, Int)
    assert sim1.to_string() == "5"
    
    # 2. 同類項のまとめ: x + x -> 2x
    x = Var("x")
    expr2 = Add(x, x)
    sim2 = simplify(expr2)
    assert isinstance(sim2, Mul)
    assert sim2.to_string() == "2x"
    
    # 3. 恒等元の除去: x * 1 -> x, x + 0 -> x
    expr3 = Mul(x, Int(1))
    sim3 = simplify(expr3)
    assert sim3.to_string() == "x"
    
    expr4 = Add(x, Int(0))
    sim4 = simplify(expr4)
    assert sim4.to_string() == "x"


def test_phase2_sqrt_simplify():
    """平方根の簡略化と有理化のテスト"""
    # sqrt(12) -> 2*sqrt(3)
    expr1 = Sqrt(Int(12))
    sim1 = simplify(expr1)
    assert sim1.to_string() == "2\\sqrt{3}"
    
    # 1 / sqrt(2) -> sqrt(2) / 2
    expr2 = Div(Int(1), Sqrt(Int(2)))
    sim2 = simplify(expr2)
    # 結果は Div(Mul(1, Sqrt(2)), 2) だが、1* は消えるはず
    assert sim2.to_string() == "\\frac{\\sqrt{2}}{2}"


if __name__ == "__main__":
    test_phase1_nodes()
    test_phase2_operators()
    test_phase2_rational()
    test_phase2_simplify()
    test_phase2_sqrt_simplify()
    print("✅ フェーズ2: すべてのテストに合格しました！")
