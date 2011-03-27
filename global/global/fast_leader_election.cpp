
#include <eventrpc/file_utility.h>
#include <eventrpc/log.h>
#include <google/protobuf/text_format.h>
#include "global/fast_leader_election.h"
#include "protocol/server_config.pb.h"

namespace global {
struct FastLeaderElection::Impl {
 public:
  Impl();
  ~Impl();

  void Start(const string &config_file);

 private:
  Global::ServerConfig server_config_;
};

FastLeaderElection::Impl::Impl() {
}

FastLeaderElection::Impl::~Impl() {
}

void FastLeaderElection::Impl::Start(const string &config_file) {
  string content;
  if (!eventrpc::FileUtility::ReadFileContents(config_file,
                                               &content)) {
    LOG_ERROR() << "cannot config file " << config_file;
    return;
  }
  if (google::protobuf::TextFormat::ParseFromString(
      content, &server_config_)) {
    LOG_ERROR() << "parse from file " << config_file << " error";
    return;
  }
}

FastLeaderElection::FastLeaderElection()
  : impl_(new Impl) {
}

FastLeaderElection::~FastLeaderElection() {
  delete impl_;
}

void FastLeaderElection::Start(const string &config_file) {
  impl_->Start(config_file);
}
};
