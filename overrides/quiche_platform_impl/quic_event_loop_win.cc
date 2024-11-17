#include "quiche_platform_impl/quic_event_loop_win.h"

namespace quic {

#define WSA_SOCKET_EVENT 0
#define WSA_TASK_EVENT 1
#define WSA_ALARM_EVENT 2

uint32_t GetWSAMask(QuicSocketEventMask event_mask) {
  return ((event_mask & kSocketEventReadable) ? FD_READ : 0) |
         ((event_mask & kSocketEventWritable) ? FD_WRITE : 0);
}

QuicWinEventLoop::QuicWinEventLoop(QuicClock* clock)
    : clock_(clock), task_ev_(WSACreateEvent()), socket_ev_(WSACreateEvent()) {}

QuicWinEventLoop::~QuicWinEventLoop() {
  WSACloseEvent(socket_ev_);
  WSACloseEvent(task_ev_);
  task_ev_ = nullptr;
}

bool QuicWinEventLoop::RegisterSocket(SocketFd fd,
                                      QuicSocketEventMask events,
                                      QuicSocketEventListener* listener) {
  // TODO
}

bool QuicWinEventLoop::UnregisterSocket(SocketFd fd) {
  // TODO
}

bool QuicWinEventLoop::RearmSocket(SocketFd fd, QuicSocketEventMask events) {
  // TODO
}

bool QuicWinEventLoop::ArtificiallyNotifyEvent(SocketFd fd,
                                               QuicSocketEventMask events) {
  // TODO
}

void QuicWinEventLoop::RunEventLoopOnce(QuicTime::Delta default_timeout) {
  // TODO
}

std::unique_ptr<QuicAlarmFactory> QuicWinEventLoop::CreateAlarmFactory() {
  // TODO
}

}  // namespace quic
