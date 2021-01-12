#!/usr/bin/env python3
import sys
import os, os.path

class Selector(object):
    def __init__(self, file_path):
        self.path = file_path
        self.name = os.path.basename(file_path)
        self.CPU = -1
        self.board = -1
        self.project = -1
        with open(file_path, 'r') as f:
            for line in f:
                fields = line.split(':')
                v = fields[1].strip()
                if fields[0] == 'board.cpu':
                    self.CPU = int(v, base = 16)
                elif fields[0] == 'board.model':
                    self.board  = int(v, base = 16)
                elif fields[0] == 'project':
                    self.project = int(v, base = 16)
        if self.CPU < 0 or self.board < 0 or self.project < 0:
            raise RuntimeError('invalid selector file "{}"'.format(file_path))

    def __hash__(self):
        return ((self.CPU * 256) + self.board) * 256 + self.project

    def __eq__(self, other):
        return self.CPU == other.CPU and self.board == other.board and self.project == other.project

    def is_variant_of(self, other):
        return self.name.replace('-secure-', '-') == other.name.replace('-secure-', '-')


def main():
    selectors = dict()
    for selector_file in os.listdir('files'):
        s = Selector(os.path.join('files', selector_file))
        if s in selectors:
            other = selectors[s]
            if not s.is_variant_of(other):
                print('selectors {} and {} are the same'.format(s.name, other.name), file = sys.stderr)
                return 1
        else:
            selectors[s] = s
    return 0


if __name__ == '__main__':
    sys.exit(main())
