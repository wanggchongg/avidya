
#include "eventpoller.h"
#include "eventi.h"

EVENTRPC_NAMESPACE_BEGIN

static void OnRead(int fd, short event, void *arg) {
  (void)fd;
  (void)event;
  Event *ev = static_cast<Event*>(arg);

  ev->OnRead();
}

static void OnWrite(int fd, short event, void *arg) {
  (void)fd;
  (void)event;
  Event *ev = static_cast<Event*>(arg);

  ev->OnWrite();
}

struct EventPoller::Impl {
 public:
  Impl(EventPoller *event_poller)
    : event_poller_(event_poller) {
    }

  void Loop();

  void Stop();

  void AddEvent(short event, Event *ev);

  void DelEvent(short event, Event *ev);

 private:
  EventPoller *event_poller_;
};

void EventPoller::Impl::Loop() {
  event_dispatch();
}

void EventPoller::Impl::Stop() {
}

void EventPoller::Impl::AddEvent(short event, Event *ev) {
  if (event & EV_READ) {
    event_set(ev->read_event(), ev->fd(), READ_EVENT,
              &eventrpc::OnRead, ev);
    event_add(ev->read_event(), NULL);
  }

  if (event & EV_WRITE) {
    event_set(ev->write_event(), ev->fd(), WRITE_EVENT,
              &eventrpc::OnWrite, ev);
    event_add(ev->write_event(), NULL);
  }

  ev->set_event_poller(event_poller_);
}

void EventPoller::Impl::DelEvent(short event, Event *ev) {
  if (event & EV_READ) {
    event_del(ev->read_event());
  }
  if (event & EV_WRITE) {
    event_del(ev->write_event());
  }
}

EventPoller::EventPoller()
  : impl_(new Impl(this)) {
    event_init();
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

void EventPoller::AddEvent(short event, Event* ev) {
  impl_->AddEvent(event, ev);
}

void EventPoller::DelEvent(short event, Event* ev) {
  impl_->DelEvent(event, ev);
}

EVENTRPC_NAMESPACE_END
