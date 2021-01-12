#!/usr/bin/env python3

#############################################################################
# TiVo Poland Sp. z o.o. Software License Version 1.0
#
# Copyright (C) 2008-2019 TiVo Poland Sp. z o.o. All rights reserved.
#
# Any rights which are not expressly granted in this License are entirely and
# exclusively reserved to and by TiVo Poland Sp. z o.o. You may not rent, lease,
# modify, translate, reverse engineer, decompile, disassemble, or create
# derivative works based on this Software. You may not make access to this
# Software available to others in connection with a service bureau,
# application service provider, or similar business, or make any other use of
# this Software without express written permission from TiVo Poland Sp. z o.o.
#
# Any User wishing to make use of this Software must contact
# TiVo Poland Sp. z o.o. to arrange an appropriate license. Use of the Software
# includes, but is not limited to:
# (1) integrating or incorporating all or part of the code into a product for
#     sale or license by, or on behalf of, User to third parties;
# (2) distribution of the binary or source code to third parties for use with
#     a commercial product sold or licensed by, or on behalf of, User.
#############################################################################

import argparse
import itertools
import logging
import os
import subprocess
import sys


def check_file(file_path, min_size_kb):
    """Check if file should be converted.

    This function checks if file is a png (by extension) and if its size is
    greater or equal to min_size_kb kilobytes.
    """
    return (os.path.isfile(file_path) and
            file_path.endswith('.png') and
            os.path.getsize(file_path) >= (1024 * min_size_kb))


def find_large_png_files(dir_path, min_size_kb):
    """A generator which finds large png files recursively in a single directory.

    This function crawls recursively through a directory and finds all PNG files
    that are at least min_size_kb kilobytes in size.
    """
    for name in os.listdir(dir_path):
        path = os.path.join(dir_path, name)
        if os.path.isdir(path):
            yield from find_large_png_files(path, min_size_kb)
        elif check_file(path, min_size_kb):
            yield path


def find_large_png_files_in_locations(dirs_or_files_list, min_size_kb):
    """A generator which processes the list of directories or files and finds large png files there.

    For every element of input list (dirs_or_files_list) crawl it recursively to
    find large png files (if it is a directory), or check if it is a large png file
    itself. A large file is of size min_size_kb kilobytes or greater.
    """
    for path in dirs_or_files_list:
        if os.path.isdir(path):
            yield from find_large_png_files(path, min_size_kb)
        elif check_file(path, min_size_kb):
            yield path


def convert_png_to_webp(options, path):
    """Run the conversion function and return @c True if the conversion was successful."""
    result = subprocess.call(["cwebp"] + options + [path, "-o", path])
    return result == 0


def convert_all_multiprocess(options, file_paths):
    """Convert all files from the list to webp format, using passed conversion options, with a pool of worker processes."""
    from multiprocessing import Pool
    from functools import partial
    convert_func = partial(convert_png_to_webp, options)
    with Pool(3) as pool:
        return all(pool.map(convert_func, file_paths))


def main():
    if subprocess.call(["which", "cwebp"]) != 0:
        logging.error("this program requires cwebp program to work properly. "
                      "Please install libwebp first.")
        return 1

    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument("min_size_kb", type=int,
                           help="minimal size of file for conversion (in KB). "
                                "Smaller files will not be converted to webp")
    arg_parser.add_argument("quality", type=int,
                           help="conversion quality in range [0-100] (100 is lossless)")
    arg_parser.add_argument("dirs_or_files", nargs="+",
                           help="list of directories to recursively crawl and find pngs, "
                                "or list of files for conversion, or mixed")
    args = arg_parser.parse_args()

    file_paths = find_large_png_files_in_locations(args.dirs_or_files, args.min_size_kb)

    options = ["-q", str(args.quality), "alpha_q", "100"] if args.quality < 100 else ["-lossless"]

    return 0 if convert_all_multiprocess(options, file_paths) else 1


if __name__ == '__main__':
    sys.exit(main())
