#include "myecho_server.h"
#include "inc/stream_tcp_socket.h"
#include "inc/session.h"
#include "inc/tcp_client.h"
#include "inc/tcp_server.h"
#include "inc/exception.h"
#include <boost/thread.hpp>
#include <boost/make_shared.hpp>

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
    const char* port = "18811";

    try {
        tcp_client cli("localhost", port);

        int a = -12;
        int b = 13;
        callable cc = cli.call("add", a, b);
        int i = cc.get<int>();
        try {
            i = cli.call("add", 12).get<int>();
        }
        catch (const argument_error&)
        {
            printf("OK: got expected argument_error\n");
        }
        catch (const std::exception& e)
        {
            printf("FAIL: unexpected exception: %s\n", e.what());
        }

        if (i == (a + b))
            printf("OK: add: i = %d\n", i);
        else
            printf("FAIL: add: i = %d\n", i);
        std::string s("aaaBBB");
        std::string str = cli.call("echo", s).get<std::string>();
        if (s == str)
            printf("OK: echo: str = %s\n", str.c_str());
        else
            printf("FAIL: echo: str = %s\n", str.c_str());
    }
    catch (const boost::exception& e)
    {
        printf("FAIL: boost ex: %s\n", boost::diagnostic_information(e).c_str());
    }
    catch (const std::exception& e)
    {
        printf("FAIL: %s\n", e.what());
    }
    catch (...) {
        printf("client: unknown ex\n");
    }
}

int main()
{
    using namespace msgpack;
    using namespace msgpack::myrpc;
    using namespace boost;

    try {
        const int PORT = 18811;

        tcp_server server(PORT, make_shared<myecho>());

        boost::thread t_client1(run_client_test);
        boost::thread t_client2(run_client_test);
        run_client_test();
        t_client1.join();
        t_client2.join();
    }
    catch (std::exception& e) {
        printf("main: ex=%s\n", e.what());
    }

    return 0;
}
