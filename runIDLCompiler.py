import os
import sys

dir_path = os.path.dirname(os.path.realpath(__file__))

inputArray=sys.argv

idlPath=inputArray[1]
idlAlreadyCompletedFlagFile=inputArray[2]
toolchainCPU=inputArray[3]
idlOutputPath=inputArray[4]
idlLanguages=inputArray[5]
#tempToolchain=toolchain.split(":")
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
print "runIDLCompiler - idlCompilationFPath: " + idlCompilationFlagPath
if not os.path.isfile(idlCompilationFlagPath):
  print("Running idl compilation")

  compilerFullPath = getCompilerPath()
  if (compilerFullPath==None):
    sys.exit("Idl compiler doesn't exist")
    
  #compilerNewPath = os.getcwd() + "/" + compilerPath;
  #if not os.path.isfile(compilerNewPath):
  #  compilerNewPath = os.getcwd() + "/" + hostCPU + "/" + compilerPath;
  #  if not os.path.isfile(compilerNewPath):
  #    sys.exit("Idl compiler doesn't exist")
      
  os.chdir(os.path.dirname(idlPath))
  jsonFile=os.path.basename(idlPath)

  commandPath = compilerFullPath + " -idl " + idlLanguages + " -c " + jsonFile + " -o " + idlOutputPath

  print "runIDLCompiler - idlPath: " + idlPath
  print "runIDLCompiler - jsonFile: " + jsonFile
  print "runIDLCompiler - idlAlreadyCompletedFlagFile: " + idlAlreadyCompletedFlagFile
  print "runIDLCompiler - NewWorkingPath:" + os.getcwd()
  print "runIDLCompiler - compilerFullPath: " + compilerFullPath
  print "runIDLCompiler - command: " + commandPath

  result=os.system(commandPath)
  if (result!=0):
    sys.exit("Failed idl compilation" + str(result))
    
  open(idlCompilationFlagPath,'w').close()
  os.chdir(os.path.dirname(currentWorkingPath))
else:
  print("Idls have been already compiled")
