// Copyright [2020] <Copyright Kevin, kevin.lau.gd@gmail.com>

#ifndef FT_SRC_GATEWAY_VIRTUAL_VIRTUALTRADEAPI_H_
#define FT_SRC_GATEWAY_VIRTUAL_VIRTUALTRADEAPI_H_

#include <atomic>
#include <condition_variable>
#include <list>
#include <map>
#include <mutex>

#include "Core/Constants.h"

namespace ft {

struct VirtualOrderReq {
  uint32_t ticker_index;
  uint32_t type;
  uint32_t direction;
  uint32_t offset;
  int volume;
  double price;

  uint64_t order_id;
  bool to_canceled;
};

class VirtualGateway;

class VirtualTradeApi {
 public:
  VirtualTradeApi();

  void set_gateway(VirtualGateway* gateway);

  void start();

  uint64_t insert_order(VirtualOrderReq* req);

  bool cancel_order(uint64_t order_id);

  void update_quote(uint32_t ticker_index, double ask, double bid);

 private:
  void process_pendings();

 private:
  struct LatestQuote {
    double bid = 0;
    double ask = 0;
  };

 private:
  VirtualGateway* gateway_;
  std::mutex mutex_;
  std::condition_variable cv_;
  std::map<uint32_t, LatestQuote> lastest_quotes_;
  std::list<VirtualOrderReq> pendings_;
  std::map<uint32_t, std::list<VirtualOrderReq>> limit_orders_;
  std::atomic<uint64_t> next_order_id_ = 1;
};

}  // namespace ft

#endif  // FT_SRC_GATEWAY_VIRTUAL_VIRTUALTRADEAPI_H_