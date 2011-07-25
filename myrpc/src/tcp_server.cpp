#include "inc/tcp_server.h"
#include "inc/stream_tcp_socket.h"
#include "inc/session.h"
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

namespace msgpack {
namespace myrpc {

struct tcp_server::tcp_server_impl {
    tcp_server_impl(int port, shared_dispatcher dispatcher, boost::shared_ptr<logger_type> logger)
        : dispatcher(dispatcher),
        io_service(new boost::asio::io_service()),
        work(new boost::asio::io_service::work(*io_service)),
        acceptor(*io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
        logger(logger)
    {
    }

    msgpack::myrpc::shared_dispatcher dispatcher;
    boost::shared_ptr<boost::asio::io_service> io_service;
    std::auto_ptr<boost::asio::io_service::work> work;
    boost::asio::ip::tcp::acceptor acceptor;
    boost::thread thread;
    boost::shared_ptr<logger_type> logger;
};

tcp_server::tcp_server(int port, shared_dispatcher dispatcher, boost::shared_ptr<logger_type> logger)
    : pimpl(new tcp_server_impl(port, dispatcher, logger))

{
    boost::shared_ptr<stream_tcp_socket> socket(new stream_tcp_socket(pimpl->io_service));
    boost::shared_ptr<session> session(new myrpc::session(
        socket, pimpl->dispatcher, logger));

    pimpl->acceptor.async_accept(socket->get_socket(),
        boost::bind(&tcp_server::handle_accept, this, session,
        boost::asio::placeholders::error));

    pimpl->thread = boost::thread(boost::bind(&boost::asio::io_service::run, pimpl->io_service.get()));
}

void tcp_server::handle_accept(boost::shared_ptr<session> s, const boost::system::error_code& error)
{
    if (!error)
    {
        s->start();
        boost::shared_ptr<stream_tcp_socket> socket(new stream_tcp_socket(pimpl->io_service));
        s = boost::shared_ptr<session>(new session(
            socket, pimpl->dispatcher, pimpl->logger));

        pimpl->acceptor.async_accept(socket->get_socket(),
            boost::bind(&tcp_server::handle_accept, this, s,
            boost::asio::placeholders::error));
    }
}

tcp_server::~tcp_server()
{
    pimpl->acceptor.close();
    pimpl->work.reset(); // to wait all pending operations

    if (pimpl->thread.joinable())
        pimpl->thread.join();
}

} // namespace rpc {
} // namespace msgpack {
