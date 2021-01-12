#!/usr/bin/env python3
import sys
import os, os.path
from optparse import OptionParser

class Selector(object):
    def __init__(self, file_path):
        self.path = file_path
        self.name = os.path.basename(file_path)
        self.CPU = -1
        self.board = -1
        self.project = -1
        print ("", file_path)
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

def create_file(file_path, headline, board, cpu, project):
    with open(file_path, 'w') as f:
        f.write(headline)
        f.write("\n")
        f.write("board.cpu: ")
        f.write('{0:02x}'.format(board))
        f.write("\n")
        f.write("board.model: ")
        f.write('{0:02x}'.format(cpu))
        f.write("\n")
        f.write("project: ")
        f.write('{0:02x}'.format(project))
        f.write("\n")

def main():
    parser = OptionParser(usage = 'usage: %prog -p project-name\n')
    parser.add_option('-p', action = 'store', default = None, dest = 'project',
                      help = 'use -p to pass new platform project name')
    (options, args) = parser.parse_args()

    if options.project is None:
        print('You need to specify project name for new platform!')
        return 1

    platform = options.project.split('-')[1]
    max_board = 0
    max_cpu = 0
    default_selector = None
    qb_project = 1
    selector_headline = "OUI: 51 42 27"

    for selector_file in os.listdir('files'):
        s = Selector(os.path.join('files', selector_file))
        if platform in s.name:
            print('Platform You have specified has already exists! (project=', s.name, ')')
            return 1
        if selector_file == "default":
            default_selector = s
        else:
            if max_board < s.board:
                max_board = s.board
            if max_cpu < s.CPU:
                max_cpu = s.CPU
    max_board += 1
    max_cpu += 1
    if default_selector.board == max_board:
        max_board += 1
    if default_selector.CPU == max_cpu:
        max_cpu += 1

    file_path = os.path.join('files', options.project)
    create_file(file_path, selector_headline, max_board, max_cpu, qb_project)
    print ("File ", file_path, " was created with board.model =", '{0:02x}'.format(max_board), " and board.cpu =", '{0:02x}'.format(max_cpu))
    return 0

if __name__ == '__main__':
    sys.exit(main())
