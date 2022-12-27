# encoding=utf-8

import os
import sys
import re
import json

REDUNDENT_PATTERN = [
    r'// .*: interface for the .* class.\r?\n//\r?\n///+',
]

INDENT = '    '

LT_NOT_CARE = 0
LT_CLASS_STRUCT_DECLARE_START = 1
LT_ENUM_DECLARE_START = 2
LT_DEFINE = 3
LT_COMMENTS_C = 4
LT_PREPROCESSOR = 5
LT_IF = 6
LT_ELSE = 7
LT_LOOP = 8
LT_DO = 9

class LineFeeder():

    def __init__(self, lines):
        self._lines = lines
        self._pos = 0

    def next(self):
        if self._pos >= len(self._lines):
            return None

        line = self._lines[self._pos]
        line = replace_tab_to_sapce(line)
        self._pos += 1
        return Line(line)

    def back(self):
        self._pos -= 1

def count_braces(text):
    return text.count('{'), text.count('}')


class Line():

    def __init__(self, text):
        self.text_org_unstriped = text.strip('\r')
        self.text = self.text_org = text = text.strip()
        self.open_braces = 0
        self.close_braces = 0
        self.open_parens = 0
        self.close_parens = 0
        self.open_brackets = 0
        self.close_brackets = 0
        self.ending_comments = False
        self.count_eq = 0
        self.count_semi_colon = 0
        self.type = LT_NOT_CARE

        i = 0
        count = len(text)
        while i < count:
            c = text[i]
            i += 1

            if c == '\'' or c == '"':
                ending = c
                while i < count:
                    if text[i] == '\\':
                        i += 2
                    elif text[i] == ending:
                        i += 1
                        break
                    else:
                        i += 1
            elif c == '/' and i < count:
                if text[i] == '/':
                    # comments
                    self.text = text[:i - 1].strip()
                    self.ending_comments = True
                    break
                elif text[i] == '*':
                    # /*
                    self.text = text[:i - 1].strip()
                    p = text.find('*/', i + 1)
                    if p == -1:
                        self.type = LT_COMMENTS_C
                    else:
                        i = p + 2
                    break
            elif c == '{':
                self.open_braces += 1
            elif c == '}':
                self.close_braces += 1
            elif c == '(':
                self.open_parens += 1
            elif c == ')':
                self.close_parens += 1
            elif c == '[':
                self.open_brackets += 1
            elif c == ']':
                self.close_brackets += 1
            elif c == '=':
                self.count_eq += 1
            elif c == ';':
                self.count_semi_colon += 1

        w1, _, _ = text.partition(' ')
        self.word1 = w1
        if w1 in ('class', 'struct'):
            self.type = LT_CLASS_STRUCT_DECLARE_START
        elif w1 == 'enum':
            self.type = LT_ENUM_DECLARE_START
        elif w1 in ('#include', 'typedef', 'using', 'friend'):
            self.type = LT_NOT_CARE
        elif w1 == '#define':
            self.type = LT_DEFINE
        elif w1.startswith('#'):
            self.type = LT_PREPROCESSOR
        elif w1 == 'if':
            self.type = LT_IF
        elif w1 == 'do':
            self.type = LT_DO
        elif w1 == 'else':
            self.type = LT_ELSE
        elif w1 in ('while', 'for'):
            self.type = LT_LOOP

        self.last_char = self.text[-1] if self.text else ''
        self.matches = self.open_braces + self.open_brackets + self.open_parens - self.close_braces - self.close_brackets - self.close_parens

    def is_indent_temp(self):
        if self.last_char in ';{}':
            return False

        if self.word1.startswith('#') or self.word1.startswith('template') or self.word1 in ('typedef'):
            return False

        return True

def parse_block(line_feeder):
    line = line_feeder.next()

def remove_extra_spaces(text):
    a = []
    i = 0
    count = len(text)
    is_prev_space = False
    while i < count:
        c = text[i]
        i += 1

        if c == '\'' or c == '"':
            start = i - 1
            ending = c
            while i < count:
                if text[i] == '\\':
                    i += 2
                elif text[i] == ending:
                    i += 1
                    break
                else:
                    i += 1
            a.append(text[start:i])
            is_prev_space = False
        elif c == ' ' or c == '\t':
            if not is_prev_space:
                a.append(' ')
                is_prev_space = True
        else:
            a.append(c)
            is_prev_space = False
    return ''.join(a)

def replace_tab_to_sapce(text):
    a = []
    i = 0
    count = len(text)
    while i < count:
        c = text[i]
        i += 1

        if c == '\'' or c == '"':
            start = i - 1
            ending = c
            while i < count:
                if text[i] == '\\':
                    i += 2
                elif text[i] == ending:
                    i += 1
                    break
                else:
                    i += 1
            a.append(text[start:i])
        elif c == '\t':
            a.append('    ')
        else:
            a.append(c)
    return ''.join(a)

class CppFormat():

    def __init__(self):
        self._out_lines = []
        self._indent = 0
        self._var_padding = 28
        self._change_padding = True
        self._stack = []
        self._is_in_class = False
        self._is_next_in_class = False
        self._is_indent_tmp = False

    def _write_line(self, line):
        if self._change_padding:
            self._var_padding = 28
        else:
            self._change_padding = True

        if line.text[0] == '}':
            # 减小缩进
            self._indent -= 1
            self._is_in_class = self._stack[-1]
            self._stack.pop()

        # 写入当前行
        assert type(line) is Line
        text = line.text_org.rstrip()
        if text:
            self._out_lines.append(INDENT * (self._indent + self._is_indent_tmp) + text)
        else:
            self._out_lines.append('')

        #
        # 影响之后的写入行为
        #
        if line.last_char != ';' and line.word1 in ('class', 'struct'):
            self._is_next_in_class

        if line.last_char == '{':
            # 增加缩进
            self._indent += 1
            self._stack.append(self._is_in_class)
            self._is_in_class = self._is_next_in_class

        if line.is_indent_temp():
            self._is_indent_tmp = False
        else:
            # 需要临时 indent
            self._is_indent_tmp = True

    def _append_line(self, line):
        if self._change_padding:
            self._var_padding = 28
        else:
            self._change_padding = True

        if type(line) is Line:
            line = line.text_org
        if line.strip():
            self._out_lines.append(line)
        else:
            self._out_lines.append('')

    def format_program(self, line_feeder):
        while True:
            line = line_feeder.next()
            if line is None:
                break

            self._write_line(line)

            if line.word1 == 'if':
                self._format_one_stmt_to_block(line, line_feeder)


    def _format_one_stmt_to_block(self, line, line_feeder):
        # if, else, for, while
        if line.last_char in ';}':
            # 单行的
            self._append_line(line)
            return

        if line.last_char == '{':
            # 当前行是 block，不用转换，缺省处理即可
            self._write_line(line)
            return

        self._write_line(line)

        matches = line.matches
        while matches != 0:
            # 匹配到结束
            assert line.open_braces + line.close_braces == 0
            line = line_feeder.next()
            if line.last_char == '{':
                # 当前行是 block，不用转换，缺省处理即可
                self._write_line(line)
                return

            self._is_indent_tmp = True
            self._write_line(line)
            matches += line.matches

        line = line_feeder.next()
        if line.last_char == '{':
            # 当前行是 block，不用转换，缺省处理即可
            assert line.text == '{'
            self._write_line(line)
            return

        # 添加 '{}'
        assert line.text == line.text_org
        line.text += ' {'
        line.text_org = line.text
        assert not self._is_in_class
        self._write_line(line)
        self._indent -= 1
        self._write_line()


    def _write_until_open_braces(self, line_feeder):
        while True:
            line = line_feeder.next()
            if line.last_char == '}':
                # 单行的 body？
                self._write_line(INDENT + line.text_org)
                return True
            elif line.last_char == '{':
                if line.text == '{':
                    assert line.text == line.text_org
                    self._out_lines[-1] += ' {'
                else:
                    self._write_line(INDENT + line.text_org)
                return False
            else:
                self._write_line(INDENT + line.text_org)

    def format_program1(self, line_feeder):
        self._indent = -1
        self._format_body(line_feeder)
        return '\n'.join(self._out_lines)

    def _format_body(self, line_feeder, is_class_body=False):
        self._indent += 1

        while True:
            line = line_feeder.next()
            if line is None:
                break
            elif line.text.startswith('}'):
                # end of body
                break

            if line.type == LT_CLASS_STRUCT_DECLARE_START:
                if line.last_char != ';':
                    self._brace_to_up = True
                    self._next_in_class = True
                if line.last_char != '{':
                    self._write_until_open_braces(line_feeder)
            elif line.type == LT_ENUM_DECLARE_START:
                self._format_enum(line, line_feeder)
            elif line.type == LT_DEFINE:
                self._format_define(line, line_feeder)
            elif line.type == LT_COMMENTS_C:
                self._format_comments_c(line, line_feeder)
            elif line.text in ('public:', 'protected:', 'private:'):
                assert line.text == line.text_org
                self._append_line(INDENT * (self._indent - 1) + line.text)
            elif line.last_char == ':':
                if self._indent > 0:
                    self._append_line(INDENT * (self._indent - 1) + line.text_org)
                else:
                    self._write_line(line.text_org)
            elif line.text.endswith(': {'):
                if self._indent > 0:
                    self._append_line(INDENT * (self._indent - 1) + line.text_org)
                else:
                    self._write_line(line.text_org)
                self._format_body(line_feeder)
            elif line.text == '{':
                # Block 的开始
                self._write_line(line)
                self._format_body(line_feeder)
            elif line.type == LT_PREPROCESSOR:
                self._append_line(line.text_org)
            elif line.type == LT_IF or line.type == LT_DO:
                self._format_block_brace(line, line_feeder)
            elif line.type == LT_LOOP:
                self._format_block_brace(line, line_feeder)
            elif line.type == LT_ELSE:
                if self._out_lines[-1].endswith('}'):
                    # 将 else 合并到上一行
                    assert self._out_lines[-1].strip() == '}'
                    self._out_lines.pop()
                    line.text_org = '} ' + line.text_org
                self._format_block_brace(line, line_feeder)
            elif line.last_char == ';':
                if is_class_body and line.open_parens == 0:
                    # 成员变量?
                    self._format_member_vars(line)
                else:
                    # 全局声明之类的
                    if line.matches != 0:
                        print(line.text_org)
                    assert line.matches == 0
                    self._write_line(remove_extra_spaces(line.text_org))
            elif line.text == '':
                if line.text_org:
                    self._write_line(line.text_org)
                else:
                    self._append_line('')
            elif line.open_braces > 0:
                if line.matches == 0:
                    # 单行的声明
                    self._write_line(line)
                else:
                    # 函数声明
                    self._format_function(line, line_feeder)
            elif line.open_parens > 0:
                # 函数声明，或者函数调用，未确定.
                self._write_line(line)

                line = self._format_till_paren_matches(line, line_feeder)
                if line.last_char == '{':
                    # 函数
                    self._format_body(line_feeder)
                elif line.last_char == ';':
                    # 语句，并且已经结束
                    pass
                else:
                    # 是函数的 body?
                    line = line_feeder.next()
                    if line.text == '{':
                        self._out_lines[-1] += ' ' + line.text_org
                        self._format_body(line_feeder)
                    else:
                        line_feeder.back()
            elif line.count_eq > 0:
                self._write_line(remove_extra_spaces(line.text_org))
            elif line.type == LT_NOT_CARE:
                self._append_line(line.text_org_unstriped)
            else:
                # 不认识的
                print(line)
                assert 0

        self._indent -= 1
        if line:
            if line.text.endswith(' else'):
                tmp = Line('else')
                tmp.text = line.text
                tmp.text_org = line.text_org
                tmp.text_org_unstriped = line.text_org_unstriped
                self._format_block_brace(tmp, line_feeder)
                return

            self._write_line(line)

            if line.last_char == '{':
                self._format_body(line_feeder)

    def _format_class(self, line, line_feeder):
        self._write_line(line)
        if line.last_char == ';':
            return

        if not line.last_char == '{':
            if self._write_until_open_braces(line_feeder):
                return

        # member 的开始
        self._format_body(line_feeder, True)

    def _format_member_vars(self, line):
        line = line.text_org;
        if line.startswith('typedef') or line.startswith('using') or line.startswith('friend') or line.startswith('class' or line.startswith('struct') or line.startswith('enum')):
            self._write_line(line)
            return

        line, b, c = line.partition('//')
        line = line.strip()
        comments = (b + ' ' + c.strip()).strip()

        # 查找: int  *a[10], b;
        a = list(filter(lambda x: x != -1, [line.find(','), line.find('['), line.find('*')]))
        pos = len(line)
        if a:
            pos = min(a)

        left, _space, right = line[:pos].rpartition(' ')

        right = right.strip() + line[pos:]
        if comments:
            right = '{:19} '.format(right) + comments

        self._write_line(self._padding_width(left.strip(), right.strip()))

    def _format_enum(self, line, line_feeder):
        self._write_line(line)
        if line.last_char == ';':
            return

        if not line.last_char == '{':
            if self._write_until_open_braces(line_feeder):
                # 单行 body
                return

        self._indent += 1

        while True:
            line = line_feeder.next()
            if line.text == '};':
                break
            else:
                # 格式 a = 1,
                line = self._format_line_with_width(line.text_org, '=')
                self._write_line(line)

        self._indent -= 1
        self._write_line('};')

    def _format_function(self, line, line_feeder):
        self._write_line(line)
        if not line.last_char == '{':
            if self._write_until_open_braces(line_feeder):
                # 单行 body
                return

        self._format_body(line_feeder)

    def _format_block_brace(self, line, line_feeder):
        # 从 if, while, for 等进入.
        # 如果不带 '{ }'，则自点添加
        self._write_line(line)

        if line.last_char in ';}':
            # 已经结束
            return

        if line.last_char == '{':
            self._format_body(line_feeder)
        else:
            # 上一行不带 '{'
            if line.matches == 0:
                # 括号配对了，后面是带/不带 '{' 的语句
                line = line_feeder.next()
                if line.text.startswith('{'):
                    # 带 '{'
                    assert line.text == '{'
                    self._out_lines[-1] += ' ' + line.text_org
                    self._format_body(line_feeder)
                else:
                    # 不带 '{'
                    self._out_lines[-1] += ' {'
                    self._format_till_stmt_ends(line, line_feeder)
                    self._write_line('}')
            else:
                # 括号未配对，需要匹配到配对的括号，或者 '{'
                line = self._format_till_paren_matches(line, line_feeder)

                if line.last_char == '{':
                    # 匹配到了 '{'
                    self._format_body(line_feeder)
                else:
                    # 读取下一行，就是 '{'
                    line = line_feeder.next()
                    if line.text == '{':
                        assert line.text == line.text_org
                        self._out_lines[-1] += ' {'

                        self._format_body(line_feeder)
                    else:
                        # 不带 { 的语句，需要添加
                        self._out_lines[-1] += ' {'
                        self._format_till_stmt_ends(line, line_feeder)
                        self._write_line('}')

    def _format_till_stmt_ends(self, line, line_feeder):
        self._indent += 1
        self._write_line(line)

        matches = line.matches
        while matches != 0 and line.last_char != ';':
            line = line_feeder.next()
            self._write_line(INDENT + line.text_org)
            matches += line.matches

        assert line.count_semi_colon <= 1
        self._indent -= 1

    def _format_till_paren_matches(self, line, line_feeder):
        matches = line.matches
        while matches:
            line = line_feeder.next()
            self._write_line(INDENT + line.text_org)
            matches += line.matches
            if line.last_char == '{':
                break
        return line

    def _format_define(self, line, line_feeder):
        line = line.text_org
        if line.endswith('\\'):
            # 多行的 define
            self._append_line(line)
            while True:
                line = line_feeder.next().text_org
                self._append_line(line)
                if not line.endswith('\\'):
                    return

        a, b, c = line.partition(')')
        if b:
            # 函数式的 define
            left = a + b
            right = c.strip()
        else:
            a, b, c = line.rpartition(' ')
            if a.strip() == '#define':
                left = line
                right = None
            else:
                left = a.strip()
                right = c.strip()

        if right:
            self._append_line(self._padding_width(left, right))
        else:
            self._append_line(left)

    def _format_comments_c(self, line, line_feeder):
        self._write_line(line)
        while True:
            line = line_feeder.next()
            self._append_line(line.text_org_unstriped)
            if line.text_org.endswith('*/'):
                break

    def _format_line_with_width(self, line, partition_orders):
        '''
        先将注释分开，再按照 partition_orders 来分割
        '''
        a, b, c = line.partition('//')
        right = (' ' + b + ' ' + c.strip()).strip()
        a = a.strip()

        for p in partition_orders:
            a, b, c = a.partition(p)
            right = b + c + right

        if right.startswith(partition_orders[-1]):
            return self._padding_width(a, right)
        return a + right

    def _padding_width(self, left, right):
        self._change_padding = False

        left = left.strip()
        if len(left) > self._var_padding:
            self._var_padding = len(left) + 3
            self._var_padding -= self._var_padding % 4

        return ('{:%s} ' % (self._var_padding - 1)).format(left) + right.strip()

def replace_afx_header_define(text, fn, root_dir):
    pt = r'\bAFX_[\w\d_]+_INCLUDED_\b'
    name = os.path.abspath(fn)[len(os.path.abspath(root_dir)) + 1:]
    HEADER_DEF = name.replace(os.path.sep, '_').replace('.', '_').replace('-', '_')
    text = re.sub(pt, HEADER_DEF, text)
    return text.replace('#if !defined(' + HEADER_DEF + ')', '#ifndef ' + HEADER_DEF)

def format_cpp_content(content):
    '''
    将 content 解析为 Lines 数组
    '''
    lines = content.split('\n')

    line_feeder = LineFeeder(lines)
    formater = CppFormat()
    return formater.format_program(line_feeder)

def to_out_fn(fn, in_dir, out_dir):
    assert fn.startswith(in_dir)
    if not in_dir.endswith(os.path.sep):
        in_dir = in_dir + os.path.sep

    relative_name = fn[len(in_dir):]

    return os.path.join(out_dir, relative_name)

def make_sure_dir_exists(fn):
    path = os.path.dirname(fn)
    if os.path.exists(path):
        return
    os.makedirs(path)

def format_file(fn, out_fn, root_dir):
    with open(fn, 'rb') as fp:
        data = fp.read().decode('utf-8')
        data = replace_afx_header_define(data, fn, root_dir)
        content = format_cpp_content(data)

    if data == content:
        return False

    make_sure_dir_exists(out_fn)
    with open(out_fn, 'wb') as fp:
        fp.write(content.encode('utf-8'))

    return True

def format_in_dir(path, out_dir):
    count = 0
    for root, _, files in os.walk(path):
        for name in files:
            ext = os.path.splitext(name)[1]
            if ext not in ['.h', '.cpp', '.mm']:
                continue

            fn = os.path.join(root, name)
            out_fn = to_out_fn(fn, path, out_dir)
            print('{} formating: {}'.format(count, fn))
            if format_file(fn, out_fn, path):
                count += 1
                print('{} replaced: {}'.format(count, fn))

    print('Total {0} files were replaced.'.format(count))


if __name__ == '__main__':
    if len(sys.argv) != 2:
        print('{} input_dir'.format(os.path.basename(__file__)))
        sys.exit(1)

    in_dir = sys.argv[1]
    in_dir = os.path.abspath(in_dir)

    if os.path.isfile(in_dir):
        # 文件
        name, ext = os.path.splitext(in_dir)
        format_file(in_dir, name + '_formated' + ext, os.path.dirname(in_dir))
    else:
        # 目录
        out_dir = in_dir + '_out'
        format_in_dir(in_dir, out_dir)

