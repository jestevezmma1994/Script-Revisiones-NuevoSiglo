#!/usr/bin/env python
from optparse import OptionParser
import sys
import os, os.path
import errno
import shutil
import re
import cgi


report_header = """\
<html>
        <head>
                <title>Class declarations report</title>
                <style type="text/css">
                        body { font-family: Cantarell,Ubuntu,'Droid Sans','Vera Sans',sans-serif; font-size: 92%; color: #555; padding-left: 4px; }
                        div { border: solid #5f5f5f 4px; border-radius: 10px; padding-left: 4px; margin: 2ex 10%; }
                        div div { border: solid #5f5f5f; border-width: 0px 0 0px 0px; border-radius: 0; margin: 1ex 0.5em; }
                        div div div { border: solid #5f5f5f; border-width: 0 0 0 2px; padding: 2px; background: #cfcfcf; }
                        div div div * { margin: 0.5ex 0.25em; padding: 0; }
                        h1, h2 { margin: 0.5ex 2em; text-align: center; }
                        h2 { text-decoration: underline; }
                        h3, h4 { margin: 0.5ex 0.25em; padding: 0; }
                </style>
        </head>
        <body>
                <h1>Class declarations report</h1>
"""


class SanityCheckerReport(object):
    def __init__(self, component_name):
        self._component_name = component_name
        self._partial_reports_dir_path = os.path.join(os.environ['SRM_ROOT'], 'docs', 'class_declarations_report')
        self._partial_report_path = os.path.join(self._partial_reports_dir_path, component_name)
        self._files = dict()
        self._current_file_path = None
        self._current_file_messages = None
        # create directories and files
        if not os.path.isdir(self._partial_reports_dir_path):
            self._create_partial_reports_dir()
        open(os.path.join(self._partial_reports_dir_path, '__empty__'), 'w').close()
        open(self._partial_report_path, 'w').close()

    def _find_base_sbuild(self):
        base_dir = os.path.join(os.environ['SRM_ROOT'], 'base')
        if os.path.isdir(base_dir):
            entries = os.listdir(base_dir)
            if len(entries) == 1 and entries[0].startswith('sbuild-'):
                return os.path.join(base_dir, entries[0])
        return None

    def _create_partial_reports_dir(self):
        base_sbuild_path = self._find_base_sbuild()
        if base_sbuild_path:
            base_partial_reports_dir_path = os.path.join(base_sbuild_path, 'docs', 'class_declarations_report')
            if os.path.isdir(base_partial_reports_dir_path):
                shutil.copytree(base_partial_reports_dir_path, self._partial_reports_dir_path)
                return
        try:
            os.mkdir(self._partial_reports_dir_path, 0755)
        except OSError, exc:
            if exc.errno != errno.EEXIST:
                raise

    def set_current_file_path(self, file_path):
        if self._current_file_path:
            if len(self._current_file_messages) == 0:
                del self._files[self._current_file_path]
        self._current_file_path = file_path
        if file_path:
            if not self._files.has_key(file_path):
                self._files[file_path] = list()
            self._current_file_messages = self._files[file_path]
        else:
            self._current_file_messages = None

    def add_message(self, caption, text = None):
        self._current_file_messages.append((caption, text))

    def _generate_full_report(self):
        partial_report_files = os.listdir(self._partial_reports_dir_path)
        partial_report_files.sort(key = lambda path: os.stat(os.path.join(self._partial_reports_dir_path, path)).st_mtime)

        report_path = os.path.join(os.environ['SRM_ROOT'], 'docs', 'class_declarations_report.html')
        with open(report_path, 'w') as f:
            f.write(report_header)
            for partial_report_path in partial_report_files:
                with open(os.path.join(self._partial_reports_dir_path, partial_report_path), 'r') as xf:
                    f.write(xf.read())
            f.write('\t</body>\n</html>\n')

    def finish(self):
        self.set_current_file_path(None)
        if len(self._files) == 0:
            os.remove(self._partial_report_path)
            return
        with open(self._partial_report_path, 'w') as f:
            f.write('\t\t<div><a name=\"{0}\"></a>\n'.format(self._component_name))
            f.write('\t\t\t<h2>Component {0}</h2>\n'.format(self._component_name))
            for file_path, messages in self._files.iteritems():
                f.write('\t\t\t<div><h4>{0}</h4>\n'.format(file_path))
                for (caption, text) in messages:
                    if not text:
                        f.write('\t\t\t\t<div><h5>{0}</h5></div>\n'.format(caption))
                    else:
                        f.write('\t\t\t\t<div><h5>{0}</h5>\n<pre>\n{1}</pre></div>\n'.format(caption, cgi.escape(text)))
                f.write('\t\t\t</div>\n')
            f.write('\t\t</div>\n')
        self._generate_full_report()


def get_regexp_flags():
    if sys.version_info[0] == 2:
        return 0
    return re.ASCII


def check_args(file_name, class_name, src):
    if src[0] != '(':
        return 'syntax error'

    args = list()
    arg = ''
    level = 0
    next_pos = 0
    for i, c in enumerate(src):
        if c == '(':
            if level > 0:
                arg += c
            level += 1
        elif c == ')':
            level -= 1
            if level == 0:
                args.append(arg.strip())
                next_pos = i + 1
                break
            else:
                arg += c
        elif c == ',' and level == 1:
            args.append(arg.strip())
            arg = ''
        else:
            arg += c

    while src[next_pos] == ' ' or src[next_pos] == '\t':
        next_pos += 1

    if src[next_pos] == '{':
        # found definition of SvTypeCreateManaged()
        return None
    elif src[next_pos] != ';':
        return 'missing semicolon after function call'

    if len(args) < 5 or (len(args) - 5) % 2 != 0:
        return 'invalid number of arguments'
    if args[-1] != 'NULL' and args[-1] != '0':
        return 'list of arguments is not terminated with NULL'
    if not args[0].startswith('"') or not args[0].endswith('"'):
        return 'invalid class name'

    if args[0][1:-1] != class_name:
        return 'declared class name <tt>{0}</tt> doesn\'t match name in function <tt>{1}_getType()</tt>'.format(args[0][1:-1], class_name)

    match = re.match('sizeof[\s]*\\(struct[\s]+([A-Z][A-Za-z0-9_]+)[\s]*\\)', args[1], get_regexp_flags())
    if match is None:
        if re.match('SvTypeGetInstanceSize[\s]*\\(', args[1], get_regexp_flags()) is None:
            return 'invalid class instance size specification'
        return None

    struct_name = match.group(1)
    norm_struct_name = struct_name
    if struct_name.endswith('_'):
        norm_struct_name = struct_name[:-1]
    elif struct_name.endswith('_t') or struct_name.endswith('_s'):
        norm_struct_name = struct_name[:-2]

    if norm_struct_name == class_name:
        return None

    if args[2] == 'SvObject_getType()' or args[2] == '&SvGenericObjectType':
        if norm_struct_name == 'SvObject' or norm_struct_name == 'SvGenericObject':
            return None
    elif args[2].endswith('_getType()'):
        base_class_name = args[2][:-10]
        if norm_struct_name == base_class_name:
            return None

    return 'struct name <tt>{0}</tt> doesn\'t match class name <tt>{1}</tt>'.format(struct_name, class_name)


def check(file_name, src, report):
    ret = 0

    exp = re.compile('\s+', get_regexp_flags())
    lines = [ exp.sub(' ', x) for x in src.split('\n') ]

    func_exp = re.compile('([A-Za-z0-9_]+)_getType', get_regexp_flags())
    full_func_exp = re.compile('([A-Za-z0-9_]+)_getType[\s]*\\([\s]*void[\s]*\\)', get_regexp_flags())
    class_name = None

    call_exp = re.compile('(^|[^\w]+)(SvTypeCreateManaged)[ \t]*(\\(.*)?$', get_regexp_flags())
    for n, l in enumerate(lines):
        match = func_exp.search(l)
        if match is not None:
            match = full_func_exp.search(' '.join(lines[n:n+3]))
            if match is not None:
                class_name = match.group(1)
                continue

        match = call_exp.search(l)
        if match is None:
            continue
        if class_name is None:
            text = '{0}: {1}'.format(n + 1, l)
            report.add_message('found SvTypeCreateManaged() outside of *_getType() function', text)
            continue
        remainder = l[match.end(2):] + ' '.join(lines[n+1:])
        error_desc = check_args(file_name, class_name, remainder)
        if error_desc is None:
            continue
        ret = 1
        # find all lines containg call to SvTypeCreateManaged()
        src_lines = src.split('\n')
        i = n
        while src_lines[i].find('(') < 0:
            i += 1
        j = src_lines[i].find('(') + 1
        depth = 1
        while depth > 0:
            while j == len(src_lines[i]):
                i += 1
                j = 0
            if src_lines[i][j] == '(':
                depth += 1
            elif src_lines[i][j] == ')':
                depth -= 1
            j += 1
        text = ''.join([ '{0}: {1}\n'.format(n + j + 1, k) for j, k in enumerate(src_lines[n:i+1]) ])
        report.add_message(error_desc, text)

    return ret


def load_config():
    config_type = os.environ['SANITY_CHECKER_CONFIG'] if 'SANITY_CHECKER_CONFIG' in os.environ else 'lenient'
    config_file_path = os.path.join(os.environ['SRM_ROOT'], 'host', 'local', 'usr', 'local', 'share', 'sanity_checker', config_type)
    config = dict()

    try:
        with open(config_file_path, 'r') as f:
            for line in f:
                if line.startswith('export '):
                    var = line[7:].split('=', 1)
                    config[var[0].strip()] = var[1].strip() if len(var) > 0 else ''
    except:
        sys.stderr.write('check_class_declarations: error reading config file "{0}"!\n'.format(config_file_path))
        raise

    return config


def main():
    try:
        config = load_config()
    except:
        return 1
    error_status = int(config['EXIT_ON_ERROR']) if 'EXIT_ON_ERROR' in config else 1

    parser = OptionParser()
    parser.add_option('-c', dest = 'component_name', default = '__empty__',
                      help = 'files belong to component named NAME', metavar = 'NAME')
    (options, args) = parser.parse_args()

    report = SanityCheckerReport(options.component_name)

    ret = 0
    for file_path in args:
        report.set_current_file_path(file_path)
        f = None
        src = None
        try:
            f = open(file_path, 'rb')
            src = f.read()
        except:
            sys.stderr.write('check_class_declarations: error reading file "{0}"!\n'.format(file_path))
            return 1
        finally:
            f.close()
        if check(file_path, src.decode('utf-8'), report) != 0:
            ret = error_status

    report.finish()

    return ret


if __name__ == '__main__':
    sys.exit(main())
