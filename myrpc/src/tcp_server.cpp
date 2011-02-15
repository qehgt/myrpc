#include "inc/tcp_server.h"
#include "inc/stream_tcp_socket.h"
#include "inc/session.h"
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

namespace msgpack {
namespace myrpc {

struct tcp_server::tcp_server_impl {
    tcp_server_impl(int port, shared_dispatcher dispatcher)
        : dispatcher(dispatcher),
        acceptor(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
    {
    }

    msgpack::myrpc::shared_dispatcher dispatcher;
    boost::asio::io_service io;
    boost::asio::ip::tcp::acceptor acceptor;
    boost::thread thread;

    typedef boost::shared_ptr<msgpack::myrpc::session> created_session;
    typedef std::map<msgpack::myrpc::session*, created_session> session_holder_type;
    session_holder_type session_holder;
    typedef boost::mutex mutex_type;
    mutex_type mutex;
};

tcp_server::tcp_server(int port, shared_dispatcher dispatcher)
    : pimpl(new tcp_server_impl(port, dispatcher))

{
    boost::shared_ptr<stream_tcp_socket> socket(new stream_tcp_socket(pimpl->io));
    boost::shared_ptr<session> session(new myrpc::session(
        socket, pimpl->dispatcher));

    pimpl->session_holder[session.get()] = session;

    pimpl->acceptor.async_accept(*socket,
        boost::bind(&tcp_server::handle_accept, this, session,
        boost::asio::placeholders::error));

    pimpl->thread = boost::thread(boost::bind(&boost::asio::io_service::run, &pimpl->io));
}

void tcp_server::handle_accept(boost::shared_ptr<session> s, const boost::system::error_code& error)
{
    if (!error)
    {
        s->start(this);

        boost::shared_ptr<stream_tcp_socket> socket(new stream_tcp_socket(pimpl->io));
        s = boost::shared_ptr<session>(new session(
            socket, pimpl->dispatcher));

        {
            tcp_server_impl::mutex_type::scoped_lock lock(pimpl->mutex);
            pimpl->session_holder[s.get()] = s;
        }

        pimpl->acceptor.async_accept(*socket,
            boost::bind(&tcp_server::handle_accept, this, s,
            boost::asio::placeholders::error));
    }
}

tcp_server::~tcp_server()
{
    pimpl->acceptor.close();

    {
        tcp_server_impl::mutex_type::scoped_lock lock(pimpl->mutex);
        for (tcp_server_impl::session_holder_type::iterator i = pimpl->session_holder.begin();
            i != pimpl->session_holder.end();
            ++i)
        {
            (*i).second->stop();
        }
    }

    if (pimpl->thread.joinable())
        pimpl->thread.join();
}

void tcp_server::on_session_finish(session* s)
{
    tcp_server_impl::mutex_type::scoped_lock lock(pimpl->mutex);
    pimpl->session_holder.erase(s);
}


} // namespace rpc {
} // namespace msgpack {
