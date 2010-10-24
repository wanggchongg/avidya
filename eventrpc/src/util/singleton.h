
#ifndef __EVENTRPC_SINGLETON_H__
#define __EVENTRPC_SINGLETON_H__

#include <pthread.h>
#include <base/base.h>

EVENTRPC_NAMESPACE_BEGIN

template <class T>
class Singleton {
 public:
  static T* Instance() {
    ::pthread_once(&once_, &Singleton<T>::Init);
    return instance_;
  }

 private:
  // Create the instance.
  static void Init() {
    instance_ =  ::new T;
  }

  static pthread_once_t once_;
  static T* instance_;
};

template <class T>
pthread_once_t Singleton<T>::once_ = PTHREAD_ONCE_INIT;

template <class T>
T* Singleton<T>::instance_ = NULL;

EVENTRPC_NAMESPACE_END
#endif  // __EVENTRPC_SINGLETON_H__
