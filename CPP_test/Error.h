#pragma once
#ifndef SAOFUERROR
#define SAOFUERROR

#define SAOFU_EXCEPTION(hr) SaoFU::Error(__LINE__, __FILE__, hr)
#define SAOFU_LASTERROR() SaoFU::Error(__LINE__, __FILE__, GetLastError())
#define SAOFU_TRY_INIT() HRESULT _hr = S_OK
#define TRY_(RES) if(FAILED(_hr = RES)) { throw SaoFU::Error(SAOFU_EXCEPTION(_hr)); }


#include <string>
typedef long HRESULT;

namespace SaoFU {
    class Error : public std::exception {
    public:
        Error(int line, const char* file, HRESULT hr);
        char const* what() const override;
    private:
        int line;
        HRESULT hr;
        std::string file;
    protected:
        mutable std::string buffer;
    };
}

#endif