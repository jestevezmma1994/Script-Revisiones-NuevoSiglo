#!/usr/bin/env python
import upgrade_catalog_sections2ts
import upgrade_catalog_mksections
import upgrade_catalog_scan
import upgrade_catalog_set_force_flag
import ota_config

from optparse import OptionParser
import tarfile
import tempfile
import fnmatch
import shutil
import traceback
import sys
import os


def upgrade_package_to_catalog_name(package_name):
    # remove .tgz extension
    extracted_dir_name = os.path.splitext(package_name)[0]
    # split by dashes
    extracted_dir_name = extracted_dir_name.split("-")
    # remove "upgrade" word
    extracted_dir_name.remove("upgrade")
    # concatenate back into name
    extracted_dir_name = "-".join(extracted_dir_name)
    return extracted_dir_name


def prepare_upgrade_package(sbuild_path, upgrade_file, field, data, forced):
    dest_dir = os.path.dirname(upgrade_file)
    tmp_dir = tempfile.mkdtemp()
    print >> sys.stderr, "Created tmp work directory: %s" % tmp_dir
    extracted_dir = os.path.join(tmp_dir, upgrade_package_to_catalog_name(os.path.basename(upgrade_file)))
    # removing _signed suffix (case for CAS signed images like: Latens, VMX, etc.)
    extracted_dir = extracted_dir.replace("_signed", "")

    os.mkdir(extracted_dir, 0o700)
    print >> sys.stderr, "Unpacking upgrade file %s" % os.path.basename(upgrade_file)
    upgrade_tgz = tarfile.open(upgrade_file)
    upgrade_tgz.extractall(extracted_dir)
    upgrade_tgz.close()

    # file names and paths for other python scripts
    output_file_name = os.path.splitext(os.path.basename(upgrade_file))[0] + "_" + ota_config.upgrade_stream_types[field]

    if forced:
        output_file_name += "_forced"
    output_file_path = os.path.join(tmp_dir, output_file_name)
    tmp_catalog_file = os.path.join(tmp_dir, "catalog")

    if any("output_file_extension" in field for field in data["general"]):
        output_file_extension = data["general"]["output_file_extension"]
    else:
        output_file_extension = "ts"

    try:
        upgrade_catalog_scan.scan_upgrade_catalog(tmp_catalog_file, extracted_dir, sbuild_path)
        if forced:
            upgrade_catalog_set_force_flag.set_force_flag(tmp_catalog_file)
        upgrade_catalog_mksections.make_sections(tmp_catalog_file, output_file_path + ".sec")
        upgrade_catalog_sections2ts.sections_to_ts(tmp_catalog_file, output_file_path + ".sec", sbuild_path,
                                                   output_file_path + "." + output_file_extension, field, data)

    except RuntimeError:
        print >> sys.stderr, 'Error! Removing tmp directory %s' % tmp_dir
        shutil.rmtree(tmp_dir)
        raise

    print >> sys.stderr, "Moving %s* file to destination directory: %s" % (output_file_path + "." + output_file_extension, dest_dir)
    dest = os.path.join(dest_dir, output_file_name) + "." + output_file_extension
    if os.path.exists(dest):
        os.remove(dest)
    shutil.move(output_file_path + "." + output_file_extension, dest)
    if os.path.exists(dest + ".info"):
        os.remove(dest + ".info")
    shutil.move(output_file_path + "." + output_file_extension + ".info", dest + ".info")

    print >> sys.stderr, "Work done, removing tmp directory %s" % tmp_dir
    shutil.rmtree(tmp_dir)
    print >> sys.stderr, ""


def generate_upgrade_streams_from_sbuild(sbuild_path):
    # check if JSON file exists and retrieve contents
    success, data = ota_config.open_config(os.path.join(sbuild_path, 'target', 'bSpec', 'OTA.json'))
    if not success:
        raise RuntimeError("No JSON config file, aborting process")

    # find upgrade files and for each prepare transport stream
    print >> sys.stderr, "Preparing upgrade stream for directory: %s" % sbuild_path
    for file_ in os.listdir(sbuild_path):
        if fnmatch.fnmatch(file_, "*upgrade*.tgz"):
            print >> sys.stderr, "Upgrade package found: %s" % file_
            for field in data:
                if field in ota_config.upgrade_stream_types:
                    print >> sys.stderr, "Found field %s (short name %s), preparing upgrade package" % (
                        field, ota_config.upgrade_stream_types[field])

                    file_path = os.path.join(sbuild_path, file_)
                    prepare_upgrade_package(sbuild_path, file_path, field, data, forced=True)
                    prepare_upgrade_package(sbuild_path, file_path, field, data, forced=False)


def generate_upgrade_streams_from_upgrade_tgz(sbuild_path, upgrade_package):
    # check if JSON file exists and retrieve contents
    success, data = ota_config.open_config(os.path.join(sbuild_path, 'target', 'bSpec', 'OTA.json'))
    if not success:
        raise RuntimeError("No JSON config file, aborting process")

    for field in data:
        if field in ota_config.upgrade_stream_types:
            print >> sys.stderr, "Found field %s (short name %s), preparing upgrade package" % (
                field, ota_config.upgrade_stream_types[field])

            prepare_upgrade_package(sbuild_path, upgrade_package, field, data, forced=False)


def main():
    #
    # List of files used from SBUILD_PATH:
    # * target/root/etc/ARCH
    # * target/root/etc/SELECTOR
    # * target/root/bSpec/OTA.json
    #

    usage = "usage: %prog [options] SBUILD_PATH"
    parser = OptionParser(usage=usage)
    parser.add_option("-u", "--upgrade-file", dest="upgrade_file", default="",
                  help="Optional path to upgrade file (when specified using it instead of looking for upgrade files inside sbuild)", metavar="FILE")

    # Python 2.7+ feature, leaving for future...
    # parser.add_argument("sbuild_path", help="Path to sbuilds directory or specific sbuild")
    (options, args) = parser.parse_args()

    if len(args) < 1:
        raise RuntimeError("create_upgrade_streams.py: invalid usage, try --help option")

    if not os.path.exists(args[0]):
        raise RuntimeError("Incorrect path! Please enter path to an existing directory.")

    if options.upgrade_file != "" and not fnmatch.fnmatch(options.upgrade_file, "*upgrade*.tgz"):
        raise RuntimeError("Incorrect upgrade package path. ")

    print >> sys.stderr, "\nStarting OTA Upgrade creation script..."

    selected_dir_full = os.path.realpath(args[0])
    print >> sys.stderr, ""
    print >> sys.stderr, "Full path given: %s" % selected_dir_full

    selected_dir = os.path.basename(selected_dir_full)

    if options.upgrade_file != "":
        print >> sys.stderr, "Upgrade file given, creating upgrade streams only for this package"
        generate_upgrade_streams_from_upgrade_tgz(selected_dir_full, options.upgrade_file)
    elif selected_dir.startswith("sbuild"):
        print >> sys.stderr, "Path given to sbuild, creating only one set of upgrade streams"
        generate_upgrade_streams_from_sbuild(selected_dir_full)
    else:
        print >> sys.stderr, "Not an sbuild! Searching for sbuilds...\n"
        found_flag = False
        for filename in os.listdir(selected_dir_full):
            if filename.startswith("sbuild"):
                found_flag = True
                print >> sys.stderr, "Found sbuild: %s" % filename
                try:
                    generate_upgrade_streams_from_sbuild(os.path.join(selected_dir_full, filename))
                except RuntimeError:
                    print >> sys.stderr, ''
                    print >> sys.stderr, '-'*30 + "ERROR" + '-'*30
                    traceback.print_exc()
                    print >> sys.stderr, '-'*65
                    print >> sys.stderr, 'Continuing for other packages...'
                    print >> sys.stderr, ''

        if not found_flag:
            raise RuntimeError("No sbuild found! Please give a proper path")


if __name__ == "__main__":
    try:
        main()
    except Exception:
        traceback.print_exc()
        sys.exit(1)
