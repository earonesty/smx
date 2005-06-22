# Microsoft Developer Studio Project File - Name="libsmx" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libsmx - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libsmx.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libsmx.mak" CFG="libsmx - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libsmx - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libsmx - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libsmx - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libsmx - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ  /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ  /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "libsmx - Win32 Release"
# Name "libsmx - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\base64.cpp
# End Source File
# Begin Source File

SOURCE=.\buf_chain.cpp
# End Source File
# Begin Source File

SOURCE=.\buf_ref.cpp
# End Source File
# Begin Source File

SOURCE=.\card.cpp
# End Source File
# Begin Source File

SOURCE=.\cgi.cpp
# End Source File
# Begin Source File

SOURCE=.\core.cpp
# End Source File
# Begin Source File

SOURCE=.\dbpset.cpp
# End Source File
# Begin Source File

SOURCE=.\dparse.cpp
# End Source File
# Begin Source File

SOURCE=.\ex.cpp
# End Source File
# Begin Source File

SOURCE=.\ex2.cpp
# End Source File
# Begin Source File

SOURCE=.\fbuf.cpp
# End Source File
# Begin Source File

SOURCE=.\file.cpp
# End Source File
# Begin Source File

SOURCE=.\hash.c
# End Source File
# Begin Source File

SOURCE=.\hset.cpp
# End Source File
# Begin Source File

SOURCE=.\io.cpp
# End Source File
# Begin Source File

SOURCE=.\map.cpp
# End Source File
# Begin Source File

SOURCE=.\mapstr.cpp
# End Source File
# Begin Source File

SOURCE=.\math.cpp
# End Source File
# Begin Source File

SOURCE=".\open-enc.cpp"
# End Source File
# Begin Source File

SOURCE=.\opt.cpp
# End Source File
# Begin Source File

SOURCE=.\process.cpp
# End Source File
# Begin Source File

SOURCE=.\proto.cpp
# End Source File
# Begin Source File

SOURCE=.\pscache.cpp
# End Source File
# Begin Source File

SOURCE=.\pset.cpp
# End Source File
# Begin Source File

SOURCE=.\pstable.cpp
# End Source File
# Begin Source File

SOURCE=.\psximpl.cpp
# End Source File
# Begin Source File

SOURCE=.\psxutil.cpp
# End Source File
# Begin Source File

SOURCE=".\qctx-comp.cpp"
# End Source File
# Begin Source File

SOURCE=.\qctx.cpp
# End Source File
# Begin Source File

SOURCE=.\qenv.cpp
# End Source File
# Begin Source File

SOURCE=.\qfopen.cpp
# End Source File
# Begin Source File

SOURCE=.\qmail.cpp
# End Source File
# Begin Source File

SOURCE=".\qobj-ctx.cpp"
# End Source File
# Begin Source File

SOURCE=.\qobj.cpp
# End Source File
# Begin Source File

SOURCE=.\qobjx.cpp
# End Source File
# Begin Source File

SOURCE=.\qpriq.cpp
# End Source File
# Begin Source File

SOURCE=.\qsched.cpp
# End Source File
# Begin Source File

SOURCE=.\qstr.cpp
# End Source File
# Begin Source File

SOURCE=.\qthread.cpp
# End Source File
# Begin Source File

SOURCE=.\rcex.cpp
# End Source File
# Begin Source File

SOURCE=.\regx.cpp
# End Source File
# Begin Source File

SOURCE=.\res.cpp
# End Source File
# Begin Source File

SOURCE=.\sha.cpp
# End Source File
# Begin Source File

SOURCE=.\sock.cpp
# End Source File
# Begin Source File

SOURCE=.\sql.cpp
# End Source File
# Begin Source File

SOURCE=.\sqlgrp.cpp
# End Source File
# Begin Source File

SOURCE=.\stdafx.cpp
# End Source File
# Begin Source File

SOURCE=.\string.cpp
# End Source File
# Begin Source File

SOURCE=.\strx.cpp
# End Source File
# Begin Source File

SOURCE=.\tabfmt.cpp
# End Source File
# Begin Source File

SOURCE=.\tabpre.cpp
# End Source File
# Begin Source File

SOURCE=.\tabtd.cpp
# End Source File
# Begin Source File

SOURCE=.\time.cpp
# End Source File
# Begin Source File

SOURCE=.\util.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# End Target
# End Project
