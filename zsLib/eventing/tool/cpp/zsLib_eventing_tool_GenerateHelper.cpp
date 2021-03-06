/*

Copyright (c) 2016, Robin Raymond
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.

*/

#include <zsLib/eventing/tool/internal/zsLib_eventing_tool_GenerateHelper.h>
//#include <zsLib/eventing/tool/internal/zsLib_eventing_tool_Helper.h>
//
#include <zsLib/eventing/tool/OutputStream.h>
//
#include <zsLib/eventing/IHelper.h>
//#include <zsLib/eventing/IHasher.h>
//#include <zsLib/eventing/IEventingTypes.h>
//
//#include <zsLib/Exception.h>
//#include <zsLib/Numeric.h>
//
#include <sstream>
//#include <list>
//#include <set>
//#include <cctype>

namespace zsLib { namespace eventing { namespace tool { ZS_DECLARE_SUBSYSTEM(zslib_eventing_tool) } } }

namespace zsLib
{
  namespace eventing
  {
    ZS_DECLARE_TYPEDEF_PTR(IIDLTypes::Project, Project);

    namespace tool
    {
      ZS_DECLARE_TYPEDEF_PTR(eventing::IHelper, UseHelper);
      ZS_DECLARE_TYPEDEF_PTR(eventing::IHasher, UseHasher);
      typedef std::set<String> HashSet;

      namespace internal
      {
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //
        // GenerateHelper
        //


        //---------------------------------------------------------------------
        String GenerateHelper::getDashedComment(const String &indent)
        {
          std::stringstream ss;
          ss << indent << "//";
          size_t length = 2 + indent.length();
          if (length < 80) {
            for (size_t index = 0; index < (80 - length); ++index) {
              ss << "-";
            }
          }
          ss << "\n";
          return ss.str();
        }

        //---------------------------------------------------------------------
        String GenerateHelper::getDocumentation(
                                                const String &linePrefix,
                                                ContextPtr context,
                                                size_t maxLineLength
                                                )
        {
          if (!context) return String();
          if (!context->mDocumentation) return String();

          std::stringstream ss;

          ElementPtr childEl = context->mDocumentation->getFirstChildElement();
          while (childEl)
          {
            auto xmlStr = UseHelper::toString(childEl, false);

            if (xmlStr.hasData()) {
              if (String::npos != xmlStr.find("<code>")) {
                UseHelper::SplitMap split;
                UseHelper::split(xmlStr, split, "\n");
                for (auto iter = split.begin(); iter != split.end(); ++iter)
                {
                  ss << linePrefix << (*iter).second << "\n";
                }
                return ss.str();
              }

              auto startElPos = xmlStr.find('>');
              auto endElPos = xmlStr.rfind('<');
              if ((String::npos != startElPos) &&
                  (String::npos != endElPos) &&
                  (endElPos > startElPos)) {
                String openElStr = xmlStr.substr(0, startElPos + 1);
                String endElStr = xmlStr.substr(endElPos);

                xmlStr = xmlStr.substr(startElPos + 1, xmlStr.length() - openElStr.length() - endElStr.length());
                xmlStr.replaceAll("\t", " ");
                xmlStr.replaceAll("\n", " ");
                xmlStr.replaceAll("\r", " ");
                xmlStr.replaceAll("\v", " ");

                ss << linePrefix << openElStr << "\n";

                UseHelper::SplitMap split;
                UseHelper::split(xmlStr, split, " ");
                UseHelper::splitTrim(split);
                UseHelper::splitPruneEmpty(split);

                size_t totalWordsOutput = 0;
                String currentStr;

                for (auto iter = split.begin(); iter != split.end(); ++iter)
                {
                  auto wordStr = (*iter).second;

                  String sep(totalWordsOutput > 0 ? String(" ") : String());

                  if (linePrefix.length() + currentStr.length() + sep.length() + wordStr.length() > maxLineLength) {
                    if (0 == totalWordsOutput) {
                      ss << linePrefix << wordStr << "\n";
                      continue;
                    }
                    ss << linePrefix << currentStr << "\n";
                    totalWordsOutput = 0;
                    currentStr = String();
                    sep = String();
                  }

                  currentStr += sep;
                  currentStr += wordStr;
                  ++totalWordsOutput;
                }
                if (totalWordsOutput > 0) {
                  ss << linePrefix << currentStr << "\n";
                }

                ss << linePrefix << endElStr << "\n";
              } else {
                ss << linePrefix << xmlStr << "\n";
              }
            }
            childEl = childEl->getNextSiblingElement();
          }

          return ss.str();
        }

        //---------------------------------------------------------------------
        void GenerateHelper::insertFirst(
                                         std::stringstream &ss,
                                         bool &first
                                         )
        {
          if (!first) return;
          first = false;
          ss << "\n";
        }

        //---------------------------------------------------------------------
        void GenerateHelper::insertLast(
                                        std::stringstream &ss,
                                        bool &first
                                        )
        {
          if (first) return;
          ss << "\n";
        }

        //---------------------------------------------------------------------
        void GenerateHelper::insertBlob(
                                        std::stringstream &ss,
                                        const String &indentStr,
                                        const char *blob,
                                        bool blankLineHasIndent
                                        )
        {
          if (NULL == blob) return;

          bool lastWasEol {true};

          const char *p = blob;
          while ('\0' != *p)
          {
            if (lastWasEol) {
              if ((blankLineHasIndent) || ('\n' != *p))
                ss << indentStr;
            }
            if ('\n' == *p) {
              ss << *p;
              lastWasEol = true;
              ++p;
              continue;
            }
            lastWasEol = false;
            ss << *p;
            ++p;
          }
        }

        //---------------------------------------------------------------------
        bool GenerateHelper::isBuiltInType(TypePtr type)
        {
          if (!type) return false;

          type = type->getOriginalType();
          if (!type) return false;

          {
            auto basicType = type->toBasicType();
            if (basicType) return true;
          }

          {
            auto structObj = type->toStruct();
            if (structObj) {
              if (!structObj->hasModifier(Modifier_Special)) return false;
              String specialName = structObj->getPathName();
              if ("::zs::Any" == specialName) return true;
              if ("::zs::Promise" == specialName) return true;
              if ("::zs::PromiseWith" == specialName) return true;
              if ("::zs::PromiseRejectionReason" == specialName) return true;

              // check exceptions
              {
                auto exceptionList = GenerateHelper::getAllExceptions("::zs::exceptions::");
                for (auto iter = exceptionList.begin(); iter != exceptionList.end(); ++iter) {
                  String e = (*iter);
                  if (e == specialName) return true;
                }
              }

              if ("::zs::Time" == specialName) return true;
              if ("::zs::Milliseconds" == specialName) return true;
              if ("::zs::Microseconds" == specialName) return true;
              if ("::zs::Nanoseconds" == specialName) return true;
              if ("::zs::Seconds" == specialName) return true;
              if ("::zs::Minutes" == specialName) return true;
              if ("::zs::Hours" == specialName) return true;
              if ("::zs::Days" == specialName) return true;
              if ("::std::set" == specialName) return true;
              if ("::std::list" == specialName) return true;
              if ("::std::map" == specialName) return true;
              return false;
            }
          }

          {
            auto templatedStruct = type->toTemplatedStructType();
            if (templatedStruct) {
              auto parent = templatedStruct->getParent();
              if (parent) return isBuiltInType(parent->toStruct());
            }
          }
          return false;
        }

        //-------------------------------------------------------------------
        bool GenerateHelper::hasOnlyStaticMethods(StructPtr structObj)
        {
          if (!structObj) return true;

          for (auto iter = structObj->mIsARelationships.begin(); iter != structObj->mIsARelationships.end(); ++iter) {
            auto relatedObj = (*iter).second;
            if (!relatedObj) continue;

            auto relatedStruct = relatedObj->toStruct();
            if (!relatedStruct) continue;

            bool only = hasOnlyStaticMethods(relatedStruct);
            if (!only) return false;
          }

          for (auto iter = structObj->mMethods.begin(); iter != structObj->mMethods.end(); ++iter) {
            auto method = (*iter);
            if (method->hasModifier(Modifier_Static)) continue;
            return false;
          }
          for (auto iter = structObj->mProperties.begin(); iter != structObj->mProperties.end(); ++iter) {
            auto method = (*iter);
            if (method->hasModifier(Modifier_Static)) continue;
            return false;
          }
          return true;
        }


        //-------------------------------------------------------------------
        bool GenerateHelper::hasEventHandlers(StructPtr structObj)
        {
          if (!structObj) return false;

          for (auto iter = structObj->mMethods.begin(); iter != structObj->mMethods.end(); ++iter) {
            auto method = (*iter);
            if (method->hasModifier(Modifier_Method_EventHandler)) return true;
          }
          return false;
        }

        //-------------------------------------------------------------------
        bool GenerateHelper::isConstructable(StructPtr structObj)
        {
          if (!structObj) return false;
          if (structObj->hasModifier(Modifier_Struct_NotConstructable)) return false;
          if (structObj->hasModifier(Modifier_Static)) return false;
          return true;
        }

        //-------------------------------------------------------------------
        bool GenerateHelper::needsDefaultConstructor(StructPtr structObj)
        {
          if (!structObj) return false;

          if (!isConstructable(structObj)) return false;
          if (hasOnlyStaticMethods(structObj)) return false;

          for (auto iter = structObj->mMethods.begin(); iter != structObj->mMethods.end(); ++iter)
          {
            auto method = (*iter);
            if (method->hasModifier(Modifier_Method_Ctor)) return false;
          }

          return true;
        }

        //-------------------------------------------------------------------
        bool GenerateHelper::needsDefaultConstructor(TemplatedStructTypePtr templateObj)
        {
          if (!templateObj) return false;

          auto parent = templateObj->getParent();
          if (!parent) return false;
          return needsDefaultConstructor(parent->toStruct());
        }

        //-------------------------------------------------------------------
        String GenerateHelper::getBasicTypeString(IEventingTypes::PredefinedTypedefs type)
        {
          switch (type)
          {
            case PredefinedTypedef_void:        return "void";
            case PredefinedTypedef_bool:        return "bool";
            case PredefinedTypedef_uchar:       return "unsigned char";
            case PredefinedTypedef_char:        return "char";
            case PredefinedTypedef_schar:       return "signed char";
            case PredefinedTypedef_ushort:      return "unsigned short";
            case PredefinedTypedef_short:       return "short";
            case PredefinedTypedef_sshort:      return "signed short";
            case PredefinedTypedef_uint:        return "unsigned int";
            case PredefinedTypedef_int:         return "int";
            case PredefinedTypedef_sint:        return "signed int";
            case PredefinedTypedef_ulong:       return "unsigned long";
            case PredefinedTypedef_long:        return "long";
            case PredefinedTypedef_slong:       return "signed long";
            case PredefinedTypedef_ulonglong:   return "unsigned long long";
            case PredefinedTypedef_longlong:    return "long long";
            case PredefinedTypedef_slonglong:   return "signed long long";
            case PredefinedTypedef_uint8:       return "uint8_t";
            case PredefinedTypedef_int8:        return "int8_t";
            case PredefinedTypedef_sint8:       return "int8_t";
            case PredefinedTypedef_uint16:      return "uint16_t";
            case PredefinedTypedef_int16:       return "int16_t";
            case PredefinedTypedef_sint16:      return "int16_t";
            case PredefinedTypedef_uint32:      return "uint32_t";
            case PredefinedTypedef_int32:       return "int32_t";
            case PredefinedTypedef_sint32:      return "int32_t";
            case PredefinedTypedef_uint64:      return "uint64_t";
            case PredefinedTypedef_int64:       return "int64_t";
            case PredefinedTypedef_sint64:      return "int64_t";

            case PredefinedTypedef_byte:        return "uint8_t";
            case PredefinedTypedef_word:        return "uint16_t";
            case PredefinedTypedef_dword:       return "uint32_t";
            case PredefinedTypedef_qword:       return "uint64_t";

            case PredefinedTypedef_float:       return "float";
            case PredefinedTypedef_double:      return "double";
            case PredefinedTypedef_ldouble:     return "long double";
            case PredefinedTypedef_float32:     return "float";
            case PredefinedTypedef_float64:     return "double";

            case PredefinedTypedef_pointer:     return "uint64_t";

            case PredefinedTypedef_binary:      return "SecureByteBlockPtr";
            case PredefinedTypedef_size:        return "uint64_t";

            case PredefinedTypedef_string:      return "String";
            case PredefinedTypedef_astring:     return "String";
            case PredefinedTypedef_wstring:     return "::std::wstring";
          }
          return String();
        }

        //-------------------------------------------------------------------
        String GenerateHelper::getBasicTypeString(BasicTypePtr type)
        {
          if (!type) return String();
          return getBasicTypeString(type->mBaseType);
        }

        //-------------------------------------------------------------------
        String GenerateHelper::getConverstionNameString(IEventingTypes::PredefinedTypedefs type)
        {
          switch (type)
          {
            case PredefinedTypedef_void:        return "void";
            case PredefinedTypedef_bool:        return "bool";
            case PredefinedTypedef_uchar:       return "uchar";
            case PredefinedTypedef_char:        return "char";
            case PredefinedTypedef_schar:       return "schar";
            case PredefinedTypedef_ushort:      return "ushort";
            case PredefinedTypedef_short:       return "short";
            case PredefinedTypedef_sshort:      return "sshort";
            case PredefinedTypedef_uint:        return "uint";
            case PredefinedTypedef_int:         return "int";
            case PredefinedTypedef_sint:        return "sint";
            case PredefinedTypedef_ulong:       return "ulong";
            case PredefinedTypedef_long:        return "long";
            case PredefinedTypedef_slong:       return "slong";
            case PredefinedTypedef_ulonglong:   return "ulonglong";
            case PredefinedTypedef_longlong:    return "longlong";
            case PredefinedTypedef_slonglong:   return "slonglong";
            case PredefinedTypedef_uint8:       return "uint8_t";
            case PredefinedTypedef_int8:        return "int8_t";
            case PredefinedTypedef_sint8:       return "sint8_t";
            case PredefinedTypedef_uint16:      return "uint16_t";
            case PredefinedTypedef_int16:       return "int16_t";
            case PredefinedTypedef_sint16:      return "sint16_t";
            case PredefinedTypedef_uint32:      return "uint32_t";
            case PredefinedTypedef_int32:       return "int32_t";
            case PredefinedTypedef_sint32:      return "sint32_t";
            case PredefinedTypedef_uint64:      return "uint64_t";
            case PredefinedTypedef_int64:       return "int64_t";
            case PredefinedTypedef_sint64:      return "sint64_t";

            case PredefinedTypedef_byte:        return "byte";
            case PredefinedTypedef_word:        return "word";
            case PredefinedTypedef_dword:       return "dword";
            case PredefinedTypedef_qword:       return "qword";

            case PredefinedTypedef_float:       return "float";
            case PredefinedTypedef_double:      return "double";
            case PredefinedTypedef_ldouble:     return "ldouble";
            case PredefinedTypedef_float32:     return "float32";
            case PredefinedTypedef_float64:     return "float64";

            case PredefinedTypedef_pointer:     return "pointer";

            case PredefinedTypedef_binary:      return "binary";
            case PredefinedTypedef_size:        return "size";

            case PredefinedTypedef_string:      return "string";
            case PredefinedTypedef_astring:     return "astring";
            case PredefinedTypedef_wstring:     return "wstring";
          }
          return String();
        }

        //-------------------------------------------------------------------
        String GenerateHelper::getConverstionNameString(BasicTypePtr type)
        {
          if (!type) return String();
          return getConverstionNameString(type->mBaseType);
        }

        //-------------------------------------------------------------------
        bool GenerateHelper::isSafeIntType(IEventingTypes::PredefinedTypedefs type)
        {
          switch (type)
          {
            case PredefinedTypedef_void:        
            case PredefinedTypedef_bool:        

            case PredefinedTypedef_float:
            case PredefinedTypedef_double:
            case PredefinedTypedef_ldouble:
            case PredefinedTypedef_float32:
            case PredefinedTypedef_float64:

            case PredefinedTypedef_binary:      

            case PredefinedTypedef_string:      
            case PredefinedTypedef_astring:     
            case PredefinedTypedef_wstring:     return false;
            default:                            break;
          }
          return true;
        }

        //-------------------------------------------------------------------
        bool GenerateHelper::isSafeIntType(BasicTypePtr type)
        {
          if (!type) return String();
          return isSafeIntType(type->mBaseType);
        }

        //-------------------------------------------------------------------
        bool GenerateHelper::isFloat(IEventingTypes::PredefinedTypedefs type)
        {
          switch (type)
          {
            case PredefinedTypedef_float:       
            case PredefinedTypedef_double:      
            case PredefinedTypedef_ldouble:     
            case PredefinedTypedef_float32:     
            case PredefinedTypedef_float64:     return true;
            default:                            break;
          }
          return false;
        }

        //-------------------------------------------------------------------
        bool GenerateHelper::isFloat(BasicTypePtr type)
        {
          if (!type) return String();
          return isFloat(type->mBaseType);
        }

        //---------------------------------------------------------------------
        bool GenerateHelper::isDefaultExceptionType(TypePtr type)
        {
          if (!type) return false;

          auto structType = type->toStruct();
          if (!structType) return false;

          if (structType->mGenerics.size() > 0) return false;

          if (!structType->hasModifier(Modifier_Special)) return false;

          String comparison("::zs::exceptions::");
          String specialName = structType->getPathName();

          specialName = specialName.substr(0, comparison.length());

          return comparison == specialName;
        }

        //-------------------------------------------------------------------
        GenerateHelper::StringList GenerateHelper::getAllExceptions(const char *prefix) noexcept
        {
          StringList result;

          String prefixStr(prefix);

          result.push_back(prefixStr + "Exception");
          result.push_back(prefixStr + "InvalidArgument");
          result.push_back(prefixStr + "BadState");
          result.push_back(prefixStr + "SyntaxError");
          result.push_back(prefixStr + "RangeError");
          result.push_back(prefixStr + "ResourceError");
          result.push_back(prefixStr + "UnexpectedError");
          result.push_back(prefixStr + "InvalidUsage");
          result.push_back(prefixStr + "InvalidAssumption");
          result.push_back(prefixStr + "NotImplemented");
          result.push_back(prefixStr + "NotSupported");
          result.push_back(prefixStr + "InvalidModification");
          result.push_back(prefixStr + "NetworkError");
          result.push_back(prefixStr + "InternalError");

          return result;
        }

      } // namespace internal
    } // namespace tool
  } // namespace eventing
} // namespace zsLib
