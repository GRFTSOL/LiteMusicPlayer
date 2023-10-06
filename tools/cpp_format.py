# encoding=utf-8

import os
import sys
import re
import json


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

class LineFeeder(object):

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


class Line(object):

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
                        break
                    else:
                        i = len(self.text)
                        self.text = text = self.text + text[p + 2:]
                        count = len(text)
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

        w1 = text.partition(' ')[0].partition('(')[0]
        if w1 in ('class', 'struct', 'interface', 'union'):
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
        self.open_paren_brackets = self.open_parens + self.open_brackets
        self.close_paren_brackets = self.close_parens + self.close_brackets

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

    def _write_line(self, line):
        if self._change_padding:
            self._var_padding = 28
        else:
            self._change_padding = True

        if type(line) is Line:
            line = line.text_org
        line = line.rstrip()
        if line:
            self._out_lines.append(INDENT * self._indent + line)
        else:
            self._out_lines.append('')

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

    def _append_last_line(self, text):
        if '//' in self._out_lines[-1]:
            line = self._out_lines[-1]
            assert('"' not in line)
            a, b, c = line.partition('//')
            self._out_lines[-1] = a.rstrip() + text + ' // ' + c.strip()
        else:
            self._out_lines[-1] += text

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
                    self._append_last_line(' {')
                else:
                    self._write_line(INDENT + line.text_org)
                return False
            else:
                self._write_line(INDENT + line.text_org)

    def format_program(self, line_feeder):
        self._indent = -1
        self._format_body(line_feeder)

        assert line_feeder._pos == len(line_feeder._lines)

        verify_pair_matches(self._out_lines)

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
                self._format_class(line, line_feeder)
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
            elif line.open_paren_brackets > 0:
                # 函数声明，或者函数调用，未确定.
                self._format_open_paren_brackets(line, line_feeder)
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
            elif line.open_paren_brackets > 0:
                # 函数声明，或者函数调用，未确定.
                self._out_lines.pop() # 最后一行已经写入，需要弹出.
                self._format_open_paren_brackets(line, line_feeder)

    def _format_open_paren_brackets(self, line, line_feeder):
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
                self._append_last_line(' ' + line.text_org)
                self._format_body(line_feeder)
            elif line.text.startswith('{') and line.last_char == '}':
                # 单行
                self._write_line(INDENT + line.text_org)
            else:
                line_feeder.back()

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
        if line.startswith('typedef') or line.startswith('using') or line.startswith('friend') or line.startswith('class' or \
                line.startswith('struct') or line.startswith('union') or line.startswith('interface') or line.startswith('enum')):
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
        if not left:
            # 未识别的格式，不转换
            self._write_line(line)
            return

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
                line, _, comments = line.text_org.partition('//')
                line = self._format_line_with_width(line.strip(), '=')
                if comments:
                    if line:
                        if len(line + comments) < 80:
                            line = '{:32} // {}'.format(line, comments.strip())
                        else:
                            line = line + ' // ' + comments.strip()
                    else:
                        line = '// ' + comments.strip()
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
                    self._append_last_line(' ' + line.text_org)
                    self._format_body(line_feeder)
                else:
                    # 不带 '{'
                    self._append_last_line(' {')
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
                        self._append_last_line(' {')

                        self._format_body(line_feeder)
                    else:
                        # 不带 { 的语句，需要添加
                        self._append_last_line(' {')
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
        matches = line.open_paren_brackets - line.close_paren_brackets
        while matches:
            line = line_feeder.next()
            self._write_line(INDENT + line.text_org)
            matches += line.open_paren_brackets - line.close_paren_brackets
            if line.last_char == '{':
                break
        return line

    def _format_define(self, line, line_feeder):
        if line.text != line.text_org or line.text == '#define':
            # 有 comments，不格式化
            self._append_line(line.text_org)
            return

        line = line.text_org
        if line.endswith('\\'):
            # 多行的 define
            self._append_line(line)
            while True:
                line = line_feeder.next().text_org_unstriped
                self._append_line(line)
                if not line.endswith('\\'):
                    return

        a, b, c = line.partition(')')
        if b:
            # 函数式的 define
            left = a + b
            right = c.strip()
        else:
            # 字符串?
            line, b, c = line.partition('"')
            right = b + c

            a, b, c = line.rpartition(' ')
            if a.strip() == '#define':
                # 类似: #define abc
                assert not right
                left = line
                right = None
            else:
                left = a.strip()
                right = (c + right).strip()

        if right:
            self._append_line(self._padding_width(left.strip(), right))
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

def remove_afx_comments(data):
    pt = r'// .*?: \w+ \w+ the .* class.\r?\n//\r?\n///+'
    data = re.sub(pt, '', data).strip() + '\n'

    pt = r'#if _MSC_VER \> 1000\r?\n#pragma once\r?\n#endif // _MSC_VER \> 1000\r?\n'
    data = re.sub(pt, '', data).strip() + '\n'

    pt = r'/+\r?\n// Construction/Destruction\r?\n/+'
    data = re.sub(pt, '', data).strip() + '\n'

    return data

def patch_paragma_once(content):
    if '#pragma once' not in content:
        content = '#pragma once\n\n' + content

    return content

def check_include_space_lines(content):
    n = content.rfind('#include')
    if n == -1:
        return content

    n = content.find('\n', n)
    assert n != -1
    content = content[:n].strip() + '\n\n\n' + content[n:].strip()
    return content.strip() + '\n'

def verify_pair_matches(lines, is_print=False):
    i = 0
    matches = 0
    while i < len(lines):
        text = lines[i]
        i += 1
        line = Line(text)
        matches += line.matches
        if is_print:
            print(i, matches, text)

        if line.type == LT_COMMENTS_C:
            while True:
                text = lines[i]
                i += 1
                a, b, c = text.partition('*/')
                if b:
                    line = Line(c)
                    matches += line.matches
                    break

    if matches != 0 and not is_print:
        verify_pair_matches(lines, True)
        assert(0)

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
    work_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
    if root_dir.startswith(work_path):
        root_dir = work_path

    with open(fn, 'rb') as fp:
        data = fp.read().decode('utf-8')
        data = replace_afx_header_define(data, fn, root_dir)
        data = remove_afx_comments(data)
        content = format_cpp_content(data)

    #
    # 格式化修正后的代码
    # 
    if fn.endswith('.h') or fn.endswith('.hpp'):
        content = patch_paragma_once(content)

    content = check_include_space_lines(content)

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
            if ext not in ['.h', '.hpp', '.cpp', '.mm']:
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

