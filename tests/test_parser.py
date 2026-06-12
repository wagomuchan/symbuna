"""
フェーズ4 テスト: 人間スタイルパーサーの動作確認
"""

import pytest
from symbuna import parse, ParseError


def test_basic_parsing():
    """基本的な四則演算のパース（自動簡略化オフ）"""
    expr1 = parse("x + y", auto_simplify=False)
    assert expr1.to_string() == "x + y"
    
    expr2 = parse("2 * x - 3", auto_simplify=False)
    # "-3" は "+ (-3)" としてパースされ、表示は "- 3" になる
    assert expr2.to_string() == "2x - 3"


def test_implicit_multiplication():
    """暗黙の掛け算 (5x, 2pi, x sin(x) など)"""
    assert parse("5x", auto_simplify=False).to_string() == "5x"
    assert parse("2pi", auto_simplify=False).to_string() == "2\\pi"
    assert parse("(x+1)(x-2)", auto_simplify=False).to_string() == "\\left(x + 1\\right)\\left(x - 2\\right)"
    assert parse("x sin(x)", auto_simplify=False).to_string() == "x\\sin\\left(x\\right)"


def test_spaces_ignored():
    """スペースの無視"""
    expr1 = parse("x+y", auto_simplify=False)
    expr2 = parse(" x  +   y ", auto_simplify=False)
    assert expr1.to_string() == expr2.to_string() == "x + y"


def test_functions_and_constants():
    """関数と定数"""
    assert parse("sin(pi/2)", auto_simplify=False).to_string() == "\\sin\\left(\\frac{\\pi}{2}\\right)"
    assert parse("ln(e^x)", auto_simplify=False).to_string() == "\\ln\\left(e^{x}\\right)"
    assert parse("sqrt(2)", auto_simplify=False).to_string() == "\\sqrt{2}"
    assert parse("cbrt(x)", auto_simplify=False).to_string() == "\\sqrt[3]{x}"


def test_auto_simplify():
    """パース時の自動簡略化"""
    # 2x + 3x -> 5x にはまだ対応していないが、sin(pi/4) -> sqrt(2)/2 は通る
    expr = parse("sin(pi/4)") # デフォルトで auto_simplify=True
    assert expr.to_string() == "\\frac{\\sqrt{2}}{2}"
    
    # ln(e^2) -> 2
    assert parse("ln(e^2)").to_string() == "2"


def test_error_handling():
    """日本語エラーメッセージの確認"""
    # 不正な文字
    with pytest.raises(ParseError, match="不正な文字 '@'"):
        parse("x + @")
        
    # カッコの閉じ忘れ
    with pytest.raises(ParseError, match="カッコが正しく閉じられていません"):
        parse("(x + 1")
        
    # 余分な閉じカッコ
    with pytest.raises(ParseError, match="数式の末尾に余分な文字があります"):
        parse("x + 1)")
        
    # 関数のカッコ忘れ
    with pytest.raises(ParseError, match="カッコ '\\(' が必要です"):
        parse("sin x")


if __name__ == "__main__":
    test_basic_parsing()
    test_implicit_multiplication()
    test_spaces_ignored()
    test_functions_and_constants()
    test_auto_simplify()
    print("✅ フェーズ4: パーサーのテストに合格しました！")
