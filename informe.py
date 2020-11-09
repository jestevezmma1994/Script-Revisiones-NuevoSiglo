#!/usr/bin/ python
import hgapi
import json
import re
import os
import subprocess
import sys
import cgitb 
import inspect

currentdir = os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))
parentdir = os.path.dirname(currentdir)
sys.path.insert(0,parentdir) 

Par1 = sys.argv[1]
#Par2 = sys.argv[2]  
endstring = Par1
beginstring = "silo pull release nuevosiglo-homecast7582b0-secure-sdk-"
allstring =beginstring + endstring
actualdir = os.getcwd()
carpeta = "/nuevosiglo-homecast7582b0-secure-sdk-"
carpeta1 = actualdir + carpeta
carpetafinal = carpeta1 + endstring

os.system(allstring)
#cantcommit = eval(Par2)
listofrepos = [
   #"/App/Applications/CubiTV",
   #"/App/Applications/CubiTV-Bold",
   #"/App/Applications/CubiTV-Generic",
    "/App/Applications/CubiTV-NuevoSiglo",
   #"/App/Components/QBSubsManager",
	#"/App/Components/QBTextSubsManager", 
   "/Files/CubiTV/NuevoSiglo",
    "/Files/Resources/CubiTV",
   #"/Files/Resources/Selectors",
   #"/Harvest/bInstall", 
	#"/Harvest/bSpec/bold-homecast7582b0-debug", 
	#"/Harvest/bSpec/bold-homecast7582b0-secure", 
	#"/Harvest/bSpec/generic",
	#"/Harvest/bSpec/nuevosiglo-hnc2200co-debug", 
	#"/Harvest/bSpec/nuevosiglo-hnc2200co-secure", 
	#"/Harvest/bSpec/nuevosiglo-homecast7582b0-debug", 
    "/Harvest/bSpec/nuevosiglo-homecast7582b0-secure",
   #"/P/nuevosiglo", 
	#"/Platforms/BCM/logo", 
	#"/Src/CxPlayReady-NuevoSiglo",
	#"/Src/TranslationMerger", 
	#"/Src/driver-installer", 
	#"/Src/license_checker" 
   ]

for r in listofrepos:
   orden = carpetafinal+r
   os.chdir(orden)
   repo = hgapi.Repo('.')
   num = repo.hg_rev()
   i = 1
   wpath = './informe1.json'
   mode = 'a' if os.path.exists(wpath) else 'w'
   #mode = 'w'
   listofrevisions = []
   while  i <= 10:
    temp = repo.revision(num)
    del temp.parents
    del temp.tags
    del temp.author
    del temp.date
    txt = temp.desc
    x = re.search("^[[]",txt)
    if x :
      start = "["
      end = "]"
      y = txt[txt.find(start)+len(start):txt.find(end)]
    else :
         y = "empty"
    setattr(temp,'ticket',y)
    json_data = json.dumps(temp.__dict__,indent=4,sort_keys=True,separators=(',', ': ')) 
    listofrevisions.append(json_data)
    num = num - 1
    i += 1
   with open(wpath,mode) as f:
    f.writelines("[")
    for l in listofrevisions:
      if l != listofrevisions[-1]:
       f.writelines("%s," % l)
      else:
         f.writelines("%s" % l)  
         f.writelines("]")
   
  
    
       
    
    
    mothertable = """<html>
<head></head>
<body><p>Lorem ipsum dolor sit amet consectetur adipisicing elit. Distinctio neque est voluptate maiores commodi? Fuga optio, beatae harum numquam fugiat omnis tempore officia voluptas molestias, ipsum, quam accusamus iure eum.</p> 
 </body>
</html>""" 
  
    with open ('/home/jonathan/workspace/script_python/VersionesCambio2.html','wb') as aa:
      aa.write(mothertable)
