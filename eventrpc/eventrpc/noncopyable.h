
#ifndef __EVENTRPC_NONCOPYABLE_H__
#define __EVENTRPC_NONCOPYABLE_H__

#define DISALLOW_EVIL_CONSTRUCTOR(Type) \
  Type(const Type&);                    \
  void operator=(const Type&)

#endif  //  __EVENTRPC_NONCOPYABLE_H__
