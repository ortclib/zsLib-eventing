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

#include <zsLib/ITimer.h>
#include <zsLib/Log.h>
#include <zsLib/Singleton.h>
#include <zsLib/IWakeDelegate.h>

#define ZS_EVENTING_TOTAL_BUILT_IN_EVENT_DATA (5)


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
        //
        // Monitor
        //
        
        class Monitor : public MessageQueueAssociator,
                        public ISingletonManagerDelegate,
                        public IRemoteEventingDelegate,
                        public ILogEventingDelegate,
                        public ILogEventingProviderDelegate,
                        public ITimerDelegate,
                        public IWakeDelegate
        {
        protected:
          struct make_private {};
          
        protected:
          void init() noexcept;
          
          static MonitorPtr create(const ICommandLineTypes::MonitorInfo &monitorInfo) noexcept;
          static MonitorPtr singleton(const ICommandLineTypes::MonitorInfo *monitorInfo = NULL) noexcept;
          
        public:
          typedef zsLib::Log::Severity Severity;
          typedef zsLib::Log::Level Level;
          typedef zsLib::Log::EventingAtomData EventingAtomData;
          typedef zsLib::Log::EventingAtomIndex EventingAtomIndex;
          typedef zsLib::Log::ProviderHandle ProviderHandle;
          typedef zsLib::Log::EventingAtomDataArray EventingAtomDataArray;
          typedef zsLib::Log::KeywordBitmaskType KeywordBitmaskType;

          ZS_DECLARE_TYPEDEF_PTR(IEventingTypes::Provider, Provider);
          ZS_DECLARE_TYPEDEF_PTR(IEventingTypes::Event, Event);
          typedef std::map<UUID, ProviderPtr> ProviderMap;
          
          typedef size_t ValueID;
          typedef std::map<ValueID, EventPtr> EventMap;
          
          struct ProviderInfo
          {
            ProviderHandle mHandle {};
            ProviderPtr mExistingProvider;
            UUID mProviderID {};
            String mProviderName;
            String mProviderUniqueHash;
            EventMap mEvents;

            ProviderInfo() noexcept;
            ~ProviderInfo() noexcept;
          };

          typedef std::set<ProviderInfo *> ProviderInfoSet;

        public:
          Monitor(
                  const make_private &,
                  IMessageQueuePtr queue,
                  const ICommandLineTypes::MonitorInfo &monitorInfo
                  ) noexcept;
          ~Monitor() noexcept;

          //-------------------------------------------------------------------
          //
          // Monitor => (friends)
          //
          
          static void monitor(const ICommandLineTypes::MonitorInfo &monitorInfo) noexcept;
          static void interrupt() noexcept;

        protected:
          //-------------------------------------------------------------------
          //
          // Monitor::ISingletonManagerDelegate
          //
          
          void notifySingletonCleanup() noexcept override;

          //-------------------------------------------------------------------
          //
          // Monitor::ITimerDelegate
          //

          void onTimer(ITimerPtr timer) override;

          //-------------------------------------------------------------------
          //
          // Monitor::IWakeDelegate
          //

          void onWake() override;

          //-------------------------------------------------------------------
          //
          // Monitor::IRemoteEventingDelegate
          //

          void onRemoteEventingStateChanged(
                                            IRemoteEventingPtr connection,
                                            States state
                                            ) override;
          
          void onRemoteEventingRemoteSubsystem(
                                               IRemoteEventingPtr connection,
                                               const char *subsystemName
                                               ) override;

          void onRemoteEventingRemoteProvider(
                                              UUID providerID,
                                              const char *providerName,
                                              const char *providerUniqueHash
                                              ) override;
          void onRemoteEventingRemoteProviderGone(const char *providerName) override;
          
          void onRemoteEventingRemoteProviderStateChange(
                                                         const char *providerName,
                                                         KeywordBitmaskType keywords
                                                         ) override;

          void onRemoteEventingLocalDroppedEvents(
                                                  IRemoteEventingPtr connection,
                                                  size_t totalDropped
                                                  ) override;
          void onRemoteEventingRemoteDroppedEvents(
                                                   IRemoteEventingPtr connection,
                                                   size_t totalDropped
                                                   ) override;

          //-------------------------------------------------------------------
          //
          // Monitor::ILogEventingProviderDelegate
          //

          void notifyEventingProviderRegistered(
                                                ProviderHandle handle,
                                                EventingAtomDataArray eventingAtomDataArray
                                                ) noexcept override;
          void notifyEventingProviderUnregistered(
                                                  ProviderHandle handle,
                                                  EventingAtomDataArray eventingAtomDataArray
                                                  ) noexcept override;

          //-------------------------------------------------------------------
          //
          // Monitor::ILogEventingDelegate
          //

          // (ignored) virtual void notifyNewSubsystem(zsLib::Subsystem &inSubsystem) {}
          
          // notification of a log event
          void notifyWriteEvent(
                                ProviderHandle handle,
                                EventingAtomDataArray eventingAtomDataArray,
                                Severity severity,
                                Level level,
                                EVENT_DESCRIPTOR_HANDLE descriptor,
                                EVENT_PARAMETER_DESCRIPTOR_HANDLE paramDescriptor,
                                EVENT_DATA_DESCRIPTOR_HANDLE dataDescriptor,
                                size_t dataDescriptorCount
                                ) noexcept override;

        protected:
          //-------------------------------------------------------------------
          //
          // Monitor => (internal)
          //

          void internalInterrupt() noexcept;
          void cancel() noexcept;
          void step() noexcept(false); // throws Failure
          bool shouldQuit() const noexcept { return mShouldQuit; }

        protected:
          //-------------------------------------------------------------------
          //
          // Monitor => (data)
          //

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
          bool mFirstOutputEvent {true};

          ITimerPtr mAutoQuitTimer;

          IRemoteEventingPtr mRemote;
        };

      } // namespace internal
    } // namespace tool
  } // namespace eventing
} // namespace zsLib
