#ifndef qthread_h
#define qthread_h

class qThread;

#ifdef WIN32
	#include <windows.h>
	#include <process.h>
	#include <assert.h>
	#define ThreadHandle HANDLE
	#include "crit.h"
	#include "qthread-win.h"
#endif

#ifdef unix
	#define ThreadHandle int
	#include "qthread-unix.h"
	#include "crit.h"
#endif

#endif  // #ifndef qthread_h
