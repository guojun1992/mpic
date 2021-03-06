#include "dynlib.h"

#include <sstream>

namespace mpic {

DynLib::DynLib(const std::string& dll_path)
    : handler_(NULL), dll_path_(dll_path) {
}

DynLib::~DynLib() {
}

bool DynLib::IsLoaded() {
    return handler_ == NULL ? false : true;
}

bool DynLib::Load() {
    std::string name = dll_path_;
#ifdef _WIN32
    handler_ = (Handler)::LoadLibraryA(name.c_str());
#else
    handler_ = (Handler)::dlopen(name.c_str(), RTLD_LAZY | RTLD_LOCAL);
#endif

    if (handler_) {
        return true;
    }

    std::ostringstream ss;
    ss << __FUNCTION__ << " : Load dynamic library "
       << dll_path_ << " failed. "
       << "Error message : " << GetError();
    error_ = ss.str();
    return false;
}

bool DynLib::Unload() {
    if (!handler_) {
        std::ostringstream ss;
        ss << __FUNCTION__
           << " Could not unload dynamic library " << dll_path_
           << ". maybe it haven't loaded yet, because handler_=NULL";
        error_ = ss.str();
        return false;
    }

#ifdef _WIN32
    // If the function FreeLibrary() succeeds, the return value is nonzero.
    if (::FreeLibrary(handler_)) {
        handler_ = NULL;
        return true;
    }
#else
    //The function dlclose() returns 0 on success, and non-zero on error.
    if (dlclose(handler_) == 0) {
        handler_ = NULL;
        return true;
    }
#endif

    std::ostringstream ss;
    ss << __FUNCTION__ << " : Unload dynamic library "
       << dll_path_ << " failed. "
       << "Error message : " << GetError();
    error_ = ss.str();
    return false;
}

void* DynLib::GetSymbol(const std::string& name) {
    void* rp = NULL;

#ifdef _WIN32
    rp = (void*)::GetProcAddress(handler_, name.c_str());
#else
    rp = (void*)::dlsym(handler_, name.c_str());
#endif

    if (rp == NULL) {
        std::ostringstream ss;
        ss << __FUNCTION__
           << " : Could not get the symbol : " << name
           << ". Error Message : " << GetError();

        error_ = ss.str();
    }

    return rp;
}

std::string DynLib::GetError() {
#ifdef _WIN32
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        ::GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0,
        NULL
    );
    std::string ret = (char*)lpMsgBuf;
    LocalFree(lpMsgBuf);
    return ret;
#else
    return std::string(dlerror());
#endif
}

}
