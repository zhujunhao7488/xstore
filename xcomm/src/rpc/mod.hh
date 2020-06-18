#pragma once

#include <functional>

#include "../transport/trait.hh"

#include "./op.hh"

namespace xstore {
namespace rpc {

/*!
  An RPC framework assuming a given SendTrait, and RecvTrait
  for sending and receiving RPC messages.
 */
template <class SendTrait, class RecvTrait> struct RPCCore {
  using rpc_func_t = std::function<void(
      const Header &rpc_header, const MemBlock &args, SendTrait *replyc)>;
  ReplyStation reply_station;
  std::vector<rpc_func_t> callbacks;

  explicit RPCCore(int num_cors) : reply_station(num_cors) {}

  auto execute(const RPCOp &op, SendTrait *sender) -> Result<std::string> {
    return sender->send(op.msg);
  }

  auto reg_callback(rpc_func_t callback) -> usize {
    auto id = callbacks.size();
    callbacks.push_back(callback);
    return id;
  }

  /*!
    \ret the number of RPC call and replies received.
    On reply, filling the reply statis's buf
   */
  auto recv_event_loop(RecvTrait *recv) -> usize {
    usize num = 0;
    for (recv->begin(); recv->has_msgs(); recv->next()) {
      num += 1;

      auto cur_msg = recv->cur_msg();
      ASSERT(cur_msg.sz >= sizeof(Header));

      Header &h = *(reinterpret_cast<Header *>(cur_msg.mem_ptr));
      ASSERT(h.payload + sizeof(Header) == cur_msg.sz); // sanity check header and content
      MemBlock payload((char *)cur_msg.mem_ptr + sizeof(Header), h.payload);

      switch (h.type) {
      case Req: {
        try {
          auto reply_channel = recv->reply_entry();
          callbacks[h.rpc_id](h, payload, &reply_channel);
        } catch (...) {
          ASSERT(false) << "rpc called failed with rpc id " << h.rpc_id;
        }
      } break;
      case Reply: {
        // pass
        auto ret = this->reply_station.append_reply(h.cor_id, payload);
        ASSERT(ret);
      }
        break;
      default:
        ASSERT(false) << "not implemented";
      }
    }
    return num;
  }
};

} // namespace rpc
} // namespace xstore