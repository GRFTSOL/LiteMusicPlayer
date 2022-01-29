
import os
import sys
import re
import json


def parse_header_defines_file(fn):
    with open(fn, 'rb') as fp:
        content = fp.read()

    ids = []

    for line in content.splitlines():
        if not line.startswith('#define '):
            continue

        line = line[len('#define '):].strip()
        name, id = line.split()
        if name.startswith('ID'):
            ids.append((name, id))

    return ids


BEGIN = 'BEGIN'
END = 'END'
POPUP = 'POPUP'
MENUITEM = 'MENUITEM'
SEPARATOR = 'SEPARATOR'

class MenuParser():

    def __init__(self, content):
        self._lines = content.splitlines()
        self._i = 0
        self.menus = self._parse()

    def _parse(self):
        menus = []

        while self._i < len(self._lines):
            line = self._lines[self._i].strip()
            if line.startswith(POPUP):
                menus.append(self._parse_pop())
            else:
                assert line == ''

        return menus

    def _parse_menu_item(self, line):
        # parse: "&Browse File...",             IDC_BROWSE
        line = line.strip()
        name, _, id = line.rpartition(',')
        id = id.strip()
        if name == '':
            assert id == SEPARATOR
            return ['separator']
        return [name.strip('" '), id]

    def _parse_pop(self):
        line = self._lines[self._i].strip()
        self._i += 1

        items = []
        cmd, _, pop_name = line.partition(' ')
        assert cmd == POPUP
        pop_name = pop_name.strip('" \n')

        assert self._lines[self._i].strip() == 'BEGIN'
        self._i += 1

        while self._i < len(self._lines):
            line = self._lines[self._i].strip()
            if line == '':
                continue

            cmd, _, line = line.partition(' ')
            if cmd == POPUP:
                items.append(self._parse_pop())
            elif cmd == MENUITEM:
                items.append(self._parse_menu_item(line))
                self._i += 1
            elif cmd == END:
                self._i += 1
                break
            else:
                raise Exception('Unexpected cmd: {}'.format(cmd))

        return [pop_name, items]

def parse_rc_menu_file(fn):
    with open(fn, 'rb') as fp:
        content = fp.read()

    content = content.replace('\r', '')
    _, _, content = content.partition('IDR_MENU_ID MENU\nBEGIN\n')
    assert content
    content, _, _ = content.partition('\nEND')

    return MenuParser(content).menus

def convert_menu_id(menus, id_defines):
    for item in menus:
        assert type(item) is list
        if len(item) == 2:
            _name, id = item
            if type(id) is list:
                convert_menu_id(id, id_defines)
            elif id is not None:
                item.append(int(id_defines[id]))

def save_menu(fn, menus, id_defines):
    menus = dict(menus)
    with open(fn, 'wb') as fp:
        json.dump(menus, fp, indent=2)

    # lines = ['menufile_v2']

    # def dump_menu(items, indent):
    #     for name, id in items:
    #         if type(id) is list:
    #             lines.append(indent + 'popup@' + name)
    #             dump_menu(id, indent + '  ')
    #         elif id is None:
    #             lines.append(indent + name)
    #         else:
    #             lines.append(indent + name + '@' + id)

    # for name, items in menus:
    #     lines.append('\n# menu: ' + name)
    #     dump_menu(items, '  ')

    # with open(fn, 'wb') as fp:
    #     fp.write('\n'.join(lines))


def convert_menu_rc(fn_rc, fn_header):
    if fn_header.endswith('.rc'):
        fn_rc, fn_header = fn_header, fn_rc

    menus = parse_rc_menu_file(fn_rc)
    id_defines = parse_header_defines_file(fn_header)

    id_defines = dict(id_defines)
    id_defines['IDCLOSE'] = '8'

    # print(json.dumps(menus, indent=2))

    convert_menu_id(menus, id_defines)

    fn_out = os.path.splitext(fn_rc)[0] + '.json'
    save_menu(fn_out, menus, id_defines)

    print('{} was saved successfully.'.format(fn_out))


if __name__ == '__main__':
    if len(sys.argv) != 3:
        print('{} rc_file header_file'.format(os.path.basename(__file__)))
        sys.exit(1)

    convert_menu_rc(sys.argv[1], sys.argv[2])

