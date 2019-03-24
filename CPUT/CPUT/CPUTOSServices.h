//--------------------------------------------------------------------------------------
// Copyright 2011 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED "AS IS."
// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.
//--------------------------------------------------------------------------------------
#ifndef CPUTOSSERVICES_H
#define CPUTOSSERVICES_H



#include "CPUT.h"

// OS includes
#include <errno.h>  // file open error codes
#include <string>   // wstring

namespace CPUTFileSystem
{
    //Working directory manipulation
    CPUTResult GetWorkingDirectory(std::string *pPath);
    CPUTResult GetWorkingDirectory(std::wstring *pPath);
    CPUTResult SetWorkingDirectory(const std::string &path);
    CPUTResult SetWorkingDirectory(const std::wstring &path);
    CPUTResult GetExecutableDirectory(std::string *pExecutableDir);
    CPUTResult GetExecutableDirectory(std::wstring *pExecutableDir);

    // Path helpers
    CPUTResult ResolveAbsolutePathAndFilename(const std::string &fileName, std::string *pResolvedPathAndFilename);
    CPUTResult ResolveAbsolutePathAndFilename(const std::wstring &fileName, std::wstring *pResolvedPathAndFilename);
    CPUTResult SplitPathAndFilename(const std::string &sourceFilename, std::string *pDrive, std::string *pDir, std::string *pfileName, std::string *pExtension);
    CPUTResult SplitPathAndFilename(const std::wstring &sourceFilename, std::wstring *pDrive, std::wstring *pDir, std::wstring *pfileName, std::wstring *pExtension);

    // file handling
    CPUTResult DoesFileExist(const std::string &pathAndFilename);
    CPUTResult DoesFileExist(const std::wstring &pathAndFilename);
    CPUTResult DoesDirectoryExist(const std::string &path);
    CPUTResult DoesDirectoryExist(const std::wstring &path);
    CPUTResult OpenFile(const std::wstring &fileName, FILE **pFilePointer);
    CPUTResult OpenFile(const std::string &fileName, FILE **pFilePointer);
    CPUTResult ReadFileContents(const cString &fileName, UINT *psizeInBytes, void **ppData);

    CPUTResult TranslateFileError(int err);
};

namespace CPUTOSServices
{
    CPUTResult OpenMessageBox(std::wstring title, std::wstring text);
    CPUTResult OpenMessageBox(std::string title, std::string text);
};

#endif // CPUTOSSERVICES_H