#include "CurrentThread.h"
#include <stdio.h>

namespace CurrentThread
{
    thread_local int cachedTid_ = 0;
	thread_local char tidString_[32];
	thread_local int tidLength_ = 6;

	pid_t gettid()
	{
		return static_cast<pid_t>(::syscall(SYS_gettid));
	}

	void cacheTid()
	{
		if (cachedTid_ == 0) {
			cachedTid_ = gettid();
			tidLength_ = snprintf(tidString_, sizeof(tidString_), "%5d ", cachedTid_);
		}
	} 
}
