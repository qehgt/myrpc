#include "inc/tcp_client.h"
#include "inc/stream_tcp_socket.h"
#include <boost/thread.hpp>

namespace msgpack {
namespace myrpc {

struct tcp_client::tcp_client_impl {
    tcp_client_impl(boost::shared_ptr<logger_type> logger)
        : io_service(new boost::asio::io_service()),
        work(new boost::asio::io_service::work(*io_service)),
        logger(logger)
    {}

    boost::shared_ptr<boost::asio::io_service> io_service;
    std::auto_ptr<boost::asio::io_service::work> work;
    boost::shared_ptr<stream_tcp_socket> socket;
    boost::thread thread;
    boost::shared_ptr<logger_type> logger;
};

tcp_client::tcp_client(const char* host, const char* service_name, shared_dispatcher dispatcher,
                       boost::shared_ptr<logger_type> logger)
    : pimpl(new tcp_client_impl(logger))
{
    using namespace boost::asio;
    using namespace boost::asio::ip;

    tcp::resolver resolver(*pimpl->io_service);
    tcp::resolver::query query(tcp::v4(), host, service_name);
    tcp::resolver::iterator iterator = resolver.resolve(query);
    pimpl->socket = boost::shared_ptr<stream_tcp_socket>(new stream_tcp_socket(pimpl->io_service));
    pimpl->socket->get_socket().connect(*iterator);
    
    if (dispatcher.get() == NULL)
        dispatcher = shared_dispatcher(new dummy_dispatcher_type());

    session = boost::shared_ptr<myrpc::session>(new myrpc::session(
        pimpl->socket, dispatcher, logger));

    session->start();
    pimpl->thread = boost::thread(boost::bind(&io_service::run, pimpl->io_service.get()));
}

tcp_client::~tcp_client()
{
    boost::system::error_code ec;
    pimpl->socket->close(ec);
    pimpl->work.reset();
    if (pimpl->thread.joinable()) {
        pimpl->thread.join();
    }
}

} // namespace rpc {
} // namespace msgpack {
