#pragma once

#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>

// Tid = ThreadId
namespace CurrentThread
{
    // internal
    extern thread_local int cachedTid_;
    extern thread_local char tidString_[32];
    extern thread_local int tidLength_;

    inline pid_t getTid();

    void cacheTid();

    /* update thread */
    inline int tid()
    {
        if (__builtin_expect(cachedTid_ == 0, 0)) {
			cacheTid();
		}
		return cachedTid_;
    }

    inline const char* tidString()
    {
        return tidString_;
    }

    inline int stringTidLength()
    {
        return tidLength_;
    }
} 

