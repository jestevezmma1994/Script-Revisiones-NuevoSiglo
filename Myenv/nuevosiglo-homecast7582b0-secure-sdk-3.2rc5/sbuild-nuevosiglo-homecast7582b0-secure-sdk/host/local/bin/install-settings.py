#!/usr/bin/python
import os, sys, commands

try:
    destDir = sys.argv[1]
    basicFormat = sys.argv[2]
    settings = sys.argv[3:]
except:
    print >> sys.stderr, "Need to provide destination dir as first argument, basic resolution as second argument and then filenames."
    sys.exit(0)
    
formatsString = os.environ.get('Q_RESOURCES_CUBITV_FORMAT')
if formatsString == None:
    print >> sys.stderr, "No Q_RESOURCES_CUBITV_FORMAT env set."
    sys.exit(1)
formats = formatsString.split(",")

buildRoot = os.environ.get('BUILD_ROOT')
schema1=buildRoot + "/usr/share/xml/settings-v1.xsd"
schema11=buildRoot + "/usr/share/xml/settings-v1-1.xsd"
schema12=buildRoot + "/usr/share/xml/settings-v1-2.xsd"
schema15=buildRoot + "/usr/share/xml/settings-v1-5.xsd"
schema2=buildRoot + "/usr/share/xml/settings-v2.xsd"

basicResolution = float(basicFormat.rstrip("p"))

for format in formats:
    os.system("install -d " + destDir + format)

for settingPath in settings:
    if os.system("grep -q 'http://cubiware.com/settings-v2.xsd' " + settingPath + " >/dev/null") == 0:
        schema = schema2
    elif os.system("grep -q 'http://cubiware.com/settings-v1-5.xsd' " + settingPath + " >/dev/null") == 0:
        schema = schema15
    elif os.system("grep -q 'http://cubiware.com/settings-v1-2.xsd' " + settingPath + " >/dev/null") == 0:
        schema = schema12
    elif os.system("grep -q 'http://cubiware.com/settings-v1-1.xsd' " + settingPath + " >/dev/null") == 0:
        schema = schema11
    else:
        schema = schema1

    ret, output = commands.getstatusoutput("xmllint --noout --schema " + schema + " " + settingPath + " 2>&1")
    if ret != 0:
        print >> sys.stderr, output
        sys.exit(1)
        
    installedSettingRelativePath = (settingPath.split(basicFormat + "/")).pop(1)
    dirIdx = installedSettingRelativePath.rfind("/")
    for format in formats:
        installString = "install -m 0644 " + settingPath + " " + destDir + format
        
        #handle subpaths
        if dirIdx > 0:
            os.system("install -d " + destDir + format + "/" + installedSettingRelativePath[:dirIdx])
            installString += "/" + installedSettingRelativePath[:dirIdx]
            
        if os.system(installString):
            print >> sys.stderr, "Install " + settingPath + " to dir " + destDir + format + " failed."
            sys.exit(1)
        if format != basicFormat:
            resolutionScale = float(format.rstrip("p")) / basicResolution
            ret, output = commands.getstatusoutput("python " + buildRoot + "/bin/replace.py " + destDir + format + "/" + installedSettingRelativePath + " 0 " + str(resolutionScale) + " 2>&1")
            if ret != 0:
                print >> sys.stderr, "Settings scale for file" + destDir + format + "/" + installedSettingRelativePath + " failed.\n" + output
                sys.exit(1)
