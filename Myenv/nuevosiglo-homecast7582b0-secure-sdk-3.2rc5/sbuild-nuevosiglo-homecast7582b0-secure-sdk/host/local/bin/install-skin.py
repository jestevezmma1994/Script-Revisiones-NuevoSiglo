#!/usr/bin/python
import os, sys, commands, PIL.Image, math

try:
    destDir = sys.argv[1]
    basicFormat = sys.argv[2]
    bitmaps = sys.argv[3:]
except:
    print >> sys.stderr, "Need to provide destination dir as first argument, basic resolution as second argument and then filenames."
    sys.exit(0)
    
formatsString = os.environ.get('Q_RESOURCES_CUBITV_FORMAT')
if formatsString == None:
    print >> sys.stderr, "No Q_RESOURCES_CUBITV_FORMAT env set."
    sys.exit(1)
formats = formatsString.split(",")

buildRoot = os.environ.get('BUILD_ROOT')

basicResolution = float(basicFormat.rstrip("p"))

for format in formats:
    os.system("install -d " + destDir + format)

for bitmapPath in bitmaps:      
    installedBitmapRelativePath = (bitmapPath.split(basicFormat + "/")).pop(1)
    dirIdx = installedBitmapRelativePath.rfind("/")
    for format in formats:
        installString = "install -m 0644 " + bitmapPath + " " + destDir + format
        
        #handle subpaths
        if dirIdx > 0:
            os.system("install -d " + destDir + format + "/" + installedBitmapRelativePath[:dirIdx])
            installString += "/" + installedBitmapRelativePath[:dirIdx]
        if os.system(installString):
            print >> sys.stderr, "Install " + bitmapPath + " to dir " + destDir + format + " failed."
            sys.exit(1)
        if format != basicFormat:
            resolutionScale = float(format.rstrip("p")) / basicResolution
            path = destDir + format + "/" + installedBitmapRelativePath
            im = PIL.Image.open(path)
            width, height = im.size
            newWidth = int(math.ceil(width * resolutionScale));
            newHeight = int(math.ceil(height * resolutionScale));
            
            ret, output = commands.getstatusoutput("mogrify -resize " + str(newWidth) + "x" + str(newHeight) + "! " + path)
            if ret != 0:
                print >> sys.stderr, "Image scale with path " + path + " failed.\n" + output
                sys.exit(1)
