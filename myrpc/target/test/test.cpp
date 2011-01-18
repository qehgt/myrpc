#include "myecho_server.h"
#include "inc/stream_tcp_socket.h"
#include "inc/session.h"
#include <boost/thread.hpp>

#if defined(_MSC_VER) && defined(_DEBUG)
#include <crtdbg.h>

namespace {
    class run_me {
    public:
        run_me()
        {
            int tmpDbgFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
            _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG); 
            _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT); 
            _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG); 
            _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT); 
            _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG); 
            _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);

            tmpDbgFlag |=_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF;
            _CrtSetDbgFlag(tmpDbgFlag);
        }
    };

    run_me run_me; // static class with constructor
}
#endif

void run_client_test()
{
    using namespace boost::asio;
    using namespace boost::asio::ip;
    using namespace msgpack;
    using namespace msgpack::myrpc;
    const char* PORT = "18811"; // ??

    try {
        io_service io_client;
        tcp::resolver resolver(io_client);
        tcp::resolver::query query(tcp::v4(), "127.0.0.1", PORT);
        tcp::resolver::iterator iterator = resolver.resolve(query);
        boost::shared_ptr<stream_tcp_socket> socket(new stream_tcp_socket(io_client));
        socket->connect(*iterator);

        msgpack::myrpc::shared_dispatcher dispatcher(new msgpack::myrpc::dummy_dispatcher_type());
        boost::shared_ptr<myrpc::session> s(new myrpc::session(
            boost::static_pointer_cast<io_stream_object>(socket),
            dispatcher
            ));

        s->start();
        boost::thread t(boost::bind(&io_service::run, &io_client));

        callable cc = s->call("add", -12, 13);
        int i = cc.get<int>();
        printf("i = %d\n", i);
        i = s->call("add", 12, 13).get<int>();
        printf("i = %d\n", i);
        std::string str = s->call("echo", std::string("aaaBBB")).get<std::string>();
        printf("str = %s\n", str.c_str());
        /*
        s->notify("echo", std::string("test string"));
        boost::this_thread::sleep(boost::posix_time::seconds(1)); 
        */

        // close socket & session
        socket->close();

        t.join();
    }
    catch (const std::exception& e)
    {
        printf("ex: %s\n", e.what());
    }
}


int main()
{
    using namespace msgpack;
    using namespace msgpack::myrpc;
    try {
        using namespace boost::asio;
        const int PORT = 18811;
        io_service io;

        msgpack::myrpc::shared_dispatcher dispatcher(new myecho());
        boost::shared_ptr<stream_tcp_socket> socket(new stream_tcp_socket(io));
        boost::shared_ptr<msgpack::myrpc::session> s(new msgpack::myrpc::session(
            boost::static_pointer_cast<io_stream_object>(socket),
            dispatcher));

        boost::thread t_client(run_client_test);

        ip::tcp::acceptor acceptor(io, ip::tcp::endpoint(ip::tcp::v4(), PORT));
        acceptor.accept(*socket);

        s->start();
        boost::thread t(boost::bind(&io_service::run, &io));

        t.join();
        t_client.join();
    }
    catch (const std::exception& e) {
        printf("main: ex=%s\n", e.what());
    }
    return 0;
}
