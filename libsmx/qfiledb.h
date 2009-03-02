/*
Copyright 1998-2005 Erik Aronesty. All rights reserved.

        1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

	3. Original source code, information and documentation is available at www.smxlang.org

THIS SOFTWARE IS PROVIDED 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SMX AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/



#include <fcntl.h>

#ifdef WIN32
	#include <io.h>
	#include <sys/locking.h>
#endif

#include <errno.h>

#define QFDB_VERSION 65
#define QFDB_STAT_THRESHOLD 10

class CFileLock 
{
	int myFile;
	int myPos;
	int myLen;

public:
	~CFileLock() {
		Free();
	}


	bool IsOK() {
		return myFile >= 0;
	}

	CFileLock::CFileLock() {
		myFile = -1;
		myPos = 0;
		myLen = 0;
	}

	CFileLock::CFileLock(int file, long pos, int len = 1) {
		lseek(file, pos, SEEK_SET);
#ifdef DEBUG_LOCK
		printf("locking %d for %d\n", pos, len);
#endif
		if (locking(file, _LK_LOCK, len) == 0) {
			myFile = file;
			myPos = pos;
			myLen = len;
		} else {
			myFile = -1;
			myPos = 0;
			myLen = 0;
		}
	}

	bool LockAppend(int file, long *pos, int len) {
		Free();

		int r;
		*pos = lseek(file, 0, SEEK_END);
#ifdef DEBUG_LOCK
		printf("locking APPEND for %d", len);
#endif
		while ((r = locking(file, _LK_NBLCK, len)) == -1 && errno == EACCES) {
			Sleep(1);
			*pos = lseek(file, 0, SEEK_END);
		}
#ifdef DEBUG_LOCK
		printf("(%d)\n", *pos);
#endif
		if (r == 0) {
			myFile = file;
			myPos = *pos;
			myLen = len;
		} else {
			myFile = -1;
			myPos = 0;
			myLen = 0;
		}
		return myFile != -1;
	}

	void Free() {
		if (myFile >= 0) {
			long was = tell(myFile);
			lseek(myFile, myPos, SEEK_SET);
#ifdef DEBUG_LOCK
			printf("unlocking %d for %d\n", myPos, myLen);
#endif
			assert(locking(myFile, _LK_UNLCK, myLen) == 0);
			myFile = -1;
			myPos = 0;
			myLen = 0;
			lseek(myFile, was, SEEK_SET);
		}
	}
};


class qFileP {
	int myFileP;
	qFileP *myNext;

public:

	qFileP(int f, qFileP *next) {
		myFileP = f;
		myNext = next;
	}

	~qFileP() {
		if (myFileP)
			close(myFileP);
	}


	qFileP *GetNext() {
		return myNext;
	}
	void SetNext(qFileP *next) {
		myNext = next;
	}

	int GetFile() {
		return myFileP;
	}
	void SetFile(int f) {
		myFileP = f;
	}
};

class qFileDB {

	CMutex *myMutex;
	CStr  myFileName;

	int myUpdates;
	int myKeyCount;
	int myKeyBytes;
	int myDataBytes;
	int myIndexAlloc;

	qFileP *myFree;

	int GetFile();

	long CreateIndex(int file, int size);
	bool Seek(int file, long *value, long *index, const char *key, int add = 0, char *data = NULL, bool *added = NULL);
	void SaveStats(int file);

	void Close(int file) {
		_commit(file);
		CMutexLock lock(*myMutex);
	    myFree = new qFileP(file, myFree);
	}	

	struct {
		short Version;
		int KeyCount;
		int KeyBytes;
		int DataBytes;
		int Filler1;
		int Filler2;
	} myFileInfo;

public:


	qFileDB() {
		myFree = NULL;
		myMutex = new CMutex("/dev/qscript/qfiledb.cpp");
		memset(&myFileInfo, 0, sizeof(myFileInfo));
	}

	~qFileDB() {
		delete myMutex;
		CloseAll();
	}

	bool Open(const char *file);
	void CloseAll();

	bool Set(const CStr &name, const CStr &value);
	bool Get(const CStr &name, CStr &value);
};

