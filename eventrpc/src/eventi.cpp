
#include <unistd.h>
#include "eventi.h"
#include "eventpoller.h"

EVENTRPC_NAMESPACE_BEGIN

void Event::Close() {
  if (fd_ != -1) {
    event_poller_->DelEvent(event_, this);
  }
  close(fd_);
  fd_ = -1;
}

EVENTRPC_NAMESPACE_END
