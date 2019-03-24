//--------------------------------------------------------------------------------------
// Copyright 2013 Intel Corporation
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
#include "CPUTConfigBlock.h"
#include "CPUTOSServices.h"

CPUTConfigEntry  &CPUTConfigEntry::sNullConfigValue = CPUTConfigEntry(_L(""), _L(""));
CPUTConfigEntryA  &CPUTConfigEntryA::sNullConfigValue = CPUTConfigEntryA("", "");

//----------------------------------------------------------------
static bool iswhite(char ch)
{
    return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
}

template<typename T>
static void RemoveWhitespace(T &start, T &end)
{
    while (start < end && iswhite(*start))
    {
        ++start;
    }
    while (end > start && iswhite(*(end - 1)))
    {
        --end;
    }
}

//----------------------------------------------------------------
static bool ReadLine(const char **ppStart, const char **ppEnd, const char **ppCur)
{
    const char *pCur = *ppCur;
    if (!*pCur) // check for EOF
    {
        return false;
    }

    // We're at the start of a line now, skip leading whitespace
    while (*pCur == ' ' || *pCur == '\t')
    {
        ++pCur;
    }

    *ppStart = pCur;

    // Forward to the end of the line and keep track of last non-whitespace char
    const char *pEnd = pCur;
    for (;;)
    {
        char ch = *pCur++;
        if (!ch)
        {
            --pCur; // terminating NUL isn't consumed
            break;
        }
        else if (ch == '\n')
        {
            break;
        }
        else if (!iswhite(ch))
        {
            pEnd = pCur;
        }
    }

    *ppEnd = pEnd;
    *ppCur = pCur;
    return true;
}

//----------------------------------------------------------------
static const char *FindFirst(const char *start, const char *end, char ch)
{
    const char *p = start;
    while (p < end && *p != ch)
    {
        ++p;
    }
    return p;
}

static const char *FindLast(const char *start, const char *end, char ch)
{
    const char *p = end;
    while (--p >= start && *p != ch)
    {
    }
    return p;
}

static void AssignStr(cString &dest, const char *start, const char *end, _locale_t locale)
{
    dest.clear();
    if (end <= start)
    {
        return;
    }

    static const int NBUF = 64;
    wchar_t buf[NBUF];
    int nb = 0;

    size_t len = end - start;
    size_t initial = len + 1; // assume most characters are 1-byte
    dest.reserve(initial);

    const char *p = start;
    while (p < end)
    {
        int len = _mbtowc_l(&buf[nb++], p, end - p, locale);
        if (len < 1)
        {
            break;
        }

        p += len;
        if (p >= end || nb >= NBUF)
        {
            dest.append(buf, nb);
            nb = 0;
        }
    }
}

//----------------------------------------------------------------
void CPUTConfigEntry::ValueAsFloatArray(float *pFloats, int count)
{
    cString valueCopy = szValue;
    TCHAR *szOrigValue = (TCHAR*)valueCopy.c_str();

    TCHAR *szNewValue = NULL;
    TCHAR *szCurrValue = wcstok_s(szOrigValue, _L(" "), &szNewValue);
    for(int clear = 0; clear < count; clear++)
    {
        pFloats[clear] = 0.0f;
    }
    for(int ii=0;ii<count;++ii)
    {
        if(szCurrValue == NULL)
        {
            return;
        }
        pFloats[ii] = (float) _wtof(szCurrValue);
        szCurrValue = wcstok_s(NULL, _L(" "), &szNewValue);
    }
}
//----------------------------------------------------------------
CPUTConfigEntry *CPUTConfigBlock::GetValue(int nValueIndex)
{
    if(nValueIndex < 0 || nValueIndex >= mnValueCount)
    {
        return NULL;
    }
    return &mpValues[nValueIndex];
}
//----------------------------------------------------------------
CPUTConfigEntry *CPUTConfigBlock::AddValue(const cString &szName, const cString &szValue )
{
    // TODO: What should we do if it already exists?
    CPUTConfigEntry *pEntry = &mpValues[mnValueCount++];
    pEntry->szName  = szName;
    pEntry->szValue = szValue;
    return pEntry;
}
//----------------------------------------------------------------
CPUTConfigEntry *CPUTConfigBlock::GetValueByName(const cString &szName)
{
    for(int ii=0; ii<mnValueCount; ++ii)
    {
        if( _wcsicmp( mpValues[ii].szName.c_str(), szName.c_str() ) == 0 )
        {
            return &mpValues[ii];
        }
    }
    // not found - return an 'empty' object to avoid crashes/extra error checking
    return &CPUTConfigEntry::sNullConfigValue;
}

//----------------------------------------------------------------
CPUTConfigFile::~CPUTConfigFile()
{
    if(mpBlocks)
    {
        delete [] mpBlocks;
        mpBlocks = 0;
    }
    mnBlockCount = 0;
}

//----------------------------------------------------------------
CPUTResult CPUTConfigFile::LoadFile(const cString &szFilename)
{
    // Load the file
    cString             szCurrLine;
    CPUTConfigBlock    *pCurrBlock = NULL;
    FILE               *pFile = NULL;
    int                 nCurrBlock = 0;
    CPUTResult result = CPUTFileSystem::OpenFile(szFilename, &pFile);
    if(CPUTFAILED(result))
    {
        return result;
    }

    _locale_t locale = _get_current_locale();

    /* Determine file size */
    fseek(pFile, 0, SEEK_END);
    int nBytes = ftell(pFile); // for text files, this is an overestimate
    fseek(pFile, 0, SEEK_SET);

    /* Read the whole thing */
    char *pFileContents = new char[nBytes + 1];
    nBytes = (int)fread(pFileContents, 1, nBytes, pFile);
    fclose(pFile);

    pFileContents[nBytes] = 0; // add 0-terminator

    /* Count the number of blocks */
    const char *pCur = pFileContents;
    const char *pStart, *pEnd;

    while(ReadLine(&pStart, &pEnd, &pCur))
    {
        const char *pOpen = FindFirst(pStart, pEnd, '[');
        const char *pClose = FindLast(pOpen + 1, pEnd, ']');
        if (pOpen < pClose)
        {
            // This line is a valid block header
            mnBlockCount++;
        }
    }

    // For files that don't have any blocks, just add the entire file to one block
    if(mnBlockCount == 0)
    {
        mnBlockCount   = 1;
    }

    pCur = pFileContents;
    mpBlocks = new CPUTConfigBlock[mnBlockCount];
    pCurrBlock = mpBlocks;

    /* Find the first block first */
    while(ReadLine(&pStart, &pEnd, &pCur))
    {
        const char *pOpen = FindFirst(pStart, pEnd, '[');
        const char *pClose = FindLast(pOpen + 1, pEnd, ']');
        if (pOpen < pClose)
        {
            // This line is a valid block header
            pCurrBlock = mpBlocks + nCurrBlock++;
            AssignStr(pCurrBlock->mszName, pOpen + 1, pClose, locale);
        }
        else if (pStart < pEnd)
        {
            // It's a value
            if (pCurrBlock == NULL)
            {
                continue;
            }

            const char *pEquals = FindFirst(pStart, pEnd, '=');
            if (pEquals == pEnd)
            {
                // No value, just a key, save it anyway
                // Optimistically, we assume it's new
                cString &name = pCurrBlock->mpValues[pCurrBlock->mnValueCount].szName;
                AssignStr(name, pStart, pEnd, locale);

#ifdef ELIMINATE_DUPLICATES
                bool dup = false;
                for(int ii=0;ii<pCurrBlock->mnValueCount;++ii)
                {
                    if( _wcsicmp( pCurrBlock->mpValues[ii].szName.c_str(), name.c_str() ) == 0 )
                    {
                        dup = true;
                        break;
                    }
                }
                if(!dup)
                {
                    pCurrBlock->mnValueCount++;
                }
#else
                pCurrBlock->mnValueCount++;
#endif
            }
            else
            {
                const char *pNameStart = pStart;
                const char *pNameEnd = pEquals;
                const char *pValStart = pEquals + 1;
                const char *pValEnd = pEnd;

                RemoveWhitespace(pNameStart, pNameEnd);
                RemoveWhitespace(pValStart, pValEnd);

                // Optimistically assume the name is new
                cString &name = pCurrBlock->mpValues[pCurrBlock->mnValueCount].szName;
                AssignStr(name, pNameStart, pNameEnd, locale);

#ifdef ELIMINATE_DUPLICATES
                bool dup = false;
                for(int ii=0;ii<pCurrBlock->mnValueCount;++ii)
                {
                    if( _wcsicmp( pCurrBlock->mpValues[ii].szName.c_str(), name.c_str() ) == 0 )
                    {
                        dup = true;
                        break;
                    }
                }
                if(!dup)
                {
                    AssignStr(pCurrBlock->mpValues[pCurrBlock->mnValueCount].szValue, pValStart, pValEnd, locale);
                    pCurrBlock->mnValueCount++;
                }
#else
                AssignStr(pCurrBlock->mpValues[pCurrBlock->mnValueCount].szValue, pValStart, pValEnd, locale);
                pCurrBlock->mnValueCount++;
#endif
            }
        }
    }

    delete[] pFileContents;
    return CPUT_SUCCESS;
}

//----------------------------------------------------------------
CPUTConfigBlock *CPUTConfigFile::GetBlock(int nBlockIndex)
{
    if(nBlockIndex >= mnBlockCount || nBlockIndex < 0)
    {
        return NULL;
    }

    return &mpBlocks[nBlockIndex];
}

//----------------------------------------------------------------
CPUTConfigBlock *CPUTConfigFile::GetBlockByName(const cString &szBlockName)
{
    cString szString = szBlockName;
    for(int ii=0; ii<mnBlockCount; ++ii)
    {
        if( _wcsicmp( mpBlocks[ii].mszName.c_str(), szString.c_str()) == 0 )
        {
            return &mpBlocks[ii];
        }
    }
    return NULL;
}



//----------------------------------------------------------------
// The second half of this file contains the *A implementations
//----------------------------------------------------------------
void CPUTConfigEntryA::ValueAsFloatArray(float *pFloats, int count)
{
    const char *szOrigValue = szValue.c_str();
    char *pValueString = new char[szValue.length() + 1];
    

    strcpy_s(pValueString, szValue.length() + 1, szOrigValue);

    char *szNewValue = NULL;
    char *szCurrValue = strtok_s(pValueString, " ", &szNewValue);

    for(int clear = 0; clear < count; clear++)
    {
        pFloats[clear] = 0.0f;
    }
    for(int ii=0;ii<count;++ii)
    {
        if(szCurrValue == NULL)
        {
            return;
        }
        pFloats[ii] = (float) atof(szCurrValue);
        szCurrValue = strtok_s(NULL, " ", &szNewValue);

    }
}

//----------------------------------------------------------------
CPUTConfigEntryA *CPUTConfigBlockA::GetValue(int nValueIndex)
{
    if(nValueIndex < 0 || nValueIndex >= mnValueCount)
    {
        return NULL;
    }
    return &mpValues[nValueIndex];
}

//----------------------------------------------------------------
CPUTConfigEntryA *CPUTConfigBlockA::AddValue(const std::string &szName, const std::string &szValue )
{
    // TODO: What should we do if it already exists?
    CPUTConfigEntryA *pEntry = &mpValues[mnValueCount++];
    pEntry->szName  = szName;
    pEntry->szValue = szValue;
    return pEntry;
}

//----------------------------------------------------------------
CPUTConfigEntryA *CPUTConfigBlockA::GetValueByName(const std::string &szName)
{
    for(int ii=0; ii<mnValueCount; ++ii)
    {
        const std::string &valName = mpValues[ii].szName;
        if(valName.size() != szName.size())
        {
            continue;
        }

        size_t j = 0;
        while (j < valName.size() && tolower(szName[j]) == tolower(valName[j]))
        {
            ++j;
        }

        if (j == valName.size()) // match
        {
            return &mpValues[ii];
        }
    }

    // not found - return an 'empty' object to avoid crashes/extra error checking
    return &CPUTConfigEntryA::sNullConfigValue;
}

//----------------------------------------------------------------
CPUTConfigFileA::~CPUTConfigFileA()
{
    if(mpBlocks)
    {
        delete [] mpBlocks;
        mpBlocks = 0;
    }
    mnBlockCount = 0;
}

//----------------------------------------------------------------
CPUTResult CPUTConfigFileA::LoadFile(const std::string &szFilename)
{
    // Load the file
    std::string         szCurrLine;
    CPUTConfigBlockA   *pCurrBlock = NULL;
    FILE               *pFile = NULL;
    int                 nCurrBlock = 0;
    CPUTResult result = CPUTFileSystem::OpenFile(szFilename, &pFile);
    if(CPUTFAILED(result))
    {
        return result;
    }

    _locale_t locale = _get_current_locale();

    /* Determine file size */
    fseek(pFile, 0, SEEK_END);
    int nBytes = ftell(pFile); // for text files, this is an overestimate
    fseek(pFile, 0, SEEK_SET);

    /* Read the whole thing */
    char *pFileContents = new char[nBytes + 1];
    nBytes = (int)fread(pFileContents, 1, nBytes, pFile);
    fclose(pFile);

    pFileContents[nBytes] = 0; // add 0-terminator

    /* Count the number of blocks */
    const char *pCur = pFileContents;
    const char *pStart, *pEnd;

    while(ReadLine(&pStart, &pEnd, &pCur))
    {
        const char *pOpen = FindFirst(pStart, pEnd, '[');
        const char *pClose = FindLast(pOpen + 1, pEnd, ']');
        if (pOpen < pClose)
        {
            // This line is a valid block header
            mnBlockCount++;
        }
    }

    // For files that don't have any blocks, just add the entire file to one block
    if(mnBlockCount == 0)
    {
        mnBlockCount   = 1;
    }

    pCur = pFileContents;
    mpBlocks = new CPUTConfigBlockA[mnBlockCount];
    pCurrBlock = mpBlocks;

    /* Find the first block first */
    while(ReadLine(&pStart, &pEnd, &pCur))
    {
        const char *pOpen = FindFirst(pStart, pEnd, '[');
        const char *pClose = FindLast(pOpen + 1, pEnd, ']');
        if (pOpen < pClose)
        {
            // This line is a valid block header
            pCurrBlock = mpBlocks + nCurrBlock++;
            pCurrBlock->mszName.assign(pOpen + 1, pClose);
        }
        else if (pStart < pEnd)
        {
            // It's a value
            if (pCurrBlock == NULL)
            {
                continue;
            }

            const char *pEquals = FindFirst(pStart, pEnd, '=');
            if (pEquals == pEnd)
            {
                // No value, just a key, save it anyway
                // Optimistically, we assume it's new
                std::string &name = pCurrBlock->mpValues[pCurrBlock->mnValueCount].szName;
                name.assign(pStart, pEnd);

#ifdef ELIMINATE_DUPLICATES
                bool dup = false;
                for(int ii=0;ii<pCurrBlock->mnValueCount;++ii)
                {
                    if( stricmp( pCurrBlock->mpValues[ii].szName.c_str(), name.c_str() ) == 0 )
                    {
                        dup = true;
                        break;
                    }
                }
                if(!dup)
                {
                    pCurrBlock->mnValueCount++;
                }
#else
                pCurrBlock->mnValueCount++;
#endif
            }
            else
            {
                const char *pNameStart = pStart;
                const char *pNameEnd = pEquals;
                const char *pValStart = pEquals + 1;
                const char *pValEnd = pEnd;

                RemoveWhitespace(pNameStart, pNameEnd);
                RemoveWhitespace(pValStart, pValEnd);

                // Optimistically assume the name is new
                std::string &name = pCurrBlock->mpValues[pCurrBlock->mnValueCount].szName;
                name.assign(pNameStart, pNameEnd);

#ifdef ELIMINATE_DUPLICATES
                bool dup = false;
                for(int ii=0;ii<pCurrBlock->mnValueCount;++ii)
                {
                    if( stricmp( pCurrBlock->mpValues[ii].szName.c_str(), name.c_str() ) == 0 )
                    {
                        dup = true;
                        break;
                    }
                }
                if(!dup)
                {
                    pCurrBlock->mpValues[pCurrBlock->mnValueCount].szValue.assign(pValStart, pValEnd);
                    pCurrBlock->mnValueCount++;
                }
#else
                pCurrBlock->mpValues[pCurrBlock->mnValueCount].szValue.assign(pValStart, pValEnd);
                pCurrBlock->mnValueCount++;
#endif
            }
        }
    }
    delete[] pFileContents;
    return CPUT_SUCCESS;
}

//----------------------------------------------------------------
CPUTConfigBlockA *CPUTConfigFileA::GetBlock(int nBlockIndex)
{
    if(nBlockIndex >= mnBlockCount || nBlockIndex < 0)
    {
        return NULL;
    }
    return &mpBlocks[nBlockIndex];
}

//----------------------------------------------------------------
CPUTConfigBlockA *CPUTConfigFileA::GetBlockByName(const std::string &szBlockName)
{
    std::string szString = szBlockName;
    for(int ii=0; ii<mnBlockCount; ++ii)
    {
        if( _stricmp( mpBlocks[ii].mszName.c_str(), szString.c_str() ) == 0 )
        {
            return &mpBlocks[ii];
        }
    }
    return NULL;
}
