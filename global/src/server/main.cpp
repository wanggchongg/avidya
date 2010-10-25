
#include <list>
#include <google/protobuf/text_format.h>
#include <google/protobuf/message.h>
#include <eventrpc/log/log.h>
#include <proto/server_config.pb.h>
#include <server/fast_leader_electioner.h>

using std::list;

void Usage() {
  printf("usage: global_main -c config_file\n");
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    Usage();
    exit(-1);
  }

  server_config::ServerConfig config;
  ASSERT(google::protobuf::TextFormat::ParseFromString(argv[1],
                                                       &config));
  list<server_config::ServerInfo> server_info_list;
  for (int i = 0; i < config.server_info_list().size(); ++i) {
    server_info_list.push_back(config.server_info_list(i));
  }
  global::FastLeaderElectioner electioner(config.server_id(),
                                          server_info_list);
  electioner.Start();

  return 0;
}
