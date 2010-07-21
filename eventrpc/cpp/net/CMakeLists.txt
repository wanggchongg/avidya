SET(NET_SRC connectionevent.cpp eventi.cpp eventpoller.cpp meta.cpp
  rpcclientevent.cpp rpcserverevent.cpp socket_utility.cpp)
ADD_LIBRARY(eventrpc STATIC ${NET_SRC})
