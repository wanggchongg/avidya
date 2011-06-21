/*
 * Copyright(C) lichuang
 */
#ifndef __EVENTRPC_MESSAGE_HANDLER_H_
#define __EVENTRPC_MESSAGE_HANDLER_H_
namespace eventrpc {
struct MessageHandler {
  virtual ~MessageHandler() {}
  virtual bool HandlePacket(const MessageHeader &header,
                            Buffer* buffer) = 0;
 protected:
  MessageHandler() {}
};

class MessageChannel;
struct ChannelMessageHandler : public MessageHandler {
 public:
  ChannelMessageHandler(MessageChannel *channel)
    : channel_(channel) {
  }
  virtual ~ChannelMessageHandler() {}
  virtual bool HandlePacket(const MessageHeader &header,
                            Buffer* buffer) = 0;
 protected:
  MessageChannel *channel_;
};

class ServerMessageHandlerFactory;
class MessageConnection;
struct ServerMessageHandler : public MessageHandler {
  virtual ~ServerMessageHandler() {}
  virtual bool HandlePacket(const MessageHeader &header,
                            Buffer* buffer) = 0;
 protected:
  friend class ServerMessageHandlerFactory;
  ServerMessageHandler(MessageConnection *connection)
    : connection_(connection){
  }
  MessageConnection *connection_;
};

class ServerMessageHandlerFactory {
 public:
  ServerMessageHandlerFactory() {
  }
  virtual ~ServerMessageHandlerFactory() {
  }
  virtual ServerMessageHandler* CreateHandler(MessageConnection *connection) = 0;
};
};
#endif  // __EVENTRPC_MESSAGE_HANDLER_H_
