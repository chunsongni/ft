// Copyright [2020] <Copyright Kevin, kevin.lau.gd@gmail.com>

#include <atomic>
#include <iostream>

#include <getopt.hpp>
#include <spdlog/spdlog.h>

#include "ctp/CtpGateway.h"
#include "TraderInterface.h"

const char* kSimnowTradeAddr[] = {
  "tcp://180.168.146.187:10130",
  "tcp://180.168.146.187:10100",
  "tcp://180.168.146.187:10101",
  "tcp://218.202.237.33:10102"
};

const char* kBrokerID = "9999";
const char* kInvestorID = "122899";
const char* kPasswd = "lsk4129691";
const char* kAuthCode = "0000000000000000";
const char* kAppID = "simnow_client_test";

class TradeInfoCollector : public ft::TraderInterface {
 public:
  TradeInfoCollector() {
    gateway_ = new ft::CtpGateway;
    gateway_->register_cb(this);
  }

  bool login(const ft::LoginParams& params) {
    if (!gateway_->login(params)) {
      spdlog::error("[ContractExporter] login. Failed to login into trading server");
      return false;
    }
    is_login_ = true;
    return true;
  }

  bool dump_contracts(const std::string& file) {
    if (!is_login_)
      return false;

    auto status = gateway_->query_contract("", "");
    if (!status.wait())
      return false;

    std::ofstream ofs(file, std::ios_base::trunc);
    std::string line = fmt::format("#ticker,size,price_tick\n");
    ofs << line;
    for (const auto& [ticker, contract] : contracts_) {
      line = fmt::format("{},{},{}\n",
                         contract.ticker,
                         contract.size,
                         contract.price_tick);
      ofs << line;
    }

    ofs.close();
    return true;
  }

  void on_contract(const ft::Contract* contract) {
    contracts_[contract->ticker] = *contract;
  }

 private:
  ft::GatewayInterface* gateway_;
  std::map<std::string, ft::Contract> contracts_;
  std::atomic<bool> is_login_ = false;
};

int main() {
  std::size_t front_index = getarg(0UL, "--front");
  int log_level = getarg(0, "--loglevel");

  spdlog::set_level(static_cast<spdlog::level::level_enum>(log_level));

  auto collector = new TradeInfoCollector;
  ft::LoginParams params;

  if (front_index >= sizeof(kSimnowTradeAddr) / sizeof(kSimnowTradeAddr[0]))
    exit(-1);

  params.set_front_addr(kSimnowTradeAddr[front_index]);
  params.set_broker_id(kBrokerID);
  params.set_investor_id(kInvestorID);
  params.set_passwd(kPasswd);
  params.set_auth_code(kAuthCode);
  params.set_app_id(kAppID);

  if (!collector->login(params)) {
    exit(-1);
  }

  if (!collector->dump_contracts("contracts.txt")) {
    spdlog::error("Failed to export contracts");
    exit(-1);
  }

  spdlog::info("Successfully exported contracts");
}
