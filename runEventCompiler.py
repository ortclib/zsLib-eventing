import os
import sys


dir_path = os.path.dirname(os.path.realpath(__file__))
inputArray=sys.argv

eventJsonPath=inputArray[1]
idlAlreadyCompletedFlagFile=inputArray[2]
toolchainCPU=inputArray[3]
idlOutputPath=inputArray[4]
hostCPU=inputArray[5]


def getCompilerPath():
  compilerNewPath = os.getcwd() + "/" + compilerPath;
  if os.path.isfile(compilerNewPath):
    return compilerNewPath
  
  compilerNewPath = os.getcwd() + "/" + toolchainCPU + "/" + compilerPath;
  if os.path.isfile(compilerNewPath):
    return compilerNewPath
  
  compilerNewPath = os.getcwd() + "/" + hostCPU + "/" + compilerPath;
  if os.path.isfile(compilerNewPath):
    return compilerNewPath
    
  return
    

if (os.name == "posix"):
  compilerPath="zslib-eventing-tool-compiler"
else:
  compilerPath="zslib-eventing-tool-compiler.exe"
  
eventCompilationPath=os.path.join(dir_path,idlAlreadyCompletedFlagFile)

if not os.path.isfile(eventCompilationPath):
  print("Running events compilation...")
  
  compilerFullPath = getCompilerPath()
  if (compilerFullPath==None):
    sys.exit("Idl compiler doesn't exist")
  
  os.chdir(os.path.dirname(eventJsonPath))
  eventJsonNewPath = os.path.join(os.getcwd(),os.path.basename(eventJsonPath))
 
  commandToExecute = compilerFullPath + " -c " + eventJsonNewPath + " -o " + idlOutputPath
  
  result = os.system(commandToExecute)
  
  if result == 0:
    open(eventCompilationPath,'w').close()
  else:
    sys.exit("Failed events compilation with error code " + str(result))
  
else:
  print("Events " + eventJsonPath + " have been already compiled")
