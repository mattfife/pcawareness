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
#include <algorithm> // for std::string.transform()
#include "CPUTAssetLibrary.h"
#include "CPUTRenderNode.h"
#include "CPUTAssetSet.h"
#include "CPUTMaterial.h"
#include "CPUTRenderStateBlock.h"
#include "CPUTModel.h"
#include "CPUTCamera.h"
#include "CPUTLight.h"
#include "CPUTFont.h"
#include "CPUTBuffer.h"

// TODO: How do we want to store both DX and OGL assets in the same library?
//       Could use API-specific libraries.  Could move both platforms to single file.  etc.
// CPUTAssetLibrary   *CPUTAssetLibrary::mpAssetLibrary = new CPUTAssetLibraryDX11();
CPUTAssetListEntry *CPUTAssetLibrary::mpAssetSetList = NULL;
CPUTAssetListEntry *CPUTAssetLibrary::mpNullNodeList = NULL;
CPUTAssetListEntry *CPUTAssetLibrary::mpModelList = NULL;
CPUTAssetListEntry *CPUTAssetLibrary::mpCameraList = NULL;
CPUTAssetListEntry *CPUTAssetLibrary::mpLightList = NULL;
CPUTAssetListEntry *CPUTAssetLibrary::mpMaterialList = NULL;
CPUTAssetListEntry *CPUTAssetLibrary::mpTextureList = NULL;
CPUTAssetListEntry *CPUTAssetLibrary::mpBufferList = NULL;
CPUTAssetListEntry *CPUTAssetLibrary::mpConstantBufferList = NULL;
CPUTAssetListEntry *CPUTAssetLibrary::mpRenderStateBlockList = NULL;
CPUTAssetListEntry *CPUTAssetLibrary::mpFontList = NULL;

CPUTAssetListEntry *CPUTAssetLibrary::mpInstancedMaterialList = NULL;
CPUTAssetListEntry *CPUTAssetLibrary::mpInstancedTextureList = NULL;
CPUTAssetListEntry *CPUTAssetLibrary::mpInstancedBufferList = NULL;
CPUTAssetListEntry *CPUTAssetLibrary::mpInstancedConstantBufferList = NULL;

CPUTAssetListEntry *CPUTAssetLibrary::mpAssetSetListTail = NULL;
CPUTAssetListEntry *CPUTAssetLibrary::mpNullNodeListTail = NULL;
CPUTAssetListEntry *CPUTAssetLibrary::mpModelListTail = NULL;
CPUTAssetListEntry *CPUTAssetLibrary::mpCameraListTail = NULL;
CPUTAssetListEntry *CPUTAssetLibrary::mpLightListTail = NULL;
CPUTAssetListEntry *CPUTAssetLibrary::mpMaterialListTail = NULL;
CPUTAssetListEntry *CPUTAssetLibrary::mpTextureListTail = NULL;
CPUTAssetListEntry *CPUTAssetLibrary::mpBufferListTail = NULL;
CPUTAssetListEntry *CPUTAssetLibrary::mpConstantBufferListTail = NULL;
CPUTAssetListEntry *CPUTAssetLibrary::mpRenderStateBlockListTail = NULL;
CPUTAssetListEntry *CPUTAssetLibrary::mpFontListTail = NULL;

CPUTAssetListEntry *CPUTAssetLibrary::mpInstancedMaterialListTail = NULL;
CPUTAssetListEntry *CPUTAssetLibrary::mpInstancedTextureListTail = NULL;
CPUTAssetListEntry *CPUTAssetLibrary::mpInstancedBufferListTail = NULL;
CPUTAssetListEntry *CPUTAssetLibrary::mpInstancedConstantBufferListTail = NULL;

//-----------------------------------------------------------------------------
void CPUTAssetLibrary::ReleaseTexturesAndBuffers()
{
    CPUTAssetListEntry *pMaterial = mpMaterialList;
    while( pMaterial )
    {
        ((CPUTMaterial*)pMaterial->pData)->ReleaseTexturesAndBuffers();
        char **pTmp = pMaterial->pShaderMacros;
        while( pTmp && *pTmp )
        {
            SAFE_DELETE(*pTmp);
            ++pTmp;
        }
        SAFE_DELETE_ARRAY( pMaterial->pShaderMacros );
        pMaterial = pMaterial->pNext;
    }
    pMaterial = mpInstancedMaterialList;
    while( pMaterial )
    {
        ((CPUTMaterial*)pMaterial->pData)->ReleaseTexturesAndBuffers();
        char **pTmp = pMaterial->pShaderMacros;
        while( pTmp && *pTmp )
        {
            SAFE_DELETE(*pTmp);
            ++pTmp;
        }
        SAFE_DELETE_ARRAY( pMaterial->pShaderMacros );
        pMaterial = pMaterial->pNext;
    }
}

//-----------------------------------------------------------------------------
void CPUTAssetLibrary::RebindTexturesAndBuffers()
{
    CPUTAssetListEntry *pMaterial = mpMaterialList;
    while( pMaterial )
    {
        ((CPUTMaterial*)pMaterial->pData)->RebindTexturesAndBuffers();
        pMaterial = pMaterial->pNext;
    }
    pMaterial = mpInstancedMaterialList;
    while( pMaterial )
    {
        ((CPUTMaterial*)pMaterial->pData)->RebindTexturesAndBuffers();
        pMaterial = pMaterial->pNext;
    }
}

//-----------------------------------------------------------------------------
void CPUTAssetLibrary::DeleteAssetLibrary()
{
    SAFE_DELETE(mpAssetLibrary);
}

//-----------------------------------------------------------------------------
void CPUTAssetLibrary::ReleaseAllLibraryLists()
{
    // Release philosophy:  Everyone that references releases.  If node refers to parent, then it should release parent, etc...
    // TODO: Traverse lists.  Print names and ref counts (as debug aid)
#undef SAFE_RELEASE_LIST
#define SAFE_RELEASE_LIST(x) {ReleaseList(x); x = NULL;}

    SAFE_RELEASE_LIST(mpAssetSetList);
    SAFE_RELEASE_LIST(mpMaterialList);
    SAFE_RELEASE_LIST(mpModelList);
    SAFE_RELEASE_LIST(mpLightList);
    SAFE_RELEASE_LIST(mpCameraList);
    SAFE_RELEASE_LIST(mpNullNodeList);
    SAFE_RELEASE_LIST(mpTextureList );
    SAFE_RELEASE_LIST(mpBufferList );
    SAFE_RELEASE_LIST(mpConstantBufferList );
    SAFE_RELEASE_LIST(mpRenderStateBlockList );
    SAFE_RELEASE_LIST(mpFontList);

    SAFE_RELEASE_LIST(mpInstancedMaterialList);
    SAFE_RELEASE_LIST(mpInstancedTextureList);
    SAFE_RELEASE_LIST(mpInstancedBufferList);
    SAFE_RELEASE_LIST(mpInstancedConstantBufferList);

    // The following -specific items are destroyed in the derived class
    // TODO.  Move their declaration and definition to the derived class too
    // SAFE_RELEASE_LIST(mpPixelShaderList);
    // SAFE_RELEASE_LIST(mpVertexShaderList);
    // SAFE_RELEASE_LIST(mpGeometryShaderList);
}

//-----------------------------------------------------------------------------
void CPUTAssetLibrary::ReleaseList(CPUTAssetListEntry *pLibraryRoot)
{
    CPUTAssetListEntry *pNext;
    for( CPUTAssetListEntry *pNodeEntry = pLibraryRoot; NULL != pNodeEntry; pNodeEntry = pNext )
    {
        pNext = pNodeEntry->pNext;
        CPUTRefCount *pRefCountedNode = (CPUTRefCount*)pNodeEntry->pData;
        pRefCountedNode->Release();
        HEAPCHECK;
        char **pTmp = pNodeEntry->pShaderMacros;
        while( pTmp && *pTmp )
        {
            SAFE_DELETE(*pTmp);
            ++pTmp;
        }
        SAFE_DELETE_ARRAY( pNodeEntry->pShaderMacros );
        delete pNodeEntry;
    }
}

//-----------------------------------------------------------------------------
bool ShaderDefinesMatch( char **pShaderMacros1, char **pShaderMacros2 )
{
    // Compare macros in pShaderMacros1 with macros in pShaderMacros2.
    // Return true if the lists are identical.  Return false if not.
    // Note: this function directly compares the lists.  It doesn't sort before compare,
    // or otherwise search for matches.  For now, rely on caller to sort lists if desired.
    // TODO/TBD: If this later turns out to be an issue, then we can modify this function, or the shader-macro loading code.

    char **p1=(char**)pShaderMacros1, **p2=(char**)pShaderMacros2;
    if( (!p1 && p2) || (p1 && !p2) )
    {
        // Only one list is empty
        return false;
    }
    if( !p1 && !p2 )
    {
        // Both lists are empty.
        return true;
    }
    while( *p1 && *p2 )
    {
        if( strcmp( *p1, *p2 ) != 0 )
        {
            return false;
        }
        ++p1;
        ++p2;
    }
    // If both p1 and p2 are NULL, then the lists are the same.  If one of them is not NULL, then the lists are different.
    return ( !p1 && !p2 );
}

// Find an asset in a specific library
// ** Does not Addref() returned items **
// Asset library doesn't care if we're using absolute paths for names or not, it
// just adds/finds/deletes the matching string literal.
//-----------------------------------------------------------------------------
void *CPUTAssetLibrary::FindAsset(
    const cString &name,
    CPUTAssetListEntry *pList,
    bool nameIsFullPathAndFilename,
    const char **pShaderMacros,
    const CPUTModel *pModel,
    int meshIndex
){
    cString absolutePathAndFilename;
    if( !nameIsFullPathAndFilename )
    {
        CPUTFileSystem::ResolveAbsolutePathAndFilename( mAssetSetDirectoryName + name, &absolutePathAndFilename);
    }
    else
    {
        absolutePathAndFilename = name;
    }
    UINT hash = CPUTComputeHash( absolutePathAndFilename );

    while(NULL!=pList)
    {
        if(    pModel    == pList->pModel
            && meshIndex == pList->meshIndex
            && hash      == pList->hash
            && (0 == _wcsicmp( absolutePathAndFilename.data(), pList->name.data() ))
            && ShaderDefinesMatch( (char**)pShaderMacros, pList->pShaderMacros )
        )
        {
            return (void*)pList->pData;
        }
        pList = pList->pNext;
    }
    return NULL;
}

//-----------------------------------------------------------------------------
void CPUTAssetLibrary::AddAsset(
    const cString       &name,
    void                *pAsset,
    CPUTAssetListEntry **pHead,
    CPUTAssetListEntry **pTail,
    const char         **pShaderMacros,
    const CPUTModel     *pModel,
    int                  meshIndex
){
    // convert string to lowercase
    cString lowercaseName = name;
    std::transform(lowercaseName.begin(), lowercaseName.end(), lowercaseName.begin(), ::tolower);

#ifdef DEBUG
    // Do we already have one by this name?
    for( CPUTAssetListEntry *pCur=*pHead; NULL!=pCur; pCur=pCur->pNext )
    {
        // Assert that we haven't added one with this name
        ASSERT( !((0 == _wcsicmp( pCur->name.data(), name.data() )) && (pCur->pModel == pModel) && (pCur->meshIndex == meshIndex) ), _L("Warning: asset ")+name+_L(" already exists") );
    }
#endif
    CPUTAssetListEntry **pNext = *pTail ? &(*pTail)->pNext : pHead;

    *pTail = new CPUTAssetListEntry(); // TODO: init via constructor
    if(!*pNext ) *pNext = *pTail;

    (*pTail)->hash       = CPUTComputeHash(name);
    (*pTail)->name       = name;
    (*pTail)->pData      = pAsset;
    (*pTail)->pModel     = pModel;
    (*pTail)->meshIndex  = meshIndex;
    (*pTail)->pNext      = NULL;

    // Copy the shader macros
    char **p1=(char**)pShaderMacros;
    int count = 0;
    if( p1 )
    {
        while( *p1 )
        {
            ++count;
            ++p1;
        }
        (*pTail)->pShaderMacros = new char *[count+2];
        p1 = (char**)pShaderMacros;
        char **p2 = (*pTail)->pShaderMacros;
        while( *p1 && *p2 )
        {
            *p2 = new char[strlen(*p1)+1];
            strcpy( *p2, *p1 );
            ++p1;
            ++p2;
        }
        // Set the macro's name and value to NULL
        *(p2++) = NULL;
        *p2 = NULL;
    } else
    {
        (*pTail)->pShaderMacros = NULL;
    }

    pTail = &(*pTail)->pNext;

    // TODO: Our assets are not yet all derived from CPUTRenderNode.
    // TODO: For now, rely on caller performing the AddRef() as it knows the assets type.
    ((CPUTRefCount*)pAsset)->AddRef();
}

//-----------------------------------------------------------------------------
void CPUTAssetLibrary::AddAssetInstance(
    const cString       &name,
    void                *pAsset,
    CPUTAssetListEntry **pHead,
    CPUTAssetListEntry **pTail,
    CPUTAssetListEntry **pInstancedListHead,
    CPUTAssetListEntry **pInstancedListTail,
    const char         **pShaderMacros,
    const CPUTModel     *pModel,
    int                  meshIndex
)
{
    if( pModel )
    {
       AddAsset( name, pAsset, pInstancedListHead, pInstancedListTail, pShaderMacros, pModel, meshIndex );
    } else
    {
        AddAsset( name, pAsset, pHead, pTail );
    }
}

//-----------------------------------------------------------------------------
CPUTRenderStateBlock *CPUTAssetLibrary::GetRenderStateBlock(const cString &name, bool nameIsFullPathAndFilename )
{
    // Resolve name to absolute path before searching
    cString finalName;

    if( name.at(0) == '%' )
    {
        finalName = mSystemDirectoryName + _L("/Shader/") + name.substr(1);  // TODO: Instead of having the Shader/ directory hardcoded here it could be set like the normal material directory. But then there would need to be a bunch new variables like SetSystemMaterialDirectory
        CPUTFileSystem::ResolveAbsolutePathAndFilename(finalName, &finalName);
    } else if( name.at(0) == '$' )
    {
        finalName = name;
    } else
    {
        CPUTFileSystem::ResolveAbsolutePathAndFilename( nameIsFullPathAndFilename? name : (mShaderDirectoryName + name), &finalName);
    }

    // see if the render state block is already in the library
    CPUTRenderStateBlock *pRenderStateBlock = FindRenderStateBlock(finalName, true);
    if(NULL==pRenderStateBlock)
    {
        return CPUTRenderStateBlock::CreateRenderStateBlock( name, finalName );
    }
    pRenderStateBlock->AddRef();
    return pRenderStateBlock;
}

//-----------------------------------------------------------------------------
CPUTAssetSet *CPUTAssetLibrary::GetAssetSet( const cString &name, bool nameIsFullPathAndFilename, int numSystemMaterials, cString *pSystemMaterialNames )
{
    // Resolve the absolute path
    cString absolutePathAndFilename;
    CPUTFileSystem::ResolveAbsolutePathAndFilename( nameIsFullPathAndFilename ? name
        : (mAssetSetDirectoryName + name + _L(".set")), &absolutePathAndFilename );
    absolutePathAndFilename = nameIsFullPathAndFilename ? name : absolutePathAndFilename;

    CPUTAssetSet *pAssetSet = FindAssetSet(absolutePathAndFilename, true);
    if(NULL == pAssetSet)
    {
        return CPUTAssetSet::CreateAssetSet( name, absolutePathAndFilename, numSystemMaterials, pSystemMaterialNames );
    }
    pAssetSet->AddRef();
    return pAssetSet;
}


// TODO: All of these Get() functions look very similar.
// Keep them all for their interface, but have them call a common function
//-----------------------------------------------------------------------------
CPUTMaterial *CPUTAssetLibrary::GetMaterial(
    const cString   &name,
          bool       nameIsFullPathAndFilename,
    const CPUTModel *pModel,
          int        meshIndex,
    const char     **pShaderMacros, // Note: this is honored only on first load.  Subsequent GetMaterial calls will return the material with shaders as compiled with original macros.
          int        numSystemMaterials,
          cString   *pSystemMaterialNames  // Note: this is honored only on first load.  Subsequent GetMaterial calls will return the material with shaders as compiled with original macros.
){
    // Resolve name to absolute path before searching
    cString absolutePathAndFilename;
    if (name[0] == '%')
    {
        absolutePathAndFilename = mSystemDirectoryName + _L("/Material/") + name.substr(1) + _L(".mtl");  // TODO: Instead of having the Material/directory hardcoded here it could be set like the normal material directory. But then there would need to be a bunch new variables like SetSystemMaterialDirectory
        CPUTFileSystem::ResolveAbsolutePathAndFilename(absolutePathAndFilename, &absolutePathAndFilename);
    } else if( !nameIsFullPathAndFilename )
    {
        CPUTFileSystem::ResolveAbsolutePathAndFilename( mMaterialDirectoryName + name + _L(".mtl"), &absolutePathAndFilename);
    } else
    {
        absolutePathAndFilename = name;
    }

    CPUTMaterial *pMaterial=NULL;
    if( pModel )
    {
        pMaterial = pModel->GetMaterial( meshIndex );
        if( pMaterial )
        {
            pMaterial->AddRef();
            return pMaterial;
        }
    }

    if( !pMaterial && !pShaderMacros )
    {
        // Loading a non-instanced material (or, the master)
        pMaterial = FindMaterial(absolutePathAndFilename, true);
    }

    // If the material has per-model properties, then we need a material clone
    if( pMaterial )
    {
        pMaterial->AddRef();
    }
    else
    {
        pMaterial = CPUTMaterial::CreateMaterial( absolutePathAndFilename, pModel, meshIndex, pShaderMacros, numSystemMaterials, pSystemMaterialNames );
        ASSERT( pMaterial, _L("Failed creating material.") );
    }

    if( pModel )
    {
        bool needClone = pMaterial->MaterialRequiresPerModelPayload();
        if( needClone )
        {
            CPUTMaterial *pClone = pMaterial->CloneMaterial( absolutePathAndFilename, pModel, meshIndex);
            pMaterial->Release(); // We don't want to hold a reference to the master.
            return pClone;
        }
    }
    // Not looking for an instance, so return what we found
    return pMaterial;
}

// Get CPUTModel from asset library
// If the model exists, then the existing model is Addref'ed and returned
//-----------------------------------------------------------------------------
CPUTModel *CPUTAssetLibrary::GetModel(const cString &name, bool nameIsFullPathAndFilename)
{
    // Resolve name to absolute path before searching
    cString absolutePathAndFilename;

    if (!nameIsFullPathAndFilename && name.at(0) == '%')
    {
        CPUTFileSystem::ResolveAbsolutePathAndFilename(mSystemDirectoryName + _L("/Asset/") + name.substr(1) + _L(".mdl"), &absolutePathAndFilename);
    } else if (!nameIsFullPathAndFilename) {
        CPUTFileSystem::ResolveAbsolutePathAndFilename(mModelDirectoryName + name + _L(".mdl"), &absolutePathAndFilename);
    } else {
        CPUTFileSystem::ResolveAbsolutePathAndFilename(name, &absolutePathAndFilename);
    }

    // If we already have one by this name, then return it
    CPUTModel *pModel = FindModel(absolutePathAndFilename, true);
    if(NULL!=pModel)
    {
        pModel->AddRef();
        return pModel;
    }

    // Looks like no one calls GetModel().  Or, they never call it for missing models.
#if TODO // delegate
    // Model was not in the library, so create and load a new model
    pModel = new CPUTModel();
    pModel->LoadModelPayload(absolutePathAndFilename);
    AddModel(name, pModel);

    return CPUTModel::CreateMode( absolutePathAndFilename, aboslutePathAndFilename );
#endif

    return pModel;
}

//-----------------------------------------------------------------------------
CPUTCamera *CPUTAssetLibrary::GetCamera(const cString &name)
{
    // TODO: Should we prefix camera names with a path anyway?  To keek them unique?
    // If we already have one by this name, then return it
    CPUTCamera *pCamera = FindCamera(name, true);
    if(NULL!=pCamera)
    {
        pCamera->AddRef();
        return pCamera;
    }
    return NULL;
}

//-----------------------------------------------------------------------------
CPUTLight *CPUTAssetLibrary::GetLight(const cString &name)
{
    // If we already have one by this name, then return it
    CPUTLight *pLight = FindLight(name, true);
    if(NULL!=pLight)
    {
        pLight->AddRef();
        return pLight;
    }
    return NULL;
}

//-----------------------------------------------------------------------------
CPUTTexture *CPUTAssetLibrary::GetTexture(const cString &name, bool nameIsFullPathAndFilename, bool loadAsSRGB )
{
    cString finalName;

    if( name.at(0) == '%' )
    {
        finalName = mSystemDirectoryName + _L("/Texture/") + name.substr(1);  // TODO: Instead of having the Shader/ directory hardcoded here it could be set like the normal material directory. But then there would need to be a bunch new variables like SetSystemMaterialDirectory
        CPUTFileSystem::ResolveAbsolutePathAndFilename(finalName, &finalName);
    } else if( name.at(0) == '$' )
    {
        finalName = name;
    } else
    {
        CPUTFileSystem::ResolveAbsolutePathAndFilename( nameIsFullPathAndFilename? name : (mTextureDirectoryName + name), &finalName);
    }
    // If we already have one by this name, then return it
    CPUTTexture *pTexture = FindTexture(finalName, true);
    if(NULL==pTexture)
    {
        return CPUTTexture::CreateTexture( name, finalName, loadAsSRGB);
    }
    pTexture->AddRef();
    return pTexture;
}

//-----------------------------------------------------------------------------
CPUTBuffer *CPUTAssetLibrary::GetBuffer(const cString &name, const CPUTModel *pModel, int meshIndex )
{
    // If we already have one by this name, then return it
    CPUTBuffer *pBuffer = FindBuffer(name, true, pModel, meshIndex);
    ASSERT( pBuffer, _L("Can't find buffer ") + name );
    pBuffer->AddRef();
    return pBuffer;
}

//-----------------------------------------------------------------------------
CPUTBuffer *CPUTAssetLibrary::GetConstantBuffer(const cString &name, const CPUTModel *pModel, int meshIndex )
{
    // If we already have one by this name, then return it
    CPUTBuffer *pBuffer = FindConstantBuffer(name, true, pModel, meshIndex);
    ASSERT( pBuffer, _L("Can't find constant buffer ") + name );
    pBuffer->AddRef();
    return pBuffer;
}

//-----------------------------------------------------------------------------
CPUTFont *CPUTAssetLibrary::GetFont(const cString &name )
{
    // Resolve name to absolute path
    cString absolutePathAndFilename;
    CPUTFileSystem::ResolveAbsolutePathAndFilename( (mFontDirectoryName + name), &absolutePathAndFilename);

    // If we already have one by this name, then return it
    CPUTFont *pFont = FindFont(absolutePathAndFilename, true);
    if(NULL==pFont)
    {
        return CPUTFont::CreateFont( name, absolutePathAndFilename);
    }
    pFont->AddRef();
    return pFont;
}
