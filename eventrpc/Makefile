DIR=$(shell pwd)
BIN_DIR=$(DIR)/bin
LIB_DIR=$(DIR)/lib
SRC_DIR=$(DIR)/src
INCLUDE_DIR=$(DIR)/src
OBJ_DIR=$(DIR)/obj
DEPS_DIR=$(DIR)/deps
LIBNAME=eventrpc
LIB_NAME=lib$(LIBNAME).a
LIB=$(LIB_DIR)/lib$(LIBNAME).a
SAMPLES_DIR=$(DIR)/samples
INSTALL_LIB_DIR=/usr/local/lib
INSTALL_INCLUDE_DIR=/usr/local/include/avidya/eventrpc
LIBS=-lprotobuf -levent

EXTENSION=cpp
OBJS=$(patsubst $(SRC_DIR)/%.$(EXTENSION), $(OBJ_DIR)/%.o,$(wildcard $(SRC_DIR)/*.$(EXTENSION)))
DEPS=$(patsubst $(OBJ_DIR)/%.o, $(DEPS_DIR)/%.d, $(OBJS))

INCLUDE=-I$(INCLUDE_DIR)
INSTALL_FILES=rpcserverevent.h rpcclientevent.h eventpoller.h

CC=g++
STRIP=strip
CFLAGS=-Wall -W -g -Werror
STRIP_FLAGS=-g

.PHONY: all clean install uninstall rebuild test

all:$(OBJS)
	ar rcs $(LIB) $(OBJS)

sinclude $(DEPS)

$(DEPS_DIR)/%.d: $(SRC_DIR)/%.$(EXTENSION)
	@$(CC) -MM $(INCLUDE) $< > temp; \
	sed 1's,^,$(OBJ_DIR)/,' < temp > $@; \
	rm -f temp

$(OBJ_DIR)/%.o:$(SRC_DIR)/%.$(EXTENSION)
	$(CC) $< -o $@ -c $(CFLAGS) $(INCLUDE)

echo:$(SAMPLES_DIR)/echo_client.cpp $(SAMPLES_DIR)/echo_server.cpp $(LIB)
	protoc --proto_path=$(SAMPLES_DIR) --cpp_out=$(SAMPLES_DIR) $(SAMPLES_DIR)/echo.proto
	$(CC) -c $(SAMPLES_DIR)/echo.pb.cc -o $(SAMPLES_DIR)/echo.pb.o
	$(CC) $(SAMPLES_DIR)/echo_client.cpp -o $(BIN_DIR)/echo_client -l$(LIBNAME) $(LIBS) $(CFLAGS) $(INCLUDE) samples/echo.pb.o
	$(CC) $(SAMPLES_DIR)/echo_server.cpp -o $(BIN_DIR)/echo_server -l$(LIBNAME) $(LIBS) $(CFLAGS) $(INCLUDE) samples/echo.pb.o

install:
	cp $(LIB) $(INSTALL_LIB_DIR)
	test -d $(INSTALL_INCLUDE_DIR) || mkdir -p $(INSTALL_INCLUDE_DIR)
	for i in $(INSTALL_FILES); do 			\
		cd $(INCLUDE_DIR); 				\
		cp $$i $(INSTALL_INCLUDE_DIR); 	\
	done

uninstall:
	rm -f $(INSTALL_LIB_DIR)/$(LIB_NAME)
	rm -fr ${INSTALL_INCLUDE_DIR}

rebuild: clean all

clean:
	rm -rf $(OBJ_DIR)/* $(LIB_DIR)/* $(BIN_DIR)/*
	rm -fr $(DEPS_DIR)/*

