#ifndef H_MYRPC_TCP_CLIENT_H
#define H_MYRPC_TCP_CLIENT_H

#include "rpc_client.h"

namespace msgpack {
namespace myrpc {

class tcp_client : public rpc_client {
  public:
    tcp_client(const char* host, const char* service_name, 
        shared_dispatcher dispatcher = shared_dispatcher(),
        boost::shared_ptr<logger_type> logger = boost::shared_ptr<logger_type>());
    ~tcp_client();
    
  protected:
    struct tcp_client_impl;
    boost::shared_ptr<tcp_client_impl> pimpl;
};

}  // namespace myrpc
}  // namespace msgpack

#endif // H_MYRPC_TCP_CLIENT_H

