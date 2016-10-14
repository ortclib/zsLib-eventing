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

#pragma once

#include <zsLib/eventing/types.h>

#include <map>

namespace zsLib
{
  namespace eventing
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IEventingTypes
    #pragma mark

    interaction IEventingTypes
    {
      ZS_DECLARE_STRUCT_PTR(Provider);
      ZS_DECLARE_STRUCT_PTR(Channel);
      ZS_DECLARE_STRUCT_PTR(Task);
      ZS_DECLARE_STRUCT_PTR(OpCode);
      ZS_DECLARE_STRUCT_PTR(DataTemplate);
      ZS_DECLARE_STRUCT_PTR(DataType);
      ZS_DECLARE_STRUCT_PTR(Event);

      typedef String ChannelID;
      typedef std::map<ChannelID, ChannelPtr> ChannelMap;
      ZS_DECLARE_PTR(ChannelMap);

      typedef String Name;
      typedef std::map<Name, TaskPtr> TaskMap;
      ZS_DECLARE_PTR(TaskMap);

      typedef std::map<Name, OpCodePtr> OpCodeMap;
      ZS_DECLARE_PTR(OpCode);

      typedef String TypedefName;
      typedef String PredefinedTypedefName;
      typedef std::map<TypedefName, PredefinedTypedefName> TypedefMap;

      typedef String EventName;
      typedef std::map<EventName, EventPtr> EventMap;

      typedef String Hash;
      typedef std::map<Hash, DataTemplatePtr> DataTemplateMap;

      typedef std::list<DataTypePtr> DataTypeList;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark IEventingTypes::OperationalTypes
      #pragma mark

      enum OperationalTypes
      {
        OperationalType_First,

        OperationalType_Admin = OperationalType_First,
        OperationalType_Operational,
        OperationalType_Analytic,
        OperationalType_Debug,

        OperationalType_Last = OperationalType_Debug,
      };

      static const char *toString(OperationalTypes type);
      static OperationalTypes toOperationalTypes(const char *type) throw (InvalidArgument);

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark IEventingTypes::PredefinedOpCodes
      #pragma mark

      enum PredefinedOpCodes
      {
        PredefinedOpCode_First = 0,

        PredefinedOpCode_Info = PredefinedOpCode_First,
        PredefinedOpCode_Start = 1,
        PredefinedOpCode_Stop = 2,
        PredefinedOpCode_DC_Start = 3,
        PredefinedOpCode_DC_Stop = 4,
        PredefinedOpCode_Extension = 5,
        PredefinedOpCode_Reply = 6,
        PredefinedOpCode_Resume = 7,
        PredefinedOpCode_Suspend = 8,
        PredefinedOpCode_Send = 9,
        PredefinedOpCode_Receive = 240,

        PredefinedOpCode_Last = PredefinedOpCode_Receive,
      };

      static const char *toString(PredefinedOpCodes code);
      static PredefinedOpCodes toPredefinedOpCode(const char *code) throw (InvalidArgument);

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark IEventingTypes::PredefinedLevels
      #pragma mark

      enum PredefinedLevels
      {
        PredefinedLevel_First = 1,

        PredefinedLevel_Critical = PredefinedLevel_First,
        PredefinedLevel_Error = 2,
        PredefinedLevel_Warning = 3,
        PredefinedLevel_Informational = 4,
        PredefinedLevel_Verbose = 5,

        PredefinedLevel_Last = PredefinedLevel_Verbose,
      };

      static const char *toString(PredefinedLevels level);
      static PredefinedLevels toPredefinedLevel(const char *level) throw (InvalidArgument);
      static PredefinedLevels toPredefinedLevel(
                                                Log::Severity severity,
                                                Log::Level level
                                                );

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark IEventingTypes::BaseTypedefs
      #pragma mark

      enum BaseTypes
      {
        BaseType_First,

        BaseType_Boolean = BaseType_First,
        BaseType_Integer,
        BaseType_Float,
        BaseType_Pointer,
        BaseType_Binary,
        BaseType_String,

        BaseType_Last = BaseType_String,
      };

      static const char *toString(BaseTypes type);
      static BaseTypes toBaseType(const char *type) throw (InvalidArgument);

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark IEventingTypes::PredefinedTypedefs
      #pragma mark

      enum PredefinedTypedefs
      {
        PredefinedTypedef_First,

        PredefinedTypedef_boolean = PredefinedTypedef_First,

        PredefinedTypedef_uchar,
        PredefinedTypedef_char,
        PredefinedTypedef_schar,
        PredefinedTypedef_ushort,
        PredefinedTypedef_short,
        PredefinedTypedef_sshort,
        PredefinedTypedef_uint,
        PredefinedTypedef_int,
        PredefinedTypedef_sint,
        PredefinedTypedef_ulong,
        PredefinedTypedef_long,
        PredefinedTypedef_slong,
        PredefinedTypedef_ulonglong,
        PredefinedTypedef_longlong,
        PredefinedTypedef_slonglong,
        PredefinedTypedef_uint8,
        PredefinedTypedef_int8,
        PredefinedTypedef_sint8,
        PredefinedTypedef_uint16,
        PredefinedTypedef_int16,
        PredefinedTypedef_sint16,
        PredefinedTypedef_uint32,
        PredefinedTypedef_int32,
        PredefinedTypedef_sint32,
        PredefinedTypedef_uint64,
        PredefinedTypedef_int64,
        PredefinedTypedef_sint64,

        PredefinedTypedef_byte,
        PredefinedTypedef_word,
        PredefinedTypedef_dword,
        PredefinedTypedef_qword,

        PredefinedTypedef_float,
        PredefinedTypedef_double,
        PredefinedTypedef_float32,
        PredefinedTypedef_float64,

        PredefinedTypedef_ptr,

        PredefinedTypedef_binary,
        PredefinedTypedef_size,

        PredefinedTypedef_string,
        PredefinedTypedef_astring,
        PredefinedTypedef_wstring,

        PredefinedTypedef_Last = PredefinedTypedef_wstring,
      };

      static const char *toString(PredefinedTypedefs type);
      static PredefinedTypedefs toPredefinedTypedef(const char *type);
      static PredefinedTypedefs toPreferredPredefinedTypedef(PredefinedTypedefs type);

      static BaseTypes getBaseType(PredefinedTypedefs type);
      static bool isSigned(PredefinedTypedefs type);
      static size_t getMinBits(PredefinedTypedefs type);
      static size_t getMaxBits(PredefinedTypedefs type);

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark IEventingTypes::Provider
      #pragma mark

      struct Provider
      {
        GUID mGUID {};
        String mName;
        String mResourceName;

        TypedefMap mTypedefs;
        ChannelMap mChannels;
        TaskMap mTasks;
        DataTemplateMap mTemplates;
      };

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark IEventingTypes::Channel
      #pragma mark

      struct Channel
      {
        ChannelID         mID;
        Name              mName; // Company-Product-Component
        OperationalTypes  mType {OperationalType_Last};
      };

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark IEventingTypes::Task
      #pragma mark

      struct Task
      {
        Name mName;
        OperationalTypes mType {OperationalType_Last};
      };

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark IEventingTypes::OpCode
      #pragma mark

      struct OpCode
      {
        Name mName;
      };

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark IEventingTypes::Event
      #pragma mark

      struct Event
      {
        Name mName;
        ChannelID mChannel;

        Log::Severity mSeverity {Log::Informational};
        Log::Level mLevel {Log::None};
        Name mTask;
        Hash DataTemplatePtr;
      };

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark IEventingTypes::DataTemplate
      #pragma mark

      struct DataTemplate
      {
        DataTypeList mDataTypes;
      };

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark IEventingTypes::DataTemplate
      #pragma mark

      struct DataType
      {
        PredefinedTypedefs mType {PredefinedTypedef_First};
        String mName;
      };
    };

  } // namespace eventing
} // namespace zsLib
