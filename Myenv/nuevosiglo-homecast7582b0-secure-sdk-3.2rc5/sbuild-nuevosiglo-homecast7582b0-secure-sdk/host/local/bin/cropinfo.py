#!/usr/bin/python

import os
import sys
import subprocess
import argparse
import glob
import tempfile
import re
import xml.etree.ElementTree as ET
from argparse import RawTextHelpFormatter

verbose = False

def errorMessage(msg):
    print '\033[1;31mcropinfo.py >> '+msg+'\033[1;m'

def getFilesFromDirectory(directory, prefix, sufix):
    files = []
    for extension in ('png', 'webp'):
        files.extend(glob.glob(directory+'/'+prefix+'*'+sufix+'.'+extension))
    return set(f.split('/')[-1] for f in files)

def imageCoordinates(directory, file):
    name, extension = os.path.splitext(file)
    tmpPath = tempfile.mktemp(suffix = extension)
    fullFile = directory + '/' + file
    proces = subprocess.Popen(['convert', '-verbose', '-trim', fullFile, tmpPath], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    answer = proces.communicate()
    if proces.returncode != 0 :
        errorMessage('ERROR with convert bitmap ' + file)
        sys.exit(1)
    os.unlink(tmpPath)
    match = re.search(r'([0-9]+)x([0-9]+)=>([0-9]+)x([0-9]+) ([0-9]+x[0-9]+)\+([0-9]+)\+([0-9]+)', ' '.join(answer))
    if verbose:
        print file, match.group(0)
    # offset x, offset y , width, height
    return [int(match.group(6)), int(match.group(7)), int(match.group(3)), int(match.group(4)), int(match.group(1)), int(match.group(2))]

def perImageCoordinates(directory, files):
    for f in files:
        ans = imageCoordinates(directory, f)
        print '\n%s' % f
        print 'offsetX       : %d' % ans[0]
        print 'offsetY       : %d' % ans[1]
        print 'clippedWidth  : %d' % ans[2]
        print 'clippedHeight : %d' % ans[3]

def generateJSON(directory, files, json) :
    out = None
    if json == 'stdout' :
        out = sys.stdout
    else :
        out = open(json, "w+")
    out.write("{\n")
    first = True
    for f in files:
        if first :
            first = False
        else :
            out.write(",\n")
        ans = imageCoordinates(directory, f)
        out.write("    \"icon/" + f + "\" : {\n")
        out.write("        \"__jsonclass__\": [\n            \"QBIconCropInfo\",\n            []\n        ],\n")
        out.write("        \"offsetX\": " + str(ans[0]) + ",\n")
        out.write("        \"offsetY\": " + str(ans[1]) + ",\n")
        out.write("        \"clippedWidth\": " + str(ans[2]) + ",\n")
        out.write("        \"clippedHeight\": " + str(ans[3]) + ",\n")
        out.write("        \"orginalWidth\": " + str(ans[4]) + ",\n")
        out.write("        \"orginalHeight\": " + str(ans[5]) + "\n")
        out.write("    }")
    out.write("\n}\n")

def getCoordinates(directory, files):
    ret = []
    for f in files:
        ans = imageCoordinates(directory, f)
        ret.append(ans)
    return ret

def getIconNamesFromAppmenus(directory, onlySmall, onlyBig):
    # ugly
    nameSpace ='{http://cubiware.com/activetree-v1.xsd}'
    ans = set()
    for appmenuFile in glob.glob(directory+'/*.appmenu'):
        try:
            tree = ET.parse(appmenuFile)
            iter = tree.getroot()
            for elem in iter:
                if elem.tag == nameSpace+'node':
                    for child in elem.getchildren():
                        if 'name' in child.attrib:
                            name = child.attrib['name']
                            if (name == 'icon' and not onlySmall) or (name == 'iconSmall' and not onlyBig):
                                if verbose: print name, '-->', child.text
                                ans.add(str(child.text).split('/')[-1])
        except ET.ParseError as error:
            print 'ERROR %s: %s' % (appmenuFile, str(error))
    return ans

def findTrim(coordinates):
    if verbose: print 'coordinates: ', coordinates
    if len(coordinates) == 0:
        return (0,0,0,0)
    a = list(coordinates)
    if len(a) == 1:
        return a[0]
    ans = a[0]
    # offset X, offset Y, width, height
    for cX, cY, cW, cH, w, h in a[1:] :
        ansX, ansY, ansW, ansH = ans[0:4]
        newX = cX if cX < ansX else ansX
        newY = cY if cY < ansY else ansY

        endRight = max(ansX + ansW, cX + cW)
        newW = endRight - newX

        endBottom = max(ansY + ansH, cY + cH)
        newH = endBottom - newY

        ans = newX, newY, newW, newH

    if verbose: print 'ans: ', ans
    return ans

parser = argparse.ArgumentParser(
    formatter_class=RawTextHelpFormatter,
    description ="""
Program is used to calculate 'the best' trimmed rectangle which contain all
specified images.""",
    epilog ="""
example - calculate coordinates for icons containing '_nonfocus' sufix:
%s -i /tmp/720p/icon/ -a /tmp/CubiTV-Generic/settings/ -s _nonfocus
example - calculate coordinates for icons containing '_focus' sufix:
%s -i /tmp/720p/icon/ -a /tmp/CubiTV-Generic/settings/ -s _focus
example -  calculate coordinates for big icons only:
%s -i /tmp/720p/icon/ -a /tmp/CubiTV-Generic/settings/ --only-big
example -  calculate coordinates for small icons only:
%s -i /tmp/720p/icon/ -a /tmp/CubiTV-Generic/settings/ --only-small
""" % (sys.argv[0], sys.argv[0], sys.argv[0], sys.argv[0]))

parser.add_argument('-i', '--icons-dir',help='directory which contain icon images', required=True)
parser.add_argument('-a', '--appmenus-dir', help='directory which contain *.appmenu files', required=True)
parser.add_argument('-p', '--prefix', help='prefix of process files', required=False, default='')
parser.add_argument('-s', '--suffix', help='sufix of process files', required=False, default='')
parser.add_argument('-v', '--verbose', help='verbose mode', required=False, action='store_true')
parser.add_argument(      '--only-big', help='analyze only big icons', required=False, default=False, action='store_true')
parser.add_argument(      '--only-small', help='analyze only small icons', required=False, default=False, action='store_true')
parser.add_argument(      '--per-image', help='analyze and print each icon separately', required=False, default=False, action='store_true')
parser.add_argument('-j', '--json', help='generate JSON file', required=False, default=None)

args = vars(parser.parse_args())

iconDirectory = args['icons_dir']
prefix = args['prefix']
sufix = args['suffix']
appmenuDir = args['appmenus_dir']
verbose = args['verbose']
onlySmall = args['only_small']
onlyBig = args['only_big']
per_image = args['per_image']
json = args['json']
files = getFilesFromDirectory(iconDirectory, prefix, sufix)
icons = getIconNamesFromAppmenus(appmenuDir, onlySmall, onlyBig)
toTrimFiles = files.intersection(icons)

if json is not None :
    generateJSON(iconDirectory, toTrimFiles, json)
    sys.exit()

if per_image :
    perImageCoordinates(iconDirectory, toTrimFiles)
else :
    coordinates = getCoordinates(iconDirectory, toTrimFiles)
    ans = findTrim(coordinates)
    if ans != (0,0,0,0):
        print 'offsetX       : %d\n' % ans[0],
        print 'offsetY       : %d\n' % ans[1],
        print 'clippedWidth  : %d\n' % ans[2],
        print 'clippedHeight : %d\n' % ans[3],
