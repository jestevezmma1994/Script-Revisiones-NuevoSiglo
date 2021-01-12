#!/usr/bin/python
#
# Cubiware Sp. z o.o. Software License Version 1.0
#
# Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
#
# Any rights which are not expressly granted in this License are entirely and
# exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
# modify, translate, reverse engineer, decompile, disassemble, or create
# derivative works based on this Software. You may not make access to this
# Software available to others in connection with a service bureau,
# application service provider, or similar business, or make any other use of
# this Software without express written permission from Cubiware Sp. z o.o.
#
# Any User wishing to make use of this Software must contact
# Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
# includes, but is not limited to:
# (1) integrating or incorporating all or part of the code into a product for
#     sale or license by, or on behalf of, User to third parties;
# (2) distribution of the binary or source code to third parties for use with
#     a commercial product sold or licensed by, or on behalf of, User.
#

import sys, os, fnmatch
from itertools import chain
try:
    from elftools.common.py3compat import bytes2str
    from elftools.elf.elffile import ELFFile
    from elftools.elf.dynamic import DynamicSection
    from elftools.elf.relocation import RelocationSection
    from elftools.elf.sections import Section, SymbolTableSection
    from elftools.elf.descriptions import *
    from pytrie import SortedStringTrie as trie
except:
    print '''You'll need to install pyelftools first so check_licenses.py runs properly. You can try either apt-get or pip:
             sudo apt-get install pyelftools
              or
             sudo pip install pyelftools
             sudo pip install pytrie
            
             In case of any problems please contact Adam Dawidziuk (adam@cubiware.com). Quitting for now.
          '''
    sys.exit(1)

from stat import *
import json
import collections
import copy
import re

GLOBAL_ERROR = 0
GLOBAL_PARSE_DLKM_SYMBOLS = 0

global_trie = trie()

# Note that compatibility is set according to Cubiware release needs, and not purely due to general license compatibilities.
# E.g. gplv3 "anty-lock-down" provisions apply here.
# dict - first column is "output license produced", list entries: whether such input/lib can be used to produce license in the first columni
#
# I guess most interactions for all combinations are really unknown here... so we'll just put things here that we know for 99% are INCOMPATIBLE. The rest (most notably) permissive/weak copy-left licenses
# are assumed to be safe from Cubiware perspective.
#
# Exit with error when unknown license is being used.
license_db= {
    'GPLv2':{
        'attribution': False,
        'FOSS': True,
        'allowed': True
    },
    'GPLv3':{
        'attribution': False,
        'FOSS': True,
        'allowed': False
    },
    'GPLv2e':{
        'attribution': False,
        'FOSS': True,
        'allowed': True
    },
    'GPLv3e':{
        'attribution': False,
        'FOSS': True,
        'allowed': True
    },
    'LGPLv2':{
        'attribution': False,
        'FOSS': True,
        'allowed': True
    },
    'LGPLv2.1':{
        'attribution': False,
        'FOSS': True,
        'allowed': True
    },
    'FREE':{
        'attribution': False,
        'FOSS': True,
        'allowed': True
    },
    'BSD':{
        'attribution': False,
        'FOSS': True,
        'allowed': True
    },
    'BSD-2C':{
        'attribution': False,
        'FOSS': True,
        'allowed': True
    },
    'BSD-3C':{
        'attribution': False,
        'FOSS': True,
        'allowed': True
    },
    'BSD-4C':{
        'attribution': False,
        'FOSS': True,
        'allowed': True
    },
    'ZLIB':{
        'attribution': False,
        'FOSS': True,
        'allowed': True
    },
    'MIT':{
        'attribution': False,
        'FOSS': True,
        'allowed': True
    },
    'SSLeay':{
        'attribution': False,
        'FOSS': True,
        'allowed': True
    },
    'INFO-ZIP':{
        'attribution': False,
        'FOSS': True,
        'allowed': True
    },
    'NTP':{
        'attribution': False,
        'FOSS': True,
        'allowed': True
    },
    'libpng':{
        'attribution': False,
        'FOSS': True,
        'allowed': True
    },
    'FTL':{
        'attribution': False,
        'FOSS': True,
        'allowed': True
    },
    'QB':{
        'attribution': False,
        'FOSS': False,
        'allowed': True
    },
    'VMX':{
        'attribution': False,
        'FOSS': False,
        'allowed': True
    },
    'BRCM':{
        'attribution': False,
        'FOSS': False,
        'allowed': True
    },
    'Quadrille':{
        'attribution': False,
        'FOSS': False,
        'allowed': True
    },
    'STMicro':{
        'attribution': False,
        'FOSS': False,
        'allowed': True
    },
    'ALi':{
        'attribution': False,
        'FOSS': False,
        'allowed': True
    },
    'Intek':{
        'attribution': False,
        'FOSS': False,
        'allowed': True
    },
    'Latens':{
        'attribution': False,
        'FOSS': False,
        'allowed': True
    },
    'MSFT':{
        'attribution': False,
        'FOSS': False,
        'allowed': True
    },
    'Opera':{
        'attribution': False,
        'FOSS': False,
        'allowed': True
    },
    'Kaon':{
        'attribution': False,
        'FOSS': False,
        'allowed': True
    },
    'ZTE':{
        'attribution': False,
        'FOSS': False,
        'allowed': True
    },
    'Arris':{
        'attribution': False,
        'FOSS': False,
        'allowed': True
    },
    'MStar':{
        'attribution': False,
        'FOSS': False,
        'allowed': True
    },
    'Crenova':{
        'attribution': False,
        'FOSS': False,
        'allowed': True
    }
}

foss_licenses = [i for i in license_db if license_db[i]['FOSS'] == True]
prop_licenses = [i for i in license_db if license_db[i]['FOSS'] == False]

known_licenses   =  foss_licenses + prop_licenses
allowed_licenses =  [i for i in license_db if license_db[i]['allowed'] == True]

allowed_combinations = {
# OUTPUT/releasing as: [INPUTS/dependencies]

'GPLv2':    ['GPLv2', 'GPLv2e',          'GPLv3e', 'LGPLv2', 'LGPLv2.1', 'FREE',        'BSD-2C', 'BSD-3C',           'ZLIB', 'MIT',                       'NTP', 'libpng',                                                          ],
'GPLv2e':   [         'GPLv2e',                    'LGPLv2', 'LGPLv2.1', 'FREE', 'BSD', 'BSD-2C', 'BSD-3C', 'BSD-4C', 'ZLIB', 'MIT', 'SSLeay', 'INFO-ZIP', 'NTP', 'libpng', 'FTL',                                                   ],
'GPLv3':    ['GPLv2', 'GPLv2e',          'GPLv3e', 'LGPLv2', 'LGPLv2.1', 'FREE',        'BSD-2C', 'BSD-3C',           'ZLIB', 'MIT',                       'NTP', 'libpng',                                                          ],
'GPLv3e':   [                            'GPLv3e', 'LGPLv2', 'LGPLv2.1', 'FREE', 'BSD', 'BSD-2C', 'BSD-3C', 'BSD-4C', 'ZLIB', 'MIT', 'SSLeay', 'INFO-ZIP', 'NTP', 'libpng',                                                          ],
'LGPLv2':   [         'GPLv2e',          'GPLv3e', 'LGPLv2', 'LGPLv2.1', 'FREE', 'BSD', 'BSD-2C', 'BSD-3C', 'BSD-4C', 'ZLIB', 'MIT', 'SSLeay', 'INFO-ZIP', 'NTP', 'libpng', 'FTL',                                                   ],
'LGPLv2.1': [         'GPLv2e',          'GPLv3e', 'LGPLv2', 'LGPLv2.1', 'FREE', 'BSD', 'BSD-2C', 'BSD-3C', 'BSD-4C', 'ZLIB', 'MIT', 'SSLeay', 'INFO-ZIP', 'NTP', 'libpng', 'FTL',                                                   ],
'FREE':     [         'GPLv2e',          'GPLv3e', 'LGPLv2', 'LGPLv2.1', 'FREE', 'BSD', 'BSD-2C', 'BSD-3C', 'BSD-4C',                                                                                                                ],
'BSD':      [         'GPLv2e',          'GPLv3e', 'LGPLv2', 'LGPLv2.1', 'FREE', 'BSD', 'BSD-2C', 'BSD-3C', 'BSD-4C', 'ZLIB', 'MIT', 'SSLeay', 'INFO-ZIP', 'NTP', 'libpng', 'FTL', 'QB'                                              ],
'BSD-2C':   ['GPLv2', 'GPLv2e', 'GPLv3', 'GPLv3e', 'LGPLv2', 'LGPLv2.1', 'FREE', 'BSD', 'BSD-2C', 'BSD-3C', 'BSD-4C', 'ZLIB', 'MIT', 'SSLeay', 'INFO-ZIP', 'NTP', 'libpng', 'FTL',                                                   ],
'BSD-3C':   ['GPLv2', 'GPLv2e', 'GPLv3', 'GPLv3e', 'LGPLv2', 'LGPLv2.1', 'FREE', 'BSD', 'BSD-2C', 'BSD-3C', 'BSD-4C', 'ZLIB', 'MIT', 'SSLeay', 'INFO-ZIP', 'NTP', 'libpng', 'FTL',                                                   ],
'BSD-4C':   [         'GPLv2e',          'GPLv3e', 'LGPLv2', 'LGPLv2.1', 'FREE', 'BSD', 'BSD-2C', 'BSD-3C', 'BSD-4C', 'ZLIB', 'MIT', 'SSLeay', 'INFO-ZIP', 'NTP', 'libpng', 'FTL',                                                   ],
'ZLIB':     ['GPLv2', 'GPLv2e', 'GPLv3', 'GPLv3e', 'LGPLv2', 'LGPLv2.1', 'FREE', 'BSD', 'BSD-2C', 'BSD-3C', 'BSD-4C', 'ZLIB', 'MIT', 'SSLeay', 'INFO-ZIP', 'NTP', 'libpng', 'FTL',                                                   ],
'MIT':      ['GPLv2', 'GPLv2e', 'GPLv3', 'GPLv3e', 'LGPLv2', 'LGPLv2.1', 'FREE', 'BSD', 'BSD-2C', 'BSD-3C', 'BSD-4C', 'ZLIB', 'MIT', 'SSLeay', 'INFO-ZIP', 'NTP', 'libpng', 'FTL',                                                   ],
'SSLeay':   [         'GPLv2e',          'GPLv3e', 'LGPLv2', 'LGPLv2.1', 'FREE', 'BSD', 'BSD-2C', 'BSD-3C', 'BSD-4C', 'ZLIB', 'MIT', 'SSLeay', 'INFO-ZIP', 'NTP', 'libpng', 'FTL',                                                   ],
'INFO-ZIP': [         'GPLv2e',          'GPLv3e', 'LGPLv2', 'LGPLv2.1', 'FREE', 'BSD', 'BSD-2C', 'BSD-3C', 'BSD-4C', 'ZLIB', 'MIT', 'SSLeay', 'INFO-ZIP', 'NTP', 'libpng', 'FTL',                                                   ],
'NTP':      ['GPLv2', 'GPLv2e', 'GPLv3', 'GPLv3e', 'LGPLv2', 'LGPLv2.1', 'FREE', 'BSD', 'BSD-2C', 'BSD-3C', 'BSD-4C', 'ZLIB', 'MIT', 'SSLeay', 'INFO-ZIP', 'NTP', 'libpng', 'FTL',                                                   ],
'libpng':   ['GPLv2', 'GPLv2e', 'GPLv3', 'GPLv3e', 'LGPLv2', 'LGPLv2.1', 'FREE', 'BSD', 'BSD-2C', 'BSD-3C', 'BSD-4C', 'ZLIB', 'MIT', 'SSLeay', 'INFO-ZIP', 'NTP', 'libpng', 'FTL',                                                   ],
'FTL':      [         'GPLv2e',          'GPLv3e', 'LGPLv2', 'LGPLv2.1', 'FREE', 'BSD', 'BSD-2C', 'BSD-3C', 'BSD-4C', 'ZLIB', 'MIT', 'SSLeay', 'INFO-ZIP', 'NTP', 'libpng', 'FTL',                                                   ],
'QB':       [         'GPLv2e',          'GPLv3e', 'LGPLv2', 'LGPLv2.1', 'FREE', 'BSD', 'BSD-2C', 'BSD-3C', 'BSD-4C', 'ZLIB', 'MIT', 'SSLeay', 'INFO-ZIP', 'NTP', 'libpng', 'FTL', 'QB', 'VMX', 'BRCM', 'Quadrille', "STMicro", "ALi", "Intek", "Latens", "MSFT", "Opera", "Kaon", "ZTE", "Arris", "MStar", "Crenova"],
'VMX':      [         'GPLv2e',          'GPLv3e', 'LGPLv2', 'LGPLv2.1', 'FREE', 'BSD', 'BSD-2C', 'BSD-3C', 'BSD-4C', 'ZLIB', 'MIT', 'SSLeay', 'INFO-ZIP', 'NTP', 'libpng', 'FTL',       'VMX', 'BRCM',              "STMicro", "ALi"                           ],
'BRCM':     [         'GPLv2e',          'GPLv3e', 'LGPLv2', 'LGPLv2.1', 'FREE', 'BSD', 'BSD-2C', 'BSD-3C', 'BSD-4C', 'ZLIB', 'MIT', 'SSLeay', 'INFO-ZIP', 'NTP', 'libpng', 'FTL',              'BRCM',                                                         ],
'Quadrille':[         'GPLv2e',          'GPLv3e', 'LGPLv2', 'LGPLv2.1', 'FREE', 'BSD', 'BSD-2C', 'BSD-3C', 'BSD-4C', 'ZLIB', 'MIT', 'SSLeay', 'INFO-ZIP', 'NTP', 'libpng', 'FTL',              'BRCM', 'Quadrille', "STMicro", "ALi"                           ],
'STMicro':  [         'GPLv2e',          'GPLv3e', 'LGPLv2', 'LGPLv2.1', 'FREE', 'BSD', 'BSD-2C', 'BSD-3C', 'BSD-4C', 'ZLIB', 'MIT', 'SSLeay', 'INFO-ZIP', 'NTP', 'libpng', 'FTL', 'QB',                             "STMicro",        "Intek"                  ],
'ALi':      [         'GPLv2e',          'GPLv3e', 'LGPLv2', 'LGPLv2.1', 'FREE', 'BSD', 'BSD-2C', 'BSD-3C', 'BSD-4C', 'ZLIB', 'MIT', 'SSLeay', 'INFO-ZIP', 'NTP', 'libpng', 'FTL',                                              "ALi"                           ],
'Intek':    [         'GPLv2e',          'GPLv3e', 'LGPLv2', 'LGPLv2.1', 'FREE', 'BSD', 'BSD-2C', 'BSD-3C', 'BSD-4C', 'ZLIB', 'MIT', 'SSLeay', 'INFO-ZIP', 'NTP', 'libpng', 'FTL',                                   "STMicro",        "Intek"                  ],
'Latens':   [         'GPLv2e',          'GPLv3e', 'LGPLv2', 'LGPLv2.1', 'FREE', 'BSD', 'BSD-2C', 'BSD-3C', 'BSD-4C', 'ZLIB', 'MIT', 'SSLeay', 'INFO-ZIP', 'NTP', 'libpng', 'FTL',                                                              "Latens"        ],
'MSFT':     [         'GPLv2e',          'GPLv3e', 'LGPLv2', 'LGPLv2.1', 'FREE', 'BSD', 'BSD-2C', 'BSD-3C', 'BSD-4C', 'ZLIB', 'MIT', 'SSLeay', 'INFO-ZIP', 'NTP', 'libpng', 'FTL',                                                                        "MSFT"],
'Opera':    [         'GPLv2e',          'GPLv3e', 'LGPLv2', 'LGPLv2.1', 'FREE', 'BSD', 'BSD-2C', 'BSD-3C', 'BSD-4C', 'ZLIB', 'MIT', 'SSLeay', 'INFO-ZIP', 'NTP', 'libpng', 'FTL',              'BRCM',                                                          "Opera"],
'Kaon':     [         'GPLv2e',          'GPLv3e', 'LGPLv2', 'LGPLv2.1', 'FREE', 'BSD', 'BSD-2C', 'BSD-3C', 'BSD-4C', 'ZLIB', 'MIT', 'SSLeay', 'INFO-ZIP', 'NTP', 'libpng', 'FTL',              'BRCM',                                                                  "Kaon"],
'ZTE':      [         'GPLv2e',          'GPLv3e', 'LGPLv2', 'LGPLv2.1', 'FREE', 'BSD', 'BSD-2C', 'BSD-3C', 'BSD-4C', 'ZLIB', 'MIT', 'SSLeay', 'INFO-ZIP', 'NTP', 'libpng', 'FTL',              'BRCM',                                                                         "ZTE"],
'Arris':    [         'GPLv2e',          'GPLv3e', 'LGPLv2', 'LGPLv2.1', 'FREE', 'BSD', 'BSD-2C', 'BSD-3C', 'BSD-4C', 'ZLIB', 'MIT', 'SSLeay', 'INFO-ZIP', 'NTP', 'libpng', 'FTL',              'BRCM',                                                                              "Arris"],
'MStar':    [         'GPLv2e',          'GPLv3e', 'LGPLv2', 'LGPLv2.1', 'FREE', 'BSD', 'BSD-2C', 'BSD-3C', 'BSD-4C', 'ZLIB', 'MIT', 'SSLeay', 'INFO-ZIP', 'NTP', 'libpng', 'FTL',              'BRCM',                                                                                     "MStar"],
'Crenova':  [         'GPLv2e',          'GPLv3e', 'LGPLv2', 'LGPLv2.1', 'FREE', 'BSD', 'BSD-2C', 'BSD-3C', 'BSD-4C', 'ZLIB', 'MIT', 'SSLeay', 'INFO-ZIP', 'NTP', 'libpng', 'FTL',              'BRCM',                                                                                             "Crenova"]
}

# Build a global prefix tree for a db, this is ugly but easier...
def build_prefix_tree(db):
    local_trie = trie()

    for entry in db:
        local_trie[entry] = db[entry]

    return local_trie

# TODO: Use OrdereDict for processing
def find_files_iterables(paths, pattern):
    for root, dirs, files in chain.from_iterable(os.walk(path) for path in paths):
        for basename in files:
            if fnmatch.fnmatch(basename, pattern):
                filename = os.path.join(root, basename)
                try:
                    mode = os.lstat(filename).st_mode
                    if not (S_ISCHR(mode) or S_ISDIR(mode) or S_ISLNK(mode) or S_ISBLK(mode) or S_ISFIFO(mode)):
                        yield filename, root
                except:
                    pass

# Find dependency for a specific symbol in a module
def find_symbol_dep(depmod, symbol, module_deps_so_far):
    for module in depmod:
        if symbol in depmod[module]["exports"]:
            if module not in module_deps_so_far:
                module_deps_so_far.append(module)
    return module_deps_so_far

# Find all dependencies for a module
def find_module_dep(shallow_deps, module, output):
    for dep in shallow_deps[module]:
        if dep not in output:
            output.append(dep)
            find_module_dep(shallow_deps, dep, output)

# Find best (or first rather..) matching key (in case some keys in db are regex expressions)
def find_best_key(db, key):
    global global_trie

    if key in db:
        return key
    else:
        try:
            k = global_trie.longest_prefix(key)
            if db[k]["license"] == "QB":
                return k
            else:
                return None
        except:
            return None

# Build module dependency tree
def build_depmod(depmod):
    shallow_deps = {}

    # Find dependencies based on used symbols, this is shallow
    for module in depmod:
        module_deps_so_far = []
        for symbol in depmod[module]["depends"]:
            module_deps_so_far = find_symbol_dep(depmod, symbol, module_deps_so_far)
        shallow_deps.update({module : module_deps_so_far})

    # Find deep module dependencies
    deep_deps = {}
    for module in shallow_deps:
        deep = []
        find_module_dep(shallow_deps, module, deep)
        deep_deps.update({module : deep})

    return deep_deps

# Print out deep-tree dependency for arbitrary soname/ELF
def audit_soname_license(soname, db, scan, indent, cache):
    if soname in db:
        if soname in scan:
            for needed in scan[soname]:
                if needed not in cache:
                    cache.append(needed)
                    print "*" * indent, needed
                    audit_soname_license(needed, db, scan, indent + 1, cache)

# Scan / search for all ELF files under location that contain a dynamic section (.so/executables)
# or .modinfo section (DLKM)
def scan_location_ko(location):
    global GLOBAL_PARSE_DLKM_SYMBOLS

    # Mapping of license strings from Kernel/Linux like naming to QB naming scheme
    # Note: treat all Proprietary licenses the same way as QB license
    lkms_to_qb_mapping = { "GPL" : "GPLv2", 
                           "Dual BSD/GPL" : "GPLv2", 
                           "Cubiware-proprietary" : "QB",
                           "UNKNOWN" : "UNKNOWN",
                           "Proprietary" : "QB" } 
    # .so / executable results
    scan = {}

    # DLKM results
    depmod = {}
    db_modules = {}

    # Scan ALL files from location really, since perms/names/"extensions" may be messed up
    # We will decide if it's an ELF based on the exception below
    for filename,r in find_files_iterables([location], "*"):
        try:
            elffile = ELFFile(open(filename, "r"))
        except:
            continue

        # Try a dynamic section, likely .so or an executable
        section = elffile.get_section_by_name(b'.dynamic')

        # Assume for a while that we will use soname from filename, which
        # for actual .so libraries will not be true
        soname = os.path.basename(filename)

        # .so / executable
        if isinstance(section, DynamicSection):
            # Continue with regular ELFs
            soname = os.path.basename(filename)

            dlist = []

            for i in section.iter_tags():
                if i.entry.d_tag in i._HANDLED_TAGS:
                    tag = i.entry.d_tag[3:].lower()
                    s = '%s' % getattr(i, tag)
                    if tag == "needed":
                        dlist.append(os.path.basename(s))
                    elif tag == "soname":
                        soname = os.path.basename(s)
            if soname in scan:
                pass
            else:
                scan.update({soname : dlist})
        else:
            # Special case for DLKM - we're looking for .modinfo section
            section = elffile.get_section_by_name(b'.modinfo')

            # We will try to retrieve DLKM license
            retrieved_license = None

            # .modinfo not found, likely not DLKM
            if not isinstance(section, Section):
                continue
            else:
                items = section.data().split("\0")
                for item in items:
                    if item != "":
                        m = re.match("license=(.*)", item)
                        if m != None:
                            retrieved_license = m.group(1)
                            break
                    
            if retrieved_license == None:
                retrieved_license = "UNKNOWN"

            if GLOBAL_PARSE_DLKM_SYMBOLS == 1:
                # Now let see what's inside the symbol table
                symtab = elffile.get_section_by_name(b'.symtab')
                if not isinstance(symtab, SymbolTableSection):
                    # Very unusual.... should we return error here?
                    continue
                else:
                    deplist = []
                    explist = []
                    # Go through the list of symbols and narrow down those that are exported and those that are needed
                    for symbol in symtab.iter_symbols():
                        # We ignore all STB_LOCAL symbols
                        if symbol.entry["st_info"]["bind"] != "STB_LOCAL":
                            # EXPORT list
                            if symbol.entry["st_info"]["type"] == "STT_FUNC" and symbol.entry["st_shndx"] != "SHN_UNDEF":
                                explist.append(symbol.name)
                            # DEPENDS list
                            elif symbol.entry["st_shndx"] == "SHN_UNDEF":
                                deplist.append(symbol.name)
                    # Update depmod dict
                    depmod.update({soname : {"exports" : explist, "depends" : deplist}})
#
# This is not necessary after all
#            gpllist = []
#            # Now let's see if there's a rela_ksymtab_gpl present
#            # if so then there are some symbols that are GPL licensed, so we need to extrat this info
#             # rela__ksymtab_gpl
#            section = elffile.get_section_by_name(b'.rela__ksymtab_gpl')
#            if isinstance (section, RelocationSection):
#                for relocation in section.iter_relocations():
#                    idx = relocation.entry["r_info_sym"]
#                    sym = symtab.get_symbol(idx)
#                    gpllist.append(sym.name)
#                depmod[soname].update({"gpllist" : gpllist})
            else:
                # Just create empty/fake depmod entry to be compatible with build_depmo function
                depmod.update({soname : {"exports" : [], "depends" : []}})

            if retrieved_license in lkms_to_qb_mapping:
                mod_license = lkms_to_qb_mapping[retrieved_license]
            else:
                mod_license = "UNKNOWN"
            # Update license db entry for this module
            db_modules.update({soname : {"license": mod_license, "original": retrieved_license}}) 

    # Build deep module dependencies and return along with ELF scan
    return scan, build_depmod(depmod), db_modules

# Check DLKM cross-compliance
def check_compliance_ko(db, scan, db_modules):
    global GLOBAL_ERROR
    
    # Just check if licenses in db and db_modules agree with the exception of
    # UNKNOWN (which is just unset)

    for module in scan:
        if (db_modules[module]["license"] != db[module]["license"]) and db_modules[module]["license"] != "UNKNOWN":
            print "[%s]: License mismatch. db.json says [%s], .ko file says [ (%s) -> (%s) ]" % (module, db[module]["license"], db_modules[module]["original"], db_modules[module]["license"])
            GLOBAL_ERROR = 1
    
# Check cross-license compliance
def check_compliance(db, scan, cache):
    # For each soname from "scan" : for each DT_NEEDED check it's license and run through our compliance mask
    # e.g. QB + MIT + LGPL + others = OK
    # e.g. QB + GPLv2 = NOK
    # e.g. Any GPLv3 NOK!
    # Notes: ssleay incompat with gpl, FTL license incompat with GPLv2
    global GLOBAL_ERROR

    for soname in scan:
        soname_from_scan = soname
        soname = find_best_key(db, soname)

        if soname not in db:
            print "[%s]: Could not determine license for this component. Make sure your scan works properly and db.json is up-to-date." % soname
            GLOBAL_ERROR = 1
        if db[soname]["license"] not in allowed_licenses:
            print "[%s]: License [%s] is not allowed on the box!" % (soname, db[soname]["license"])
            GLOBAL_ERROR = 1
        if db[soname]["license"] not in allowed_combinations:
            print "[%s]: Interactions for license [%s] are not defined!" % (soname, db[soname]["license"])
            GLOBAL_ERROR = 1

        for needed in scan[soname_from_scan]:
            needed = find_best_key(db, needed)

            if needed not in db:
                print "     Could not determine license for [%s]. Make sure your scan works properly and db.json is up-to-date." % needed
            else:
                if db[needed]["license"] == "":
                    print "    Could not determine license type for [%s]" % needed
                else:
                    # Deep check, iterate to the absolute bottom - really audit if compatibility checks out for the ENTIRE dependency tree
                    # audit_soname_license(needed, db, scan, 0, cache)

                    # Shallow check - only immediate NEEDED dependency check
                    if db[needed]["license"] not in allowed_licenses:
                        print "  [%s] License [%s] of [%s] is not allowed on the box!" % (soname, db[needed]["license"], needed)
                        GLOBAL_ERROR = 1

                    if db[soname]["license"] not in allowed_combinations:
                        GLOBAL_ERROR = 1
                        continue
                       
                    if db[needed]["license"] not in allowed_combinations[db[soname]["license"]]:
                        if "exceptions" in db[soname] and db[needed]["license"] in db[soname]["exceptions"] :
                            print "  [%s : %s] USING EXCEPTION for [%s : %s]" % (soname, db[soname]["license"], needed, db[needed]["license"])
                        else:
                            print "  [%s : %s] INCOMPATIBILITY with dependency [%s : %s]" % (soname, db[soname]["license"], needed, db[needed]["license"])
                            print "I'm here"
                            GLOBAL_ERROR = 1

# Return an aggregate list of all licenses used in the scan
def check_licenses_used(db):
    licenses = []
    for soname in db:
        if db[soname]["license"] not in licenses:
            licenses.append(db[soname]["license"])
    return licenses 

# Report components missing license definition
def check_db(db):
    global GLOBAL_ERROR

    for soname in db:
        if db[soname]["license"] == "":
            print "[%s] Missing License" % soname
            GLOBAL_ERROR = 1

# Report attribution requirements
def check_attribution(db, scan, scan_modules, attribution_file):
    global GLOBAL_ERROR

    packages = []

    combined = scan.copy()
    combined.update(scan_modules)

    for soname in combined:
        soname_p = find_best_key(db, soname)
        if soname_p in db and  db[soname_p]["license"] != "" and  license_db[db[soname_p]["license"]]["FOSS"] == True:
                if db[soname_p]["package"] not in packages:
                    packages.append(db[soname_p]["package"])

    if attribution_file == "-":
        attr_output = sys.stdout
    else:
        attr_output = open(attribution_file, "w")

    packages.sort()
    print >> attr_output, '''Open-Source software compliance notice.\n
The list of open-source software used in this product can be found below.
For more information on copyright and licensing please visit:
    https://opensource.cubiware.com\n

(All copyrights belong to their respective owners)
'''

    for package in packages:
        print >>attr_output, package

# Update License DB with new entries found from the scan
def merge_db(db, scan):

    global global_trie
    global_trie = build_prefix_tree(db)

    for soname in scan:
        soname_p = find_best_key(db, soname)
        if soname_p == None:
            print "New entry found [%s]" % soname
            db.update({soname : {"license": ""}})

    global_trie = build_prefix_tree(db)
    return db

# Update license DB with new entires from the scan (if any) and re-write source db .json file
def update_db(dbfile, scan):
    new = 0
    try:
        f = open(dbfile, "r")
    except:
        new = 1
    
    if not new:
        try:
            idata = json.loads(f.read())
            f.close()
        except:
            print "Could not parse json from [%s]" % dbfile
            # Throw instead I guess
            sys.exit(0)

    db = copy.deepcopy(idata)

    db = merge_db(db, scan)

    if db != idata:
        print "Rewriting db file [%s]" % dbfile
        f = open(dbfile, "w+")
        f.write(json.dumps(db, sort_keys = True, indent=4, separators=(',', ': ') ))

    check_db(db)

    return db

if __name__ == '__main__':
    # Print program info
    print "Checking software licenses..."

    # Get License DB filename from args
    try:
        dbfile = sys.argv[1]
    except:
        print "Please provide path to [dbfile] as a first argument"
        sys.exit(1)

    # Get location from args
    try:
        location = sys.argv[2]
    except:
        print "Please provide path to [location] as a second argument"
        sys.exit(1)

    # Get (optional) third parameter - output "attribution.txt" file
    try:
        attribution_file = sys.argv[3]
    except:
        attribution_file = None

    # Scan/Search for all ELF under the location
    scan, scan_modules, db_modules = scan_location_ko(location)

    # Artificially update the scan to include components
    # missing from rootfs but otherwise present in the
    # product like bootloaders, etc.
    # TODO: Ali, MStar cases
    if "libnexus.so" in scan:
        # Assume a Broadcom platform, CFE bootloader
        scan.update({"CFE" : [] })
    elif "libstapi.so" in scan:
        # Assume ST platform, u-boot bootloader at some point
        scan.update({"u-boot" : [] })

    # Update License DB with new ELF entires (.so and executable, DLKMs) (if found)
    # Note that DLKM entries are also live from modules themselves, so we
    # will compare both statically defined licenses as well as dynamically parsed out of those modules
    # This approach is mostly for LOTS of DLKMs out there that do not specify a license
    # explicitly, though we know the license anyway

    # So update the scan_all with entries from scan_modules
    scan_all = copy.deepcopy(scan)
    scan_all.update(scan_modules)

    # Update static DB, ELF/DLKM
    db = update_db(dbfile, scan_all)

    # Cache to optimize deep-check time (if used)
    cache = []

    # Get aggregate list of licenses used across the entire License DB
    # Note that current scan results will leak to db via update_db
    used = check_licenses_used(db)

    # Exit with error on unknown licenses 
    for license in used:
        if license not in known_licenses:
            print "License [%s] is unknown." % (license)
            GLOBAL_ERROR = 1

    # Check license cross-compliance (static mode for ELFs/DKMS)
    check_compliance(db, scan, cache)
    
    # Check module compliance.
    # This check is different, we will just make sure that
    # static licenses from db match those parsed dynamically out of respective modules.
    check_compliance_ko(db, scan_modules, db_modules)

    # Attribution check
    if attribution_file != None:
        check_attribution(db, scan, scan_modules, attribution_file)

    # Exit with error if any error occurred
    if GLOBAL_ERROR == 1:
        print ''' 
                  !!! WARNING !!!

                  There are critical errors detected by this script. There are some very specific steps you need to take to correct these errors:

                  1. Please download and install the most up-to-date Src/license_checker repo and re-run the script.
                  2. Please try to understand the nature of these errors. Likely some components violate Cubiware and/or third-party licenses,
                     however errors may also be caused by missing licensing information for new (or previously undiscovered) components.
                  3. Please contact Adam Dawidziuk (adam@cubiware.com) immediately! Please provide full debug log (above) AND a file
                     [%s]
                  4. Please DO NOT continue building any software deliverables until all errors are corrected by Cubiware.
                     This will usually mean pulling-in an updated Src/license_checker repo.
                  5. Please DO NOT modify/remove this script from your build path and/or its data files in order to get around these errors. Contact Adam Dawidziuk first.
                  6. Please DO NOT try to single-handedly define/modify licensing information for any new and existing components. Contact Adam Dawidziuk first.
                  7. Please DO NOT under any circumstances release to ANYONE any software deliverables that were made while any errors from this script were present.
                  
                  
                  
              ''' % (dbfile)
        sys.exit(1)

    print "Software licenses seem OK."
    # Explicitly return success if no errors occurred
    sys.exit(0)
