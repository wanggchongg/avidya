
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
  if (argc < 3) {
    Usage();
    exit(-1);
  }

  server_config::ServerConfig config;
  string content;
  FILE *file = fopen(argv[2], "r");
  char buf[200];
  while (fgets(buf, 200, file)) {
    content.append(buf);
  }
  ASSERT(google::protobuf::TextFormat::ParseFromString(content,
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
