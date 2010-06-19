
#include "eventpoller.h"
#include "eventi.h"

EVENTRPC_NAMESPACE_BEGIN

static void HandleEvent(int fd, short event_flags, void *arg) {
  (void)fd;
  Event *event = static_cast<Event*>(arg);

  if (event_flags & WRITE_EVENT) {
    event->OnWrite();
  }
  if (event_flags & READ_EVENT) {
    event->OnRead();
  }
}

struct EventPoller::Impl {
 public:
  Impl(EventPoller *event_poller)
    : event_poller_(event_poller)
    , event_base_(NULL) {
    event_base_ = event_init();
  }

  void Loop();

  void Stop();

  bool AddEvent(Event *event);

  bool DelEvent(Event *event);

 private:
  EventPoller *event_poller_;
  struct event_base *event_base_;
};

void EventPoller::Impl::Loop() {
  event_base_dispatch(event_base_);
}

void EventPoller::Impl::Stop() {
}

bool EventPoller::Impl::AddEvent(Event *event) {
  if (event->event_flags() != -1) {
    event_set(event->event(), event->fd(), event->event_flags(),
              &eventrpc::HandleEvent, event);
    if (event_base_set(event_base_, event->event()) == -1) {
      return false;
    }
    if (event_add(event->event(), NULL) == -1) {
      return false;
    }
  }

  event->set_event_poller(event_poller_);
  return true;
}

bool EventPoller::Impl::DelEvent(Event *event) {
  event->set_event_flags(-1);
  return event_del(event->event()) == -1 ? false : true;
}

EventPoller::EventPoller()
  : impl_(new Impl(this)) {
}

EventPoller::~EventPoller() {
  delete impl_;
}

void EventPoller::Loop() {
  impl_->Loop();
}

void EventPoller::Stop() {
  impl_->Stop();
}

bool EventPoller::AddEvent(Event* event) {
  return impl_->AddEvent(event);
}

bool EventPoller::DelEvent(Event* event) {
  return impl_->DelEvent(event);
}

EVENTRPC_NAMESPACE_END
