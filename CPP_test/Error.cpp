#include "Error.h"
#include <string>
#include <fmt/format.h>

#include "Windows.h"

SaoFU::Error::Error(int line, const char* file, HRESULT hr) : line(line), file(file), hr(hr) {}

char const* SaoFU::Error::what() const {
    LPSTR p_msgbuf = nullptr;
    DWORD msg_len = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        hr,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&p_msgbuf, 0, NULL
    );

    std::string error_msg = "Unknown Exception";
    std::string msg;

    if (msg_len) {
        error_msg = p_msgbuf;
    }

    msg = fmt::format("{}\n", error_msg);
    msg += fmt::format("\"{}\"\n[line] {}\n[code] 0x{:X}\n", file, line, hr);
    this->buffer = msg;

    if (!msg_len) {
        return this->buffer.c_str();
    }

    LocalFree(p_msgbuf);
    return this->buffer.c_str();
}

HRESULT SaoFU::e_what(int line, const char* file, HRESULT hr) {
    Error error_msg(line, file, hr);
    MessageBoxA(0, error_msg.what(), 0, MB_ICONERROR);
    return hr;
}
