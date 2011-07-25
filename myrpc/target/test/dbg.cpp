
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
            _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_WNDW); 
            _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT); 
            _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_WNDW); 
            _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);

            tmpDbgFlag |=_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF;
            _CrtSetDbgFlag(tmpDbgFlag);
        }
    };

    run_me run_me; // static class with constructor
}
#elif defined (__GNUC__)

#include <mcheck.h>

namespace {
class run_me {
 public:
  run_me()
  {
    mtrace();
  }
  ~run_me()
  {
    muntrace();
  }
};
run_me run_me; // static class with constructor
}

#endif
