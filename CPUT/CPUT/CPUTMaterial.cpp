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
#include "CPUT.h"
#include "CPUTAssetLibrary.h"

#ifdef CPUT_FOR_DX11
#include "CPUTMaterialDX11.h"
#else    
    #error You must supply a target graphics API (ex: #define CPUT_FOR_DX11), or implement the target API for this file.
#endif


//--------------------------------------------------------------------------------------
CPUTMaterial *CPUTMaterial::CreateMaterial(
    const cString   &absolutePathAndFilename,
    const CPUTModel *pModel,
          int        meshIndex,
    const char     **pShaderMacros, // Note: this is honored only on first load.  Subsequent GetMaterial calls will return the material with shaders as compiled with original macros.
          int        numSystemMaterials,
          cString   *pSystemMaterialNames
){
    // Create the material and load it from file.
#ifdef CPUT_FOR_DX11
    CPUTMaterial *pMaterial = new CPUTMaterialDX11();
#else    
    #error You must supply a target graphics API (ex: #define CPUT_FOR_DX11), or implement the target API for this file.
#endif
    CPUTResult result = pMaterial->LoadMaterial( absolutePathAndFilename, pModel, meshIndex, pShaderMacros, numSystemMaterials, pSystemMaterialNames );
    ASSERT( CPUTSUCCESS(result), _L("\nError - CPUTAssetLibrary::GetMaterial() - Error in material file: '")+absolutePathAndFilename+_L("'") );
    UNREFERENCED_PARAMETER(result);

    // Add the material to the asset library.
    if( pModel && pMaterial->MaterialRequiresPerModelPayload() )
    {
        CPUTAssetLibrary::GetAssetLibrary()->AddMaterial( absolutePathAndFilename, pMaterial, pShaderMacros, pModel, meshIndex );
    } else
    {
        CPUTAssetLibrary::GetAssetLibrary()->AddMaterial( absolutePathAndFilename, pMaterial, pShaderMacros );
    }

    return pMaterial;
}
