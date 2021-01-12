#!/usr/bin/python
import sys, re
from whitelist import whitelist

try:
    xmlfilename = sys.argv[1]
    dryrun = int(sys.argv[2])
    scale = float(sys.argv[3])
except:
    print >>sys.stderr, "Need to provide filename as the first param, 0/1 as the second param and scale factor as third."
    sys.exit(0)

print >> sys.stderr, "Reading:", xmlfilename
print >> sys.stderr, "Dryrun:", dryrun

f = open(xmlfilename)

xmldata = [line for line in f]
f.close()

pattern='.*<param tag="(.*)"><int>(.*)</int></param>'
pattern2='.*<constant name="(.*)"><int>(.*)</int></constant>'
#p = re.compile("<int>.*<int>")
p = re.compile("<int>.*</int>")

if dryrun == 1:
    output = sys.stdout
else:
    output = open(xmlfilename, "w")

for line in xmldata:
    result = re.match(pattern, line)
    if result == None:
    	result = re.match(pattern2, line)
    if result and result.group(1) in whitelist:
        value = result.group(2)
        valueint = int(value)
        finalvalue = str(int(valueint * scale))
        final = p.sub("<int>"+finalvalue+"</int>", line)
        output.write(final)
    else:
        output.write(line)
