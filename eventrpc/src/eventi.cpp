
#include <unistd.h>
#include "eventi.h"
#include "eventpoller.h"

EVENTRPC_NAMESPACE_BEGIN

void Event::Close() {
  if (fd_ != -1) {
    event_poller_->DelEvent(this);
    close(fd_);
    fd_ = -1;
    event_flags_ = -1;
  }
}

bool Event::UpdateEvent(short new_event_flags) {
  if (event_flags_ == new_event_flags) {
    return true;
  }
  if (event_flags_ != -1 && !event_poller_->DelEvent(this)) {
    return false;
  }
  event_flags_ = new_event_flags;
  if (!event_poller_->AddEvent(this)) {
    return false;
  }
  return true;
}

EVENTRPC_NAMESPACE_END
