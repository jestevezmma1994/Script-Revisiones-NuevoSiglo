#!/usr/bin/python
import logging
from optparse import OptionParser
import sys

try:
    import simplejson as json
except ImportError:
    import json

if sys.version_info < (2,7,0):
	from ordereddict import OrderedDict
else:
	from collections import OrderedDict

class DictDiffer(object):
    """
    Calculate the difference between two dictionaries as:
    (1) items added
    (2) items removed
    (3) keys same in both but changed values
    (4) keys same in both and unchanged values
    """
    def __init__(self, current_dict, past_dict):
        self.current_dict, self.past_dict = current_dict, past_dict
        self.set_current, self.set_past = set(current_dict.keys()), set(past_dict.keys())
        self.intersect = self.set_current.intersection(self.set_past)
    def added(self):
        return self.set_current - self.intersect 
    def removed(self):
        return self.set_past - self.intersect 
    def changed(self):
        return set(o for o in self.intersect if self.past_dict[o] != self.current_dict[o])
    def unchanged(self):
        return set(o for o in self.intersect if self.past_dict[o] == self.current_dict[o])


def sanityCheck(objects,constObjects):
    differ = DictDiffer(objects, constObjects)
    changed = differ.changed()
    if changed != None and len(changed)>0:
        print "There are differences between const_conf.json and conf.json. Please fix it!"
        print "Keys that differ (key, const, default):"
        for key in changed:
            print key
            print ">>>", constObjects[key]
            print "<<<", objects[key]
            print "===="
        sys.exit(1)

def exportAll(objects,key):
	for obj in objects:
		if isinstance(objects[obj],dict):
			subkey = str(key + obj + '_')
			exportAll(objects[obj],subkey)
		elif isinstance(objects[obj], unicode) or isinstance(objects[obj], str):
			print 'export ' + key + obj + '=' + "'" + objects[obj].replace("'","'\"'\"'") + "'"
		else:
			logging.error("Unsupported type for key: %s, %s", str(key), type(objects[obj]))

def printKey(objects,key):
	if '.' in str(key):
		firstDot = str(key).find('.')
		if firstDot == len(key)-1 or firstDot == 0:
			logging.error("Unproper key format, key: %s", str(key))
		substr = key[:firstDot]
		if substr in objects:	
			if isinstance(objects[substr], dict):
				printKey(objects[substr],key[firstDot+1:])
			elif isinstance(objects[key], unicode) or isinstance(objects[key], str):
				logging.error("This key: %s does not have more subkeys", str(key))
			else:
				logging.error("Unsupported type for key: %s", str(key))
	else: 
		if key in objects:
			if isinstance(objects[key], dict):
				logging.error("this key: %s have more subkeys", str(key))
			elif isinstance(objects[key], unicode) or isinstance(objects[key], str):
				print unicode(objects[key])
			else:
				logging.error("Unsupported type for key: %s", str(key))

def changeKey(objects,key,value):
	if '.' in str(key):
		firstDot = str(key).find('.')
		if firstDot == len(key)-1 or firstDot == 0:
			logging.error("Unproper key format, key: %s", str(key))
		substr = key[:firstDot]
		if substr in objects:	
			if isinstance(objects[substr], dict):
				changeKey(objects[substr],key[firstDot+1:],value)
			elif isinstance(objects[key], unicode) or isinstance(objects[key], str):
				logging.error("This key: %s does not have more subkeys", str(key))
			else:
				logging.error("Unsupported type for key: %s", str(key))
	else: 
		if key in objects:
			if isinstance(objects[key], dict):
				logging.error("this key: %s have more subkeys", str(key))
			elif isinstance(objects[key], unicode) or isinstance(objects[key], str):
				#print 'old value ' + ('' if objects[key] is None else objects[key])
				objects[key] = str(value)
				#print 'new value ' + ('' if objects[key] is None else objects[key])
			else:
				logging.error("Unsupported type for key: %s", str(key))

#main program
parser = OptionParser()
parser.add_option("-i", dest="filename", help="input filename")
parser.add_option("-c", dest="const", help="const option filename")
parser.add_option("-n", dest="noescape", help="do not escape values", action="store_true", default=False)

(options, args) = parser.parse_args()

optName = None
optValue = None
if len(args) > 0:
	optName = args[0]
	if not options.noescape:
		optName = optName.replace('_', '.')

if len(args) > 1:
	optValue = args[1]

file = open(options.filename, "r")
inputbuffer =  file.read()
file.close

const_inputbuffer = None

if options.const != None:
    const_file = open(options.const, "r")
    const_inputbuffer =  const_file.read()
    const_file.close

jsonObjects = json.loads(inputbuffer, object_pairs_hook=OrderedDict)
if const_inputbuffer != None:
    constObjects = json.loads(const_inputbuffer, object_pairs_hook=OrderedDict)
    # Do the sanity check - for each key make sure it's the same in both places
    sanityCheck(jsonObjects, constObjects)

if len(args) == 0:
	exportAll(jsonObjects,'')
elif len(args) == 1:
	printKey(jsonObjects,optName)
elif len(args) == 2:
	changeKey(jsonObjects, optName, optValue)
	outputFile = open(options.filename, "w")
	outputFile.write(json.dumps(jsonObjects, indent = 4))
	outputFile.close()


