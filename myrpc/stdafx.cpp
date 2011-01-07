// stdafx.cpp : source file that includes just the standard includes
// myrpc.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file
#include <crtdbg.h>

#if defined(_MSC_VER) && defined(_DEBUG)

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
