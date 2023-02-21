#pragma once
#ifndef SAOFUERROR
#define SAOFUERROR

#define SAOFU_EXCEPTION(hr) SaoFU::Error(__LINE__, __FILE__, hr)
#define SAOFU_LASTERROR() SaoFU::Error(__LINE__, __FILE__, GetLastError())
#define SAOFU_TRY_INIT() HRESULT _hr = S_OK
#define SAOFU_EXCEPTION_LABEL(name) _SaoFU_Label_##name

#ifdef SET_SAOFU_EXCEPTION_THROW
#   define HR_(RES) HR_T(RES)
#elif defined SET_SAOFU_EXCEPTION_RETURN
#   define HR_(RES) HR_R(RES)
#endif

//throw Error
#define HR_T(RES)                                 \
    if (FAILED(_hr = RES)) {                       \
        throw SaoFU::Error(SAOFU_EXCEPTION(_hr)); \
    }

//Return Result
#define HR_R(RES)                                 \
    if (FAILED(_hr = RES)) { \
        return SaoFU::e_what(__LINE__, __FILE__, _hr); \
    }

//Break Result
#define HR_B(RES)                                 \
    if (FAILED(_hr = RES)) { \
        SaoFU::e_what(__LINE__, __FILE__, _hr);   \
        break; \
    }

//Goto Result
#define HR_G(LABEL, RES)                                 \
    if (FAILED(_hr = RES)) { \
        SaoFU::e_what(__LINE__, __FILE__, _hr);          \
        goto _SaoFU_Label_##LABEL; \
    }


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

    HRESULT e_what(int line, const char* file, HRESULT hr);
}

#endif