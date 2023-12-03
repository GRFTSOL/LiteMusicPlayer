
import os
import sys
import json


def add_utf8_bom(fn):
    with open(fn, 'rb') as fp:
        content = fp.read()

    if content.startswith(b'\xef\xbb\xbf') or content.startswith(b'\xfe\xff') or content.startswith(b'\xff\xfe'):
        # Already has bom
        print('Already has bom: ', fn)
        return

    print('Add bom: ', fn)

    with open(fn, 'wb') as fp:
        fp.write(b'\xef\xbb\xbf' + content)

    return True

def add_utf8_bom_in_dir(dir):
    count_added = 0
    count_already_has = 0
    for root, _, names in os.walk(dir):
        for name in names:
            fn = os.path.join(root, name)
            ext = os.path.splitext(name)[1]
            if not name.startswith('.') and ext in ('.h', '.hpp', '.cpp', '.c'):
                if add_utf8_bom(fn):
                    count_added += 1
                else:
                    count_already_has += 1

    print('{} files have bom flag already.'.format(count_already_has))
    print('{} files were added successfully.'.format(count_added))

if __name__ == '__main__':
    # add_utf8_bom_in_dir(r'C:\Users\xhy\Documents\Mp3Player\TinyJS')

    if len(sys.argv) != 2:
        print('{} DIR_TO_ADD_BOM'.format(os.path.basename(__file__)))
        sys.exit(1)

    add_utf8_bom_in_dir(sys.argv[1])

