
#ifndef __EVENTRPC_GLOBAL_INIT_H__
#define __EVENTRPC_GLOBAL_INIT_H__

class GlobalInitializer {
 public:
  typedef void (*void_func)(void);

  GlobalInitializer(const char *name, void_func fn) {
    fn();
  }
};

#define REGISTER_MODULE_INITIALIZER(name, body)                 \
  namespace {                                                   \
    static void eventrpc_init_module_##name () { body; }        \
    GlobalInitializer eventrpc_initializer_module_##name(#name, \
            eventrpc_init_module_##name);                       \
  }

#endif  //  __EVENTRPC_GLOBAL_INIT_H__
