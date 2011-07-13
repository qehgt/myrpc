#ifndef H_MYRPC_TCP_SERVER_H
#define H_MYRPC_TCP_SERVER_H

#include "interfaces.h"

namespace msgpack {
namespace myrpc {

class tcp_server {
public:
    tcp_server(int port, shared_dispatcher dispatcher);
    ~tcp_server();

protected:
    void handle_accept(boost::shared_ptr<session> session, const boost::system::error_code& error);
    void on_session_finish(msgpack::myrpc::session* s);

    struct tcp_server_impl;
    std::auto_ptr<tcp_server_impl> pimpl;
};

}  // namespace myrpc
}  // namespace msgpack

#endif // H_MYRPC_TCP_SERVER_H
