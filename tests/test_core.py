"""
フェーズ1 テスト: Int / Var ノードの基本動作
"""

import symbuna


def test_int_node():
    """整数ノードの生成と LaTeX 出力"""
    n = symbuna.Int(5)
    assert n.value() == 5
    assert n.to_string() == "5"


def test_int_negative():
    """負の整数"""
    n = symbuna.Int(-3)
    assert n.value() == -3
    assert n.to_string() == "-3"


def test_int_zero():
    """ゼロ"""
    n = symbuna.Int(0)
    assert n.value() == 0
    assert n.to_string() == "0"


def test_var_node():
    """変数ノードの生成と LaTeX 出力"""
    x = symbuna.Var("x")
    assert x.name() == "x"
    assert x.to_string() == "x"


def test_var_multichar():
    """複数文字の変数名"""
    theta = symbuna.Var("theta")
    assert theta.name() == "theta"
    assert theta.to_string() == "theta"


def test_inheritance():
    """Int / Var は Expr のサブクラス"""
    n = symbuna.Int(1)
    x = symbuna.Var("x")
    assert isinstance(n, symbuna.Expr)
    assert isinstance(x, symbuna.Expr)


if __name__ == "__main__":
    test_int_node()
    test_int_negative()
    test_int_zero()
    test_var_node()
    test_var_multichar()
    test_inheritance()
    print("✅ フェーズ1: すべてのテストに合格しました！")
