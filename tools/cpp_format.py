
import os
import sys
import re
import json

REDUNDENT_PATTERN = [
    r'// .*: interface for the .* class.\r?\n//\r?\n///+',
]

NT_NOT_SET = 0
NT_PREPROCESSOR_IF = 0
NT_STRUCT = 1

class Node:

    def __init__(self, typ, content):
        self.node_type = NT_NOT_SET
        self.children = []

class NodeStruct(Node):

    def __init__(self):
        self.node_type = NT_STRUCT
        self.children = []

TK_UNKOWN           = -1
TK_EOF              = 0
TK_ID               = 1
TK_NUMBER           = 2
TK_STRING           = 3
TK_ARROW            = 4 #->
TK_SPACE            = 5 # blank space
TK_NEW_LINE         = 6 # \r\n
TK_COMMENTS         = 7
TK_OP               = 8 # +-*/><=|&^%!?
TK_PREPROCESSOR     = ord('#')
TK_LITTLE           = ord('<')
TK_GREAT            = ord('>')
TK_L_PAREN          = ord('(')
TK_R_PAREN          = ord(')')
TK_L_BRACKET        = ord('[')
TK_R_BRACKET        = ord(']')
TK_L_BRACE          = ord('{')
TK_R_BRACE          = ord('}')
TK_SEMI_COLON       = ord(';')
TK_COLON            = ord(':')
TK_ASSIGN           = ord('=')
TK_DOT              = ord('.')
TK_COMMA            = ord(',')

CHARS_NUMBER = '0123456789'
CHARS_NUMBER_FOLLOWS = 'x0123456789ABCDEFabcdef'
CHARS_ID_STARTS = '_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz'
CHARS_ID_FOLLOWS = CHARS_ID_STARTS + CHARS_NUMBER
CHARS_OP = '+-*/|&^%!?~'
CHARS_OP_FOLLOWS = '+-=&!~'
CHARS_SPACE = ' \T'

def make_token_table():
    table = [TK_UNKOWN] * 256

    for c in CHARS_ID_STARTS:
        table[ord(c)] = TK_ID

    for c in CHARS_NUMBER:
        table[ord(c)] = TK_NUMBER

    for c in CHARS_OP:
        table[ord(c)] = TK_OP

    for c in CHARS_SPACE:
        table[ord(c)] = TK_SPACE

    for c in '"\'':
        table[ord(c)] = TK_STRING

    for c in '#()[]{};:=.,':
        table[ord(c)] = ord(c)

    return table

TOKEN_TABLE = make_token_table()

class CppLexer:

    def __init__(self, content):
        self._content = content
        self._pos = 0
        self._len = len(content)
        self._line_no = 1

    def next_token(self):
        if self._pos >= self._len:
            return TK_EOF, ''

        start = self._pos

        token = self._next_token()
        if self._pos == -1:
            self._pos = self._len

        text = self._content[start:self._pos]
        self._line_no += text.count('\n')

        return token, text

    def _next_token(self):
        c = self._content[self._pos]
        self._pos += 1

        if c == '/':
            if self._content[self._pos] == '/':
                # Line comments
                self._pos = self._content.find('\n', self._pos + 1)
                return TK_COMMENTS
            elif self._content[self._pos] == '*':
                # Block comments
                self._pos = self._content.find('*/', self._pos + 2)
                return TK_COMMENTS
        elif c == '-' and self._content[self._pos] == '>':
            self._pos += 1
            return TK_ARROW

        token = TOKEN_TABLE[ord(c)]
        if token == TK_ID:
            return self._read_follows(CHARS_ID_FOLLOWS, TK_ID)
        elif token == TK_SPACE:
            return self._read_follows(CHARS_SPACE, TK_SPACE)
        elif token == TK_STRING:
            return self._read_string(c)
        elif token == TK_NUMBER:
            return self._read_follows(CHARS_NUMBER_FOLLOWS, TK_NUMBER)
        elif token == TK_OP:
            return self._read_follows(CHARS_OP_FOLLOWS, TK_OP)
        elif token == TK_UNKOWN:
            raise Exception('Unkown token')

        while self._pos < self._len:
            pass

    def _read_string(self, quote):
        while self._pos < self._len:
            c = self._content[self._pos]
            self._pos += 1
            if c == '\\':
                # escape
                self._pos += 1
                continue
            elif c == quote:
                return
            elif c == '\n':
                raise Exception('New line is NOT expected in string')
        return TK_STRING

    def _read_follows(self, follow_chars, typ):
        while self._pos < self._len:
            c = self._content[self._pos]
            if c not in follow_chars:
                break
            self._pos += 1
        return typ

class CppParser:

    def __init__(self, content):
        self.node_type = NT_STRUCT
        self.children = []
        self._lexer = CppLexer(content)

        self._parse()

    def parse(self):
        self._root = NodeProgram()
        while self._expect_any(self._root):
            pass

    def _expect_any(self, block):
        token, tk_str = self._lexer.next_token()
        if token == TK_PREPROCESSOR:
            node = self._expect_preprocessor()
        elif token == TK_ID:
            if tk_str == 'class' or tk_str == 'struct':
                node = self._expect_struct()
            elif tk_str == 'enum':
                node = self._expect_enum()
            else:
                node = Node()
                node.children.append((token, tk_str))
                tk_next, tk_str_next = self._lexer.next_token()
                while tk_next is not TK_EOF:
                    node.children.append((tk_next, tk_str_next))
                    if tk_next == TK_L_PAREN:
                        node.children.append(self._expect_pair(TK_R_PAREN))
                    elif tk_next == TK_L_BRACKET:
                        node.children.append(self._expect_pair(TK_R_BRACKET))
                    elif tk_next == TK_L_BRACE:
                        node.children.append(self._expect_pair(TK_R_BRACE))
                    elif tk_next == TK_SEMI_COLON:
                        break
            

    def _expect_pair(self, expected_end_pair_token):
        pass

    def _expect_struct(self):
        pass

    def _expect_enum(self):
        pass


def parse_cpp_content(content):
    '''
    将 content 解析为 Node 数组
    '''
    pass

def format_file(fn):
    pass

def format_in_dir(path):
    count = 0
    for root, _, files in os.walk(path):
        for name in files:
            ext = os.path.splitext(name)[1]
            if ext not in ['.h', '.cpp', '.mm']:
                continue
            fn = os.path.join(root, name)
            if format_in_dir(fn):
                count += 1
                print('{} replaced: {}'.format(count, fn))

    print('Total {0} files were replaced.'.format(count))


if __name__ == '__main__':
    if len(sys.argv) != 2:
        print('{} input_dir'.format(os.path.basename(__file__)))
        sys.exit(1)

    format_in_dir(sys.argv[1])

