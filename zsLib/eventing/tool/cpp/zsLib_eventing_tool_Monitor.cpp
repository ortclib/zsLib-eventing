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

#include <zsLib/eventing/tool/internal/zsLib_eventing_tool_Monitor.h>
#include <zsLib/eventing/tool/internal/zsLib_eventing_tool_Compiler.h>

#include <zsLib/eventing/tool/OutputStream.h>
#include <zsLib/eventing/IHelper.h>
#include <zsLib/eventing/Log.h>

#include <zsLib/IMessageQueueManager.h>
#include <zsLib/Numeric.h>

namespace zsLib { namespace eventing { namespace tool { ZS_DECLARE_SUBSYSTEM(zsLib_eventing_tool) } } }

namespace zsLib
{
  namespace eventing
  {
    namespace tool
    {
      ZS_DECLARE_USING_PTR(zsLib::eventing, IHelper);
      
      typedef eventing::USE_EVENT_DESCRIPTOR USE_EVENT_DESCRIPTOR;
      typedef eventing::USE_EVENT_PARAMETER_DESCRIPTOR USE_EVENT_PARAMETER_DESCRIPTOR;
      typedef eventing::USE_EVENT_DATA_DESCRIPTOR USE_EVENT_DATA_DESCRIPTOR;
      
      namespace internal
      {
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark (helpers)
        #pragma mark
        
        //---------------------------------------------------------------------
        static bool hasSingleton(bool nowHaveSingle = false)
        {
          static bool hasSingleton {};
          if (nowHaveSingle) hasSingleton = true;
          return hasSingleton;
        }
        
        //---------------------------------------------------------------------
        static uint64_t getUnsignedValue(USE_EVENT_DATA_DESCRIPTOR &data)
        {
          if (!data.Ptr) return 0;
          
          switch (data.Size) {
            case 1: {
              uint8_t value = 0;
              memcpy(&value, (const void *)(data.Ptr), sizeof(uint8_t));
              return value;
            }
            case 2: {
              uint16_t value = 0;
              memcpy(&value, (const void *)(data.Ptr), sizeof(uint16_t));
              return value;
            }
            case 4: {
              uint32_t value = 0;
              memcpy(&value, (const void *)(data.Ptr), sizeof(uint32_t));
              return value;
            }
            case 8: {
              uint64_t value = 0;
              memcpy(&value, (const void *)(data.Ptr), sizeof(uint64_t));
              return value;
            }
            default: {
              uint64_t value = 0;
              memcpy(&value, (const void *)(data.Ptr), sizeof(uint64_t) > data.Size ? data.Size : sizeof(uint64_t));
              return value;
            }
          }
          return 0;
        }
        
        //---------------------------------------------------------------------
        static int64_t getSignedValue(USE_EVENT_DATA_DESCRIPTOR &data)
        {
          if (!data.Ptr) return 0;
          
          switch (data.Size) {
            case 1: {
              int8_t value = 0;
              memcpy(&value, (const void *)(data.Ptr), sizeof(int8_t));
              return value;
            }
            case 2: {
              int16_t value = 0;
              memcpy(&value, (const void *)(data.Ptr), sizeof(int16_t));
              return value;
            }
            case 4: {
              int32_t value = 0;
              memcpy(&value, (const void *)(data.Ptr), sizeof(int32_t));
              return value;
            }
            case 8: {
              int64_t value = 0;
              memcpy(&value, (const void *)(data.Ptr), sizeof(int64_t));
              return value;
            }
            default: {
              int64_t value = 0;
              memcpy(&value, (const void *)(data.Ptr), sizeof(int64_t) > data.Size ? data.Size : sizeof(int64_t));
              return value;
            }
          }
          return 0;
        }
        
        //---------------------------------------------------------------------
        static double getFloatValue(USE_EVENT_DATA_DESCRIPTOR &data)
        {
          if (!data.Ptr) return 0.0f;
          
          if (sizeof(float) == data.Size) {
            float value {};
            memcpy(&value, (const void *)(data.Ptr), sizeof(value));
            return value;
          }
          if (sizeof(double) == data.Size) {
            double value ={};
            memcpy(&value, (const void *)(data.Ptr), sizeof(value));
            return value;
          }
          
          double value = 0;
          memcpy(&value, (const void *)(data.Ptr), sizeof(double) > data.Size ? data.Size : sizeof(double));
          return value;
        }
        
        //---------------------------------------------------------------------
        static String valueAsString(
                                    USE_EVENT_PARAMETER_DESCRIPTOR &param,
                                    USE_EVENT_DATA_DESCRIPTOR &data,
                                    bool &outIsNumber
                                    )
        {
          outIsNumber = true;
          
          switch (param.Type) {
            case EventParameterType_Boolean:          {
              outIsNumber = false;
              if (0 != getUnsignedValue(data)) return "true";
              return "false";
            }
            case EventParameterType_UnsignedInteger:  return string(getUnsignedValue(data));
            case EventParameterType_SignedInteger:    return string(getSignedValue(data));
            
            case EventParameterType_FloatingPoint:    return string(getFloatValue(data));
            case EventParameterType_Pointer:          return string(getUnsignedValue(data));
            case EventParameterType_AString:          {
              outIsNumber = false;
              if (!data.Ptr) return String();
              if (0 == data.Size) return String();
              auto temp = IHelper::convertToBuffer(reinterpret_cast<const BYTE *>(data.Ptr), data.Size);
              return String(reinterpret_cast<const char *>(temp->BytePtr()));
            }
            case EventParameterType_WString:          {
              outIsNumber = false;
              if (!data.Ptr) return String();
              if (0 == data.Size) return String();
              size_t total = data.Size / sizeof(wchar_t);
              
              if (0 == total) return String();
              
              wchar_t *temp = new wchar_t[total+1] {};
              memcpy(temp, (const void *)(data.Ptr), data.Size);
              
              String result(&(temp[0]));
              
              delete [] temp;
              temp = NULL;
              
              return result;
            }
            case EventParameterType_Binary:
            default:
            {
              break;
            }
          }

          outIsNumber = false;
          if (!data.Ptr) return String();
          if (0 == data.Size) return String();
          return IHelper::convertToHex(reinterpret_cast<const BYTE *>(data.Ptr), data.Size);
        }
        
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark Monitor
        #pragma mark
        
        //---------------------------------------------------------------------
        Monitor::Monitor(
                         const make_private &,
                         IMessageQueuePtr queue,
                         const ICommandLineTypes::MonitorInfo &monitorInfo
                         ) :
          MessageQueueAssociator(queue),
          mMonitorInfo(monitorInfo),
          mEventingAtom(zsLib::Log::registerEventingAtom("org.zsLib.eventing.tool.Monitor"))
        {
          for (auto iter = monitorInfo.mJMANFiles.begin(); iter != monitorInfo.mJMANFiles.end(); ++iter) {
            auto fileName = (*iter);
            
            ProviderPtr provider;
            SecureByteBlockPtr jmanRaw;
            
            try {
              jmanRaw = IHelper::loadFile(fileName);
            } catch (const StdError &e) {
              ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_FILE_FAILED_TO_LOAD, String("Failed to load jman file: ") + fileName + ", error=" + string(e.result()) + ", reason=" + e.message());
            }
            if (!jmanRaw) {
              ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_FILE_FAILED_TO_LOAD, String("Failed to load jman file: ") + fileName);
            }

            auto rootEl = IHelper::read(jmanRaw);

            try {
              provider = Provider::create(rootEl);
            } catch (const InvalidContent &e) {
              ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, "Failed to parse jman file: " + e.message());
            }
            if (!provider) {
              ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_FILE_FAILED_TO_LOAD, "Failed to parse jman file: " + fileName);
            }
            
            auto found = mProviders.find(provider->mID);
            if (found != mProviders.end()) {
              ZS_THROW_CUSTOM_PROPERTIES_1(Failure, ZS_EVENTING_TOOL_INVALID_CONTENT, "Duplicate provider found in jman file: " + fileName);
            }

            mProviders[provider->mID] = provider;
          }
          
          if (mMonitorInfo.mOutputJSON) {
            tool::output() << "{ \"events\": { \"event\": [\n";
          }
        }
        
        //---------------------------------------------------------------------
        Monitor::~Monitor()
        {
          mThisWeak.reset();
          
          for (auto iter = mCleanProviderInfos.begin(); iter != mCleanProviderInfos.end(); ++iter) {
            delete (*iter);
          }
          mCleanProviderInfos.clear();
        }
        
        //---------------------------------------------------------------------
        void Monitor::init()
        {
          // set-up remote
          {
            AutoRecursiveLock lock(mLock);
            if (mMonitorInfo.mIPAddress.isAddressEmpty()) {
              mRemote = IRemoteEventing::listenForRemote(mThisWeak.lock(), mMonitorInfo.mPort, mMonitorInfo.mSecret);
              if (!mMonitorInfo.mQuietMode) {
                tool::output() << "[Info] Listening for remote connection: " << string(mMonitorInfo.mPort) << "\n";
              }
            } else {
              mRemote = IRemoteEventing::connectToRemote(mThisWeak.lock(), mMonitorInfo.mIPAddress, mMonitorInfo.mSecret);
              if (!mMonitorInfo.mQuietMode) {
                tool::output() << "[Info] Connecting to remote process: " << mMonitorInfo.mIPAddress.string() << "\n";
              }
            }
            
            if (!mRemote) {
              cancel();
              return;
            }
            
            for (auto iter = mProviders.begin(); iter != mProviders.end(); ++iter) {
              auto provider = (*iter).second;
              for (auto iterSubsystem = provider->mSubsystems.begin(); iterSubsystem != provider->mSubsystems.end(); ++iterSubsystem) {
                auto subsystem = (*iterSubsystem).second;
                mRemote->setRemoteLevel(subsystem->mName, subsystem->mLevel);
              }
            }
          }

          if (mMonitorInfo.mOutputJSON) {
            Log::addEventingListener(mThisWeak.lock());
          }
        }

        //---------------------------------------------------------------------
        MonitorPtr Monitor::create(const ICommandLineTypes::MonitorInfo &monitorInfo)
        {
          auto queue = IMessageQueueManager::getMessageQueue("org.zsLib.eventing.tool.Monitor");
          auto pThis(make_shared<Monitor>(make_private{}, queue, monitorInfo));
          pThis->mThisWeak = pThis;
          pThis->init();
          return pThis;
        }

        //---------------------------------------------------------------------
        MonitorPtr Monitor::singleton(const ICommandLineTypes::MonitorInfo *monitorInfo)
        {
          AutoRecursiveLock lock(*IHelper::getGlobalLock());
          hasSingleton(true);
          static SingletonLazySharedPtr<Monitor> pThis(Monitor::create(*monitorInfo));
          static SingletonManager::Register reg("org.zsLib.eventing.tool.Monitor", pThis.singleton());
          return pThis.singleton();
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark Monitor => (friends)
        #pragma mark
        
        //---------------------------------------------------------------------
        void Monitor::monitor(const ICommandLineTypes::MonitorInfo &monitorInfo)
        {
          auto pThis = Monitor::singleton(&monitorInfo);
          if (!pThis) return;
          
          while (!pThis->shouldQuit())
          {
            std::this_thread::sleep_for(Seconds(1));
          }
        }

        //---------------------------------------------------------------------
        void Monitor::interrupt()
        {
          {
            AutoRecursiveLock lock(*IHelper::getGlobalLock());
            if (!hasSingleton()) return;
          }

          auto pThis = Monitor::singleton();
          if (!pThis) return;

          pThis->internalInterrupt();

          while (!pThis->shouldQuit())
          {
            std::this_thread::sleep_for(Seconds(1));
          }
        }
 
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark Monitor::ISingletonManagerDelegate
        #pragma mark
        
        //---------------------------------------------------------------------
        void Monitor::notifySingletonCleanup()
        {
          internalInterrupt();
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark Monitor::IRemoteEventingDelegate
        #pragma mark
        
        //---------------------------------------------------------------------
        void Monitor::onRemoteEventingStateChanged(
                                                   IRemoteEventingPtr connection,
                                                   States state
                                                   )
        {
          if (!mMonitorInfo.mQuietMode) {
            tool::output() << "[Info] Remoting eventing state: " << IRemoteEventing::toString(state) << "\n";
          }

          switch (state) {
            case IRemoteEventingTypes::State_ShuttingDown:
            case IRemoteEventingTypes::State_Shutdown:
            {
              AutoRecursiveLock lock(mLock);
              cancel();
              break;
            }
            default: {
              break;
            }
          }
        }
        
        //---------------------------------------------------------------------
        void Monitor::onRemoteEventingAnnounceRemoteSubsystem(
                                                              IRemoteEventingPtr connection,
                                                              const char *subsystemName
                                                              )
        {
          if (!mMonitorInfo.mQuietMode) {
            tool::output() << "[Info] Remoting eventing subsystem: " << String(subsystemName) << "\n";
          }
        }
        
        //---------------------------------------------------------------------
        void Monitor::onRemoteEventingLocalDroppedEvents(
                                                         IRemoteEventingPtr connection,
                                                         size_t totalDropped
                                                         )
        {
          // ignored
        }

        //---------------------------------------------------------------------
        void Monitor::onRemoteEventingRemoteDroppedEvents(
                                                          IRemoteEventingPtr connection,
                                                          size_t totalDropped
                                                          )
        {
          mTotalEventsDropped = totalDropped;
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark Monitor::ILogEventingDelegate
        #pragma mark
        
        //---------------------------------------------------------------------
        void Monitor::notifyWriteEvent(
                                      ProviderHandle handle,
                                      EventingAtomDataArray eventingAtomDataArray,
                                      Severity severity,
                                      Level level,
                                      LOG_EVENT_DESCRIPTOR_HANDLE inDescriptor,
                                      LOG_EVENT_PARAMETER_DESCRIPTOR_HANDLE inParamDescriptor,
                                      LOG_EVENT_DATA_DESCRIPTOR_HANDLE inDataDescriptor,
                                      size_t dataDescriptorCount
                                      )
        {
          ProviderInfo *provider = reinterpret_cast<ProviderInfo *>(eventingAtomDataArray[mEventingAtom]);

          // process event
          {
            AutoRecursiveLock lock(mLock);
            if (!mRemote) return;

            if (NULL == provider) {
              provider = new ProviderInfo;
              mCleanProviderInfos.insert(provider);

              eventingAtomDataArray[mEventingAtom] = reinterpret_cast<EventingAtomData>(provider);

              if (Log::getEventingWriterInfo(handle, provider->mProviderID, provider->mProviderName, provider->mProviderUniqueHash)) {
                auto found = mProviders.find(provider->mProviderID);
                if (found != mProviders.end()) {
                  
                  auto existingProvider = (*found).second;
                  
                  if (existingProvider->mUniqueHash == provider->mProviderUniqueHash) {
                    provider->mExistingProvider = (*found).second;
                    
                    // process all events into a quick lookup map
                    for (auto iter = provider->mExistingProvider->mEvents.begin(); iter != provider->mExistingProvider->mEvents.end(); ++iter)
                    {
                      auto event = (*iter).second;
                      provider->mEvents[event->mValue] = event;
                    }
                  } else {
                    if (!mMonitorInfo.mQuietMode) {
                      tool::output() << "[Warning] Provider \"" << provider->mProviderName << "\" hashes do not match: X=" << existingProvider->mUniqueHash << " Y=" << provider->mProviderUniqueHash << "\n";
                    }
                  }
                }
              }
            }
          }
          
          String output;
          
          USE_EVENT_DESCRIPTOR *descriptor = (USE_EVENT_DESCRIPTOR *)inDescriptor;
          USE_EVENT_PARAMETER_DESCRIPTOR *paramDescriptor = (USE_EVENT_PARAMETER_DESCRIPTOR *)inParamDescriptor;
          USE_EVENT_DATA_DESCRIPTOR *dataDescriptor = (USE_EVENT_DATA_DESCRIPTOR *)inDataDescriptor;

          if (provider->mEvents.size() > 0) {
            auto found = provider->mEvents.find(descriptor->Id);
            if (found == provider->mEvents.end()) {
              auto event = (*found).second;

              ElementPtr rootEl = Element::create("event");
              rootEl->adoptAsLastChild(IHelper::createElementWithText("severity", Log::toString(severity)));
              rootEl->adoptAsLastChild(IHelper::createElementWithText("level", Log::toString(level)));
              rootEl->adoptAsLastChild(IHelper::createElementWithTextAndJSONEncode("name", event->mName));
              if (event->mChannel) {
                rootEl->adoptAsLastChild(IHelper::createElementWithTextAndJSONEncode("channel", event->mChannel->mID));
              }
              if (event->mTask) {
                rootEl->adoptAsLastChild(IHelper::createElementWithTextAndJSONEncode("task", event->mTask->mName));
              }
              if (event->mOpCode) {
                rootEl->adoptAsLastChild(IHelper::createElementWithTextAndJSONEncode("opCode", event->mOpCode->mName));
              }
              
              ElementPtr valuesEl = Element::create("values");
              rootEl->adoptAsLastChild(valuesEl);

              size_t totalDataParams = ZS_EVENTING_TOTAL_BUILT_IN_EVENT_DATA;

              if (event->mDataTemplate) {
                totalDataParams += event->mDataTemplate->mDataTypes.size();
              }

              if (totalDataParams != dataDescriptorCount) {
                if (!mMonitorInfo.mQuietMode) {
                  tool::output() << "[Warning] Event \"" << event->mName << "\" parameter count does not match: X=" << string(totalDataParams) << " Y=" << string(dataDescriptorCount) << "\n";
                  return;
                }
              }

              for (size_t index = 0; index < ZS_EVENTING_TOTAL_BUILT_IN_EVENT_DATA; ++index)
              {
                bool isNumber = false;
                String valueName("unknown");
                String value = valueAsString(paramDescriptor[index], dataDescriptor[index], isNumber);
                switch (index) {
                  case 0: valueName = "_subsystemName"; break;
                  case 1: valueName = "_function"; break;
                  case 2: valueName = "_line"; break;
                }
                
                if (isNumber) {
                  valuesEl->adoptAsLastChild(IHelper::createElementWithNumber(valueName, value));
                } else {
                  valuesEl->adoptAsLastChild(IHelper::createElementWithTextAndJSONEncode(valueName, value));
                }
              }

              if (event->mDataTemplate) {
                auto iterDataType = event->mDataTemplate->mDataTypes.begin();
                for (size_t index = ZS_EVENTING_TOTAL_BUILT_IN_EVENT_DATA; index < dataDescriptorCount; ++index, ++iterDataType)
                {
                  if (iterDataType == event->mDataTemplate->mDataTypes.end()) break;
                  
                  auto dataType = (*iterDataType);

                  bool isNumber = false;
                  String valueName = dataType->mValueName;
                  String value = valueAsString(paramDescriptor[index], dataDescriptor[index], isNumber);

                  if (isNumber) {
                    valuesEl->adoptAsLastChild(IHelper::createElementWithNumber(valueName, value));
                  } else {
                    valuesEl->adoptAsLastChild(IHelper::createElementWithTextAndJSONEncode(valueName, value));
                  }
                }
              }

              output = IHelper::toString(rootEl);
            }
          }
          
          if (!output.hasData()) {
            ElementPtr rootEl = Element::create("event");
            rootEl->adoptAsLastChild(IHelper::createElementWithText("severity", Log::toString(severity)));
            rootEl->adoptAsLastChild(IHelper::createElementWithText("level", Log::toString(level)));
            rootEl->adoptAsLastChild(IHelper::createElementWithNumber("name", string(descriptor->Id)));
            rootEl->adoptAsLastChild(IHelper::createElementWithNumber("channel", string(descriptor->Channel)));
            rootEl->adoptAsLastChild(IHelper::createElementWithNumber("task", string(descriptor->Task)));
            rootEl->adoptAsLastChild(IHelper::createElementWithNumber("opCode", string(descriptor->Opcode)));

            ElementPtr valuesEl = Element::create("values");
            rootEl->adoptAsLastChild(valuesEl);

            for (size_t index = 0; index < ZS_EVENTING_TOTAL_BUILT_IN_EVENT_DATA; ++index)
            {
              bool isNumber = false;
              String valueName("unknown");
              String value = valueAsString(paramDescriptor[index], dataDescriptor[index], isNumber);
              switch (index) {
                case 0: valueName = "_subsystemName"; break;
                case 1: valueName = "_function"; break;
                case 2: valueName = "_line"; break;
              }
              
              if (isNumber) {
                valuesEl->adoptAsLastChild(IHelper::createElementWithNumber(valueName, value));
              } else {
                valuesEl->adoptAsLastChild(IHelper::createElementWithTextAndJSONEncode(valueName, value));
              }
            }
            
            for (size_t index = ZS_EVENTING_TOTAL_BUILT_IN_EVENT_DATA; index < dataDescriptorCount; ++index)
            {
              bool isNumber = false;
              String valueName = string(index-ZS_EVENTING_TOTAL_BUILT_IN_EVENT_DATA);
              String value = valueAsString(paramDescriptor[index], dataDescriptor[index], isNumber);
              
              if (isNumber) {
                valuesEl->adoptAsLastChild(IHelper::createElementWithNumber(valueName, value));
              } else {
                valuesEl->adoptAsLastChild(IHelper::createElementWithTextAndJSONEncode(valueName, value));
              }
            }

            output = IHelper::toString(rootEl);
          }

          if (output.hasData()) {
            AutoRecursiveLock lock(mLock);
            tool::output() << ",\n" << output;
          }
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark Monitor => (internal)
        #pragma mark

        //---------------------------------------------------------------------
        void Monitor::internalInterrupt()
        {
          AutoRecursiveLock lock(mLock);
          cancel();
        }

        //---------------------------------------------------------------------
        void Monitor::cancel()
        {
          if (mShouldQuit) return;
          
          auto pThis = mThisWeak.lock();
          mGracefulShutdownReference = pThis;

          if (mRemote) {
            mRemote->shutdown();
          }

          if (mGracefulShutdownReference) {
            if (mRemote) {
              auto state = mRemote->getState();
              if (IRemoteEventingTypes::State_Shutdown != state) return;
            }
          }
          
          if (mMonitorInfo.mOutputJSON) {
            Log::removeEventingListener(pThis);
          }
          
          mRemote.reset();

          if (mMonitorInfo.mOutputJSON) {
            tool::output() << "\n] } }\n";
          }
          if (!mMonitorInfo.mQuietMode) {
            tool::output() << "\n";
            tool::output() << "[Info] Total events dropped: " << string(mTotalEventsDropped) << "\n";
            tool::output() << "[Info] Total events received: " << string(mTotalEvents) << "\n";
          }
          mShouldQuit = true;

          mGracefulShutdownReference.reset();
        }

      }
    }
  }
}
