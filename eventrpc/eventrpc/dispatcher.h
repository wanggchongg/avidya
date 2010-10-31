
#ifndef __EVENTRPC_DISPATCHER_H__
#define __EVENTRPC_DISPATCHER_H__

namespace eventrpc {

class Event;

class Dispatcher {
 public:
  Dispatcher();

  ~Dispatcher();

  void Loop();

  void Stop();

  bool AddEvent(Event *event);

  bool DelEvent(Event *event);

 private:
  struct Impl;
  Impl *impl_;
};

};

#endif  // __EVENTRPC_DISPATCHER_H__
