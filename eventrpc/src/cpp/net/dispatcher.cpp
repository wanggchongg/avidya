
#include "net/dispatcher.h"
#include "net/eventi.h"

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

struct Dispatcher::Impl {
 public:
  Impl(Dispatcher *dispatcher)
    : dispatcher_(dispatcher)
    , event_base_(NULL) {
    event_base_ = event_init();
  }

  void Loop();

  void Stop();

  bool AddEvent(Event *event);

  bool DelEvent(Event *event);

 private:
  Dispatcher *dispatcher_;
  struct event_base *event_base_;
};

void Dispatcher::Impl::Loop() {
  event_base_dispatch(event_base_);
}

void Dispatcher::Impl::Stop() {
}

bool Dispatcher::Impl::AddEvent(Event *event) {
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

  event->set_dispatcher(dispatcher_);
  return true;
}

bool Dispatcher::Impl::DelEvent(Event *event) {
  event->set_event_flags(-1);
  return event_del(event->event()) == -1 ? false : true;
}

Dispatcher::Dispatcher()
  : impl_(new Impl(this)) {
}

Dispatcher::~Dispatcher() {
  delete impl_;
}

void Dispatcher::Loop() {
  impl_->Loop();
}

void Dispatcher::Stop() {
  impl_->Stop();
}

bool Dispatcher::AddEvent(Event* event) {
  return impl_->AddEvent(event);
}

bool Dispatcher::DelEvent(Event* event) {
  return impl_->DelEvent(event);
}

EVENTRPC_NAMESPACE_END
