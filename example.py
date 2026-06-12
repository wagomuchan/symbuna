import symbuna

def main():
    print("=== 🚀 SymBuna サンプルコード ===")
    print("数式がパースされ、自動的に簡略化されて出力されます！\n")

    # 1. 暗黙の掛け算と同類項のまとめ
    expr1 = symbuna.parse("2x + 3x")
    print("入力: 2x + 3x")
    print(f"出力 (LaTeX形式): {expr1}")
    print("-" * 30)

    # 2. 三角関数の特殊角の計算
    expr2 = symbuna.parse("sin(pi/4)")
    print("入力: sin(pi/4)")
    print(f"出力 (LaTeX形式): {expr2}")
    print("-" * 30)

    # 3. 分母の自動有理化
    expr3 = symbuna.parse("1 / sqrt(2)")
    print("入力: 1 / sqrt(2)")
    print(f"出力 (LaTeX形式): {expr3}")
    print("-" * 30)

    # 4. 対数の法則と立方根
    expr4 = symbuna.parse("ln(e^5)")
    print("入力: ln(e^5)")
    print(f"出力 (LaTeX形式): {expr4}")
    print("-" * 30)

    expr5 = symbuna.parse("cbrt(27)")
    print("入力: cbrt(27)")
    print(f"出力 (LaTeX形式): {expr5}")
    print("-" * 30)
    
    print("\n※ Jupyter Notebookを使うと、これらのLaTeX文字列が美しい数式として画面に描画されます！")

if __name__ == "__main__":
    main()
