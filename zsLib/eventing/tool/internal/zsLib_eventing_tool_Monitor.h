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

#include <zsLib/eventing/tool/internal/types.h>

#include <zsLib/eventing/tool/ICommandLine.h>

#include <zsLib/eventing/IRemoteEventing.h>
#include <zsLib/eventing/IEventingTypes.h>

#include <zsLib/Log.h>
#include <zsLib/Singleton.h>

namespace zsLib
{
  namespace eventing
  {
    namespace tool
    {
      namespace internal
      {
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark Monitor
        #pragma mark
        
        class Monitor : public MessageQueueAssociator,
                        public ISingletonManagerDelegate,
                        public IRemoteEventingDelegate,
                        public ILogEventingDelegate
        {
        protected:
          struct make_private {};
          
        protected:
          void init();
          
          static MonitorPtr create(const ICommandLineTypes::MonitorInfo &monitorInfo);
          static MonitorPtr singleton(const ICommandLineTypes::MonitorInfo *monitorInfo = NULL);
          
        public:
          typedef zsLib::Log::Severity Severity;
          typedef zsLib::Log::Level Level;
          typedef zsLib::Log::EventingAtomIndex EventingAtomIndex;
          
          ZS_DECLARE_TYPEDEF_PTR(IEventingTypes::Provider, Provider);
          ZS_DECLARE_TYPEDEF_PTR(IEventingTypes::Event, Event);
          typedef std::map<UUID, ProviderPtr> ProviderMap;
          
          typedef size_t ValueID;
          typedef std::map<ValueID, EventPtr> EventMap;
          
          struct ProviderInfo
          {
            ProviderPtr mExistingProvider;
            UUID mProviderID;
            String mProviderName;
            String mProviderUniqueHash;
            EventMap mEvents;
          };

          typedef std::set<ProviderInfo *> ProviderInfoSet;

        public:
          Monitor(
                  const make_private &,
                  IMessageQueuePtr queue,
                  const ICommandLineTypes::MonitorInfo &monitorInfo
                  );
          ~Monitor();

          //-------------------------------------------------------------------
          #pragma mark
          #pragma mark Monitor => (friends)
          #pragma mark
          
          static void monitor(const ICommandLineTypes::MonitorInfo &monitorInfo);
          static void interrupt();

        protected:
          //-------------------------------------------------------------------
          #pragma mark
          #pragma mark Monitor::ISingletonManagerDelegate
          #pragma mark
          
          virtual void notifySingletonCleanup() override;

          //-------------------------------------------------------------------
          #pragma mark
          #pragma mark Monitor::IRemoteEventingDelegate
          #pragma mark

          virtual void onRemoteEventingStateChanged(
                                                    IRemoteEventingPtr connection,
                                                    States state
                                                    ) override;
          
          virtual void onRemoteEventingAnnounceRemoteSubsystem(
                                                               IRemoteEventingPtr connection,
                                                               const char *subsystemName
                                                               ) override;
          
          
          virtual void onRemoteEventingLocalDroppedEvents(
                                                          IRemoteEventingPtr connection,
                                                          size_t totalDropped
                                                          ) override;
          virtual void onRemoteEventingRemoteDroppedEvents(
                                                           IRemoteEventingPtr connection,
                                                           size_t totalDropped
                                                           ) override;

          //-------------------------------------------------------------------
          #pragma mark
          #pragma mark Monitor::ILogEventingDelegate
          #pragma mark

          // (ignored) virtual void notifyNewSubsystem(zsLib::Subsystem &inSubsystem) {}
          
          // notification of a log event
          virtual void notifyWriteEvent(
                                        ProviderHandle handle,
                                        EventingAtomDataArray eventingAtomDataArray,
                                        Severity severity,
                                        Level level,
                                        LOG_EVENT_DESCRIPTOR_HANDLE descriptor,
                                        LOG_EVENT_PARAMETER_DESCRIPTOR_HANDLE paramDescriptor,
                                        LOG_EVENT_DATA_DESCRIPTOR_HANDLE dataDescriptor,
                                        size_t dataDescriptorCount
                                        ) override;

        protected:
          //-------------------------------------------------------------------
          #pragma mark
          #pragma mark Monitor => (internal)
          #pragma mark

          void internalInterrupt();
          void cancel();
          bool shouldQuit() const { return mShouldQuit; }

        protected:
          //-------------------------------------------------------------------
          #pragma mark
          #pragma mark Monitor => (data)
          #pragma mark

          mutable RecursiveLock mLock;
          AutoPUID mID;

          MonitorWeakPtr mThisWeak;
          MonitorPtr mGracefulShutdownReference;
          ICommandLineTypes::MonitorInfo mMonitorInfo;
          
          EventingAtomIndex mEventingAtom;

          ProviderMap mProviders;
          ProviderInfoSet mCleanProviderInfos;
          
          std::atomic<bool> mShouldQuit {false};
          std::atomic<size_t> mTotalEventsDropped {};
          std::atomic<size_t> mTotalEvents {};

          IRemoteEventingPtr mRemote;
        };

      } // namespace internal
    } // namespace tool
  } // namespace eventing
} // namespace zsLib
