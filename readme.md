# SymBuna

**SymBuna** は、C++による高速なバックエンドと、Pythonによる直感的なフロントエンドを融合させた「高校数Ⅲレベル」に特化した軽量・高速な数式処理ライブラリ（Symbolic Engine）です。

## 特徴 (Features)
- **直感的な人間スタイル入力**: `5x` や `2pi` といった省略された掛け算や、`sin(pi/4)` などの直感的な入力を自動でパースします。
- **自動簡略化**: 分母の有理化（`1/sqrt(2) -> sqrt(2)/2`）や、三角関数の特殊角、対数法則などを自動で綺麗に計算・整理します。
- **Jupyter Notebook 対応**: 自動で美しい LaTeX 形式（$\LaTeX$）で数式がレンダリングされます。
- **高速性**: コアな AST（抽象構文木）と簡略化エンジンはすべて C++17 で書かれており、Python 側との連携は `pybind11` を通じて極めて低オーバーヘッドで行われます。

## インストール (Installation)

ソースコードをクローンし、`pip` で開発モードとしてインストールします。
（※ インストールには C++17 対応のコンパイラと pybind11 が必要です）

```bash
git clone https://github.com/wagomuchan/symbuna.git
cd symbuna
pip install pybind11
pip install -e .
```

## 使い方 (Usage)

### 基本的なパースと表示

`symbuna.parse` に文字列を渡すだけで、自動的にパースと簡略化が行われます。

```python
import symbuna

# 1. 自動簡略化と暗黙の掛け算のパース
expr = symbuna.parse("2x + 3x")
print(expr)  # 出力: 5 \cdot x

# 2. 三角関数と特殊角
expr2 = symbuna.parse("sin(pi/4)")
print(expr2) # 出力: \frac{\sqrt{2}}{2}

# 3. 有理化と立方根
expr3 = symbuna.parse("1 / sqrt(2)")
print(expr3) # 出力: \frac{\sqrt{2}}{2}

expr4 = symbuna.parse("cbrt(8)")
print(expr4) # 出力: 2
```

### Jupyter Notebook での利用

Jupyter環境（JupyterLab, VSCode, Google Colab等）では、変数をそのまま評価するだけで、美しく数式が描画されます。

```python
# Jupyter セル内
symbuna.parse("(-b + sqrt(b^2 - 4a c)) / (2a)")
```
*(上記を実行すると、二次方程式の解の公式が LaTeX 形式で描画されます)*

## サポートしている記号・関数

- **定数**: `pi` (円周率), `e` (ネイピア数)
- **四則演算・べき乗**: `+`, `-`, `*`, `/`, `^`
- **関数**: 
  - `sqrt` (平方根), `cbrt` (立方根)
  - `sin`, `cos`, `tan` (三角関数)
  - `log`, `ln` (自然対数)

## テストの実行

```bash
pip install pytest
python -m pytest tests/ -v
```

## ライセンス (License)

MIT License
