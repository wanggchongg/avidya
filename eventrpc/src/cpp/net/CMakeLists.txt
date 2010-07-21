SET(NET_SRC connectionevent.cpp event.cpp dispatcher.cpp meta.cpp
  rpcclientevent.cpp rpcserverevent.cpp socket_utility.cpp)
ADD_LIBRARY(eventrpc STATIC ${NET_SRC})
