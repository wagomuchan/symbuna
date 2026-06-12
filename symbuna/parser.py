import re
from . import (
    Expr, Int, Var, Neg, Add, Mul, Div, Pow, Sqrt,
    Pi, E, Cbrt, Sin, Cos, Tan, Log, simplify
)

class ParseError(ValueError):
    """構文エラーが発生したときに投げる例外"""
    pass

def tokenize(text: str):
    """
    文字列をトークン（単語や記号）のリストに分割する。
    スペースは無視する。
    """
    # 認識するパターンの正規表現
    token_specification = [
        ('NUMBER',   r'\d+'),
        ('FUNC',     r'(?<![a-zA-Z_])(sqrt|cbrt|sin|cos|tan|log|ln)\b'),
        ('CONST',    r'(?<![a-zA-Z_])(pi|e)\b'),
        ('VAR',      r'[a-zA-Z_]\w*'),
        ('OP',       r'[+\-*/^]'),
        ('LPAREN',   r'\('),
        ('RPAREN',   r'\)'),
        ('SKIP',     r'[ \t]+'),
        ('MISMATCH', r'.'),
    ]
    tok_regex = '|'.join('(?P<%s>%s)' % pair for pair in token_specification)
    
    tokens = []
    for mo in re.finditer(tok_regex, text):
        kind = mo.lastgroup
        value = mo.group()
        if kind == 'SKIP':
            continue
        elif kind == 'MISMATCH':
            raise ParseError(f"エラー: 不正な文字 '{value}' が検出されました。")
        tokens.append((kind, value))
    return tokens

class Parser:
    """
    再帰的降下パーサー
    文法:
      Expr   -> Term (('+' | '-') Term)*
      Term   -> Factor (('*' | '/') Factor | Factor)*   (★並置による暗黙の掛け算をサポート)
      Factor -> Power ('^' Power)*
      Power  -> '-' Power | Primary
      Primary-> NUMBER | CONST | VAR | FUNC '(' Expr ')' | '(' Expr ')'
    """
    def __init__(self, tokens):
        self.tokens = tokens
        self.pos = 0

    def current(self):
        return self.tokens[self.pos] if self.pos < len(self.tokens) else ('EOF', '')

    def consume(self, expected_kind):
        kind, value = self.current()
        if kind == expected_kind:
            self.pos += 1
            return value
        raise ParseError(f"エラー: '{expected_kind}' が期待されましたが、'{value}' が見つかりました。")

    def parse(self):
        expr = self.parse_expr()
        if self.current()[0] != 'EOF':
            raise ParseError("エラー: 数式の末尾に余分な文字があります。カッコが正しく閉じられていない可能性があります。")
        return expr

    def parse_expr(self):
        node = self.parse_term()
        while self.current()[0] == 'OP' and self.current()[1] in ('+', '-'):
            op = self.consume('OP')
            right = self.parse_term()
            if op == '+':
                node = Add(node, right)
            else:
                node = Add(node, Neg(right)) # 引き算は Add(a, Neg(b))
        return node

    def parse_term(self):
        node = self.parse_factor()
        
        while True:
            kind, value = self.current()
            
            # 明示的な掛け算・割り算
            if kind == 'OP' and value in ('*', '/'):
                op = self.consume('OP')
                right = self.parse_factor()
                if op == '*':
                    node = Mul(node, right)
                else:
                    node = Div(node, right)
                continue
            
            # ★暗黙の掛け算 (例: 5x, 2pi, (x+1)(x-2), x sin(x))
            # 次のトークンが Primary の始まりになり得るなら、掛け算とみなす
            if kind in ('NUMBER', 'CONST', 'VAR', 'FUNC', 'LPAREN'):
                right = self.parse_factor()
                node = Mul(node, right)
                continue
            
            break
            
        return node

    def parse_factor(self):
        node = self.parse_power()
        while self.current()[0] == 'OP' and self.current()[1] == '^':
            self.consume('OP')
            right = self.parse_power()
            node = Pow(node, right)
        return node

    def parse_power(self):
        kind, value = self.current()
        # 単項マイナス
        if kind == 'OP' and value == '-':
            self.consume('OP')
            return Neg(self.parse_power())
        # 単項プラス (無視する)
        if kind == 'OP' and value == '+':
            self.consume('OP')
            return self.parse_power()
            
        return self.parse_primary()

    def parse_primary(self):
        kind, value = self.current()
        
        if kind == 'NUMBER':
            self.consume('NUMBER')
            return Int(int(value))
            
        elif kind == 'CONST':
            self.consume('CONST')
            if value == 'pi': return Pi()
            if value == 'e':  return E()
            
        elif kind == 'VAR':
            self.consume('VAR')
            return Var(value)
            
        elif kind == 'FUNC':
            func_name = self.consume('FUNC')
            if self.current()[0] != 'LPAREN':
                raise ParseError(f"エラー: 関数 '{func_name}' の後にはカッコ '(' が必要です。")
            self.consume('LPAREN')
            arg = self.parse_expr()
            self.consume('RPAREN')
            
            if func_name == 'sqrt': return Sqrt(arg)
            if func_name == 'cbrt': return Cbrt(arg)
            if func_name == 'sin':  return Sin(arg)
            if func_name == 'cos':  return Cos(arg)
            if func_name == 'tan':  return Tan(arg)
            if func_name in ('log', 'ln'): return Log(arg)
            
        elif kind == 'LPAREN':
            self.consume('LPAREN')
            node = self.parse_expr()
            if self.current()[0] != 'RPAREN':
                raise ParseError("エラー: カッコが正しく閉じられていません。')' が必要です。")
            self.consume('RPAREN')
            return node
            
        raise ParseError(f"エラー: 数式が不正です。解釈できないトークン '{value}' が見つかりました。")

def parse(expression_str: str, auto_simplify: bool = True):
    """
    文字列をパースしてASTを返す。
    auto_simplify が True なら自動的に簡略化を行う。
    """
    if not expression_str.strip():
        raise ParseError("エラー: 数式が空です。")
        
    tokens = tokenize(expression_str)
    parser = Parser(tokens)
    ast = parser.parse()
    
    if auto_simplify:
        return simplify(ast)
    return ast
