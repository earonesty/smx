/* COPYRIGHT 1998 Prime Data Corp. 
	All Rights Reserved.  Use of this file without prior written 
	authorization is strictly prohibited.
*/
#include "res.h"

//#define RESDEBUG

#ifdef RESDEBUG
	#include "stdio.h"
#endif

#ifndef _STRING_H
  #include <string.h>
#endif

long ghRes = 0;

#if (defined _WIN32 || defined _WINDOWS) && !(defined _QRES)
// windows resource links
#include <windows.h>

long resInit(char *file) {
	HINSTANCE hInst;
	if (!(hInst = GetModuleHandle(file)))
		hInst = LoadLibrary(file);
	return (long) hInst;
}

void resTerm(long hMod) {
	return;
}

long resLoad(long hMod, long id, char **pData) {
// hack to get around windows issues..  CALLER must delete returned pointer

#ifdef RESDEBUG
	printf("resLoad: hMod %d, id %d\n", hMod, id);
#endif

	long len = 2048;
	if (pData) {
		*pData = new char[len];
		len = LoadString((HINSTANCE)hMod, id, *pData, len);
		if (len == 0) {
			if (GetLastError() ==  ERROR_RESOURCE_NAME_NOT_FOUND) 
				return RES_ERRNOTFOUND;
			else
				return RES_ERRREAD;
		}
	}

	return len;
}

long resLoad(long hMod, long id, char *pData, int len) {

#ifdef RESDEBUG
	printf("resLoad: hMod %d, id %d, len %d\n", hMod, id, len);
#endif

	if (pData) {
		len = LoadString((HINSTANCE)hMod, id, pData, len);
		if (len == 0) {
			if (GetLastError() ==  ERROR_RESOURCE_NAME_NOT_FOUND) 
				return RES_ERRNOTFOUND;
			else
				return RES_ERRREAD;
		}
	}
	return len;
}

#else 

// ANSI C++ resource implementation
#include <stdio.h>
#if defined _WIN32 || defined _WINDOWS
	#include <io.h>
	#define O_BINREAD _O_BINARY | _O_RDONLY
#else
	#include <sys/types.h>
	#include <sys/stat.h>    
	#include <unistd.h>
	#define O_BINREAD O_RDONLY
#endif
#include <fcntl.h>
  
#define min(a, b)  (((a) < (b)) ? (a) : (b)) 
/*
#ifdef LITTLE_ENDIAN
	#define fixl(l) ( ((l) << 24) | (((l) << 8) & 0xff0000) | (((l) >> 8) & 0xff00) | ((l) >> 24) )
	#define fixs(s) ( ((s) << 8) | ((s) >> 8) )
#endif
*/

#pragma pack(2)
struct ResEntryData
{
	short ref;	// size of this structure
	short type;	// data type
	short id;	// data id 
	long  size;	// data size
} resDataBuf;
#define RES_PACKEDSIZE sizeof(ResEntryData)
#pragma pack()

struct ResEntry : public ResEntryData
{
	ResEntry	*next;	// linked-list
	char		*data;  // resource data
};

class CResFork 
{
public:
	CResFork();
	~CResFork();

	int m_file;
	ResEntry *m_table;

	int Init(char *file);
	int Load(int id, char **pData);
};

// link

long resInit(char *file) {
	CResFork *fork = new CResFork;
	if (!file) {
		pid_t pid = getpid();
		char pid_buf[256];
		char file_buf[256];
		sprintf(pid_buf, "/proc/%d/exe", pid);
		readlink(pid_buf, file_buf, 256);
		file = file_buf;
	}

	if (fork->Init(file)) 
		return (long) fork;
	else {
		delete fork;
		return 0;
	}
}

void resTerm(long hMod) {
	if (hMod)
		delete ((CResFork *) hMod);
}

long resLoad(long hMod, long id, char *buf, int buf_len) {
	long tmp_len = 0;

	if (buf) {
		char *tmp;
		tmp_len = resLoad(hMod, id, &tmp);
		if (tmp_len > 0) {
			memcpy(buf, tmp, tmp_len = min(buf_len, tmp_len));
		}
	}

	return tmp_len;
}

long resLoad(long hMod, long id, char **pData) {
	if (!hMod) return -1;	
	return ((CResFork *) hMod)->Load(id, pData);
}

// implementation

CResFork::CResFork() 
{
	m_file = 0;
	m_table = NULL;
}

CResFork::~CResFork() 
{
	ResEntry *eCur = m_table, *eNex;
	while (eCur) {
		if (eCur->data) 
			delete eCur->data;

		eNex = eCur->next;
		delete eCur;
		eCur = eNex;
	}
}

int CResFork::Init(char *file)
{
	m_file = open(file, O_BINREAD);
	m_table = NULL;
	return m_file > 0;
}

int CResFork::Load(int id, char **pData) 
{
	ResEntry *eCur = m_table;
	ResEntry *ePrev = NULL;
	long loc = 0, off;

	if (pData)
		*pData = NULL;

	while (eCur && eCur->size >= 0) {
		if (eCur->id == id) break;
		loc = loc + eCur->size + RES_PACKEDSIZE;
		ePrev = eCur;
		eCur = eCur->next;
	}

	while (!eCur) {
		eCur = new ResEntry;
		if (ePrev) 
			ePrev->next = eCur;
		else
			m_table = eCur;
		eCur->next = NULL;
		eCur->data = NULL;

		off = 0 - loc - RES_PACKEDSIZE;
		if (lseek(m_file, off, SEEK_END) > 0) {
			read(m_file, (ResEntryData *) eCur, RES_PACKEDSIZE);
/*
#ifdef LITTLE_ENDIAN
			eCur->ref = fixs(eCur->ref);
			eCur->type = fixs(eCur->type);
			eCur->id = fixs(eCur->id);
			eCur->size = fixl(eCur->size);
#endif
*/
			if (eCur->ref != RES_PACKEDSIZE) {
				// reference check failed
				eCur->id = -1;
				return RES_ERRVERSION;
			}
		} else {
			eCur->id = -1;
			return RES_ERRREAD;
		}
		ePrev = eCur;
		if ((eCur->id > 0) && eCur->id != id) {
			loc = loc + eCur->size + RES_PACKEDSIZE;
			eCur = NULL;
		}
	}

	if (eCur->id == id) {
		if (pData) {
			if (!eCur->data) {
				off = 0 - loc - RES_PACKEDSIZE - eCur->size;
				if (lseek(m_file, off, SEEK_END) >= 0) {
					eCur->data = new char[eCur->size + 1];
					eCur->size = read(m_file, eCur->data, eCur->size);
					eCur->data[eCur->size] = '\0';
				} else
					return RES_ERRREAD;
			}
			*pData = eCur->data;
			return eCur->size;
		} else
			return eCur->size;
	}
	else
		return RES_ERRNOTFOUND;
}
#endif
