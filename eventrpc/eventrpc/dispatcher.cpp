
#include <errno.h>
#include "log.h"
#include "dispatcher.h"

namespace eventrpc {
Dispatcher::Dispatcher() {
  epoll_fd_ = epoll_create(1024);
  ASSERT_NE(-1, epoll_fd_);
}

Dispatcher::~Dispatcher() {
  close(epoll_fd_);
}

void Dispatcher::AddEvent(Event *event) {
  EventEntry *event_entry = new EventEntry();
  ASSERT_NE(static_cast<EventEntry*>(NULL), event_entry);
  event_entry->event = event;
  event->entry_ = static_cast<Event::entry_t>(event_entry);
  if (event->event_flags_ & EVENT_READ) {
    event_entry->epoll_ev.events = EPOLLIN;
  }
  if (event->event_flags_ & EVENT_WRITE) {
    event_entry->epoll_ev.events = EPOLLOUT;
  }
  event_entry->epoll_ev.data.ptr = event_entry;
  ASSERT_NE(-1, epoll_ctl(epoll_fd_, EPOLL_CTL_ADD,
                          event->fd_, &(event_entry->epoll_ev)));
}

void Dispatcher::DelEvent(Event *event) {
  EventEntry *event_entry = static_cast<EventEntry*>(event->entry_);
  ASSERT_NE(-1, epoll_ctl(epoll_fd_, EPOLL_CTL_DEL,
                          event->fd_, &(event_entry->epoll_ev)));
  event_entry->event = NULL;
  retired_events_.push_back(event_entry);
}

int Dispatcher::Poll() {
  while (true) {
    int number = epoll_wait(epoll_fd_, &epoll_event_buf_[0],
                            EPOLL_MAX_EVENTS, NULL);
    if (number == -1) {
      if (errno == EINTR) {
        continue;
      }
      LOG_ERROR() << "epoll_wait return -1, errno: " << errno;
      return -1;
    }
    EventEntry *event_entry;
    Event* event;
    for (int i = 0; i < number; ++i) {
      event_entry = static_cast<EventEntry*>(epoll_event_buf_[i].data.ptr);
      event = event_entry->event;
      if (event == NULL) {
        continue;
      }
      if (epoll_event_buf_[i].events & EPOLLIN) {
        event->HandleRead();
      }
      if (epoll_event_buf_[i].events & EPOLLOUT) {
        event->HandleWrite();
      }
    }
    for (RetiredEventVector::iterator iter = retired_events_.begin();
         iter != retired_events_.end(); ++iter) {
      delete (*iter);
    }
    retired_events_.clear();
    break;
  }
  return 0;
}
};
