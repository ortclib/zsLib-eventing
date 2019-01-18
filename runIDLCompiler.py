import os
import sys

dir_path = os.path.dirname(os.path.realpath(__file__))

inputArray=sys.argv

idlPath=inputArray[1]
idlAlreadyCompletedFlagFile=inputArray[2]
toolchainCPU=inputArray[3]
idlOutputPath=inputArray[4]
idlLanguages=inputArray[5]
hostCPU=inputArray[6]

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

currentWorkingPath=os.getcwd()
pathname = os.path.dirname(sys.argv[0]) 

idlCompilationFlagPath=dir_path + "/" + idlAlreadyCompletedFlagFile

if not os.path.isfile(idlCompilationFlagPath):
  print("Running idls compilation...")

  compilerFullPath = getCompilerPath()
  if (compilerFullPath==None):
    sys.exit("Idl compiler doesn't exist")
    
  os.chdir(os.path.dirname(idlPath))
  jsonFile=os.path.basename(idlPath)

  commandToExecute = compilerFullPath + " -idl " + idlLanguages + " -c " + jsonFile + " -o " + idlOutputPath

  print("commandToExecute:" + commandToExecute)
  
  result = os.system(commandToExecute)
  
  if result == 0:
    open(idlCompilationFlagPath,'w').close()
  else:
    sys.exit("Failed idls compilation with error code " + str(result))
    
else:
  print("Idls have been already compiled")
