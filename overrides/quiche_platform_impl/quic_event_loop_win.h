#ifndef QUICHE_OVERRIDES_QUICHE_PLATFORM_IMPL_QUIC_EVENT_LOOP_WIN_H_
#define QUICHE_OVERRIDES_QUICHE_PLATFORM_IMPL_QUIC_EVENT_LOOP_WIN_H_

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "absl/container/node_hash_map.h"
#include "quiche/quic/core/io/quic_event_loop.h"
#include "quiche/quic/core/io/socket.h"
#include "quiche/quic/core/quic_alarm.h"
#include "quiche/quic/core/quic_alarm_factory.h"
#include "quiche/quic/core/quic_clock.h"

#include <mmsystem.h>  // Must come after windows headers.
#include <sal.h>       // Must come after windows headers.

#include <memory>
#include <vector>

namespace quic {

class QuicWinEventLoop : public QuicEventLoop {
 public:
  QuicWinEventLoop(QuicClock* clock);

  // QuicEventLoop implementation.
  bool SupportsEdgeTriggered() const override { return false; }

  ABSL_MUST_USE_RESULT bool RegisterSocket(
      SocketFd fd,
      QuicSocketEventMask events,
      QuicSocketEventListener* listener) override;

  ABSL_MUST_USE_RESULT bool UnregisterSocket(SocketFd fd) override;

  ABSL_MUST_USE_RESULT bool RearmSocket(SocketFd fd,
                                        QuicSocketEventMask events) override;

  ABSL_MUST_USE_RESULT bool ArtificiallyNotifyEvent(
      SocketFd fd,
      QuicSocketEventMask events) override;

  void RunEventLoopOnce(QuicTime::Delta default_timeout) override;

  std::unique_ptr<QuicAlarmFactory> CreateAlarmFactory() override;

  const QuicClock* GetClock() override { return clock_; }

 private:
  class AlarmFactory : public QuicAlarmFactory {
   public:
    AlarmFactory(QuicWinEventLoop* loop) : loop_(loop) {}

    // QuicAlarmFactory implementation.
    QuicAlarm* CreateAlarm(QuicAlarm::Delegate* delegate) override;
    QuicArenaScopedPtr<QuicAlarm> CreateAlarm(
        QuicArenaScopedPtr<QuicAlarm::Delegate> delegate,
        QuicConnectionArena* arena) override;

   private:
    QuicWinEventLoop* loop_;
  };

  const QuicClock* clock_;
  // using for wakeup
  WSAEVENT task_ev_;
  // using for socket
  const WSAEVENT socket_ev_;

  class Registration {
   public:
    Registration(QuicWinEventLoop* loop,
                 SocketFd fd,
                 QuicSocketEventMask events,
                 QuicSocketEventListener* listener);
    ~Registration();

    void ArtificiallyNotify(QuicSocketEventMask events);
    void Rearm(QuicSocketEventMask events);

   private:
    QuicWinEventLoop* loop_;
    QuicSocketEventListener* listener_;
  };
  using RegistrationMap = absl::node_hash_map<SocketFd, Registration>;

  RegistrationMap registration_map_;
};
}  // namespace quic

#endif  // QUICHE_OVERRIDES_QUICHE_PLATFORM_IMPL_QUIC_EVENT_LOOP_WIN_H_
