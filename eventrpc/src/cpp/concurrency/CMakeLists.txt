SET(CONCURRENCY_SRC mutex.cpp posixthread.cpp workerthread.cpp)
ADD_LIBRARY(eventrpc STATIC ${CONCURRENCY_SRC})
