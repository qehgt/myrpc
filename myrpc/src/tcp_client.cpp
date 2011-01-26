#include "inc/tcp_client.h"
#include "inc/stream_tcp_socket.h"
#include <boost/thread.hpp>

namespace msgpack {
namespace myrpc {

struct tcp_client::tcp_client_impl {
    boost::asio::io_service io;
    boost::shared_ptr<stream_tcp_socket> socket;
    boost::thread thread;
};

tcp_client::tcp_client(const char* host, const char* service_name)
    : pimpl(new tcp_client_impl())
{
    using namespace boost::asio;
    using namespace boost::asio::ip;

    tcp::resolver resolver(pimpl->io);
    tcp::resolver::query query(tcp::v4(), host, service_name);
    tcp::resolver::iterator iterator = resolver.resolve(query);
    pimpl->socket = boost::shared_ptr<stream_tcp_socket>(new stream_tcp_socket(pimpl->io));
    pimpl->socket->connect(*iterator);
    msgpack::myrpc::shared_dispatcher dispatcher(new msgpack::myrpc::dummy_dispatcher_type());

    session = boost::shared_ptr<myrpc::session>(new myrpc::session(
        boost::static_pointer_cast<io_stream_object>(pimpl->socket),
        dispatcher));

    session->start();
    pimpl->thread = boost::thread(boost::bind(&io_service::run, &pimpl->io));
}

tcp_client::~tcp_client()
{
    if (pimpl->thread.joinable()) {
        boost::system::error_code ec;
        pimpl->socket->close(ec);
        pimpl->thread.join();
    }
}

} // namespace rpc {
} // namespace msgpack {
