//
// Copyright 2016 Pixar
//
// Licensed under the Apache License, Version 2.0 (the "Apache License")
// with the following modification; you may not use this file except in
// compliance with the Apache License and the following modification to it:
// Section 6. Trademarks. is deleted and replaced with:
//
// 6. Trademarks. This License does not grant permission to use the trade
//    names, trademarks, service marks, or product names of the Licensor
//    and its affiliates, except as required to comply with Section 4(c) of
//    the License and to reproduce the content of the NOTICE file.
//
// You may obtain a copy of the Apache License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the Apache License with the above modification is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the Apache License for the specific
// language governing permissions and limitations under the Apache License.
//
#include "pxr/pxr.h"
#include "mayaReferenceUpdater.h"

#include <../fileio/utils/adaptor.h>
#include <../fileio/primUpdaterRegistry.h>
#include <../utils/util.h>
#include <../schemas/MayaReference.h>
#include <../fileio/translators/translatorMayaReference.h>

#include "pxr/base/gf/vec2f.h"
#include "pxr/base/tf/diagnostic.h"
#include "pxr/usd/sdf/path.h"
#include "pxr/usd/usd/timeCode.h"
#include "pxr/usd/usdGeom/tokens.h"
#include "pxr/usd/usdUtils/pipeline.h"

PXR_NAMESPACE_OPEN_SCOPE


PXRUSDMAYA_REGISTER_UPDATER(AL_usd_MayaReference, PxrUsdTranslators_MayaReferenceUpdater, (UsdMayaPrimUpdater::Supports::Push | UsdMayaPrimUpdater::Supports::Clear));

PxrUsdTranslators_MayaReferenceUpdater::PxrUsdTranslators_MayaReferenceUpdater(
        const MFnDependencyNode& depNodeFn,
        const SdfPath& usdPath) :
    UsdMayaPrimUpdater(depNodeFn, usdPath)
{
}

/* virtual */
bool
PxrUsdTranslators_MayaReferenceUpdater::Pull(UsdMayaPrimUpdaterContext* context)
{
    const UsdPrim& usdPrim = GetUsdPrim<AL_usd_MayaReference>(*context);
    const MObject& parentNode = GetMayaObject();
    
    UsdMayaTranslatorMayaReference::update(
        usdPrim,
        parentNode);

    return true;
}

/* virtual */
void
PxrUsdTranslators_MayaReferenceUpdater::Clear(UsdMayaPrimUpdaterContext* context)
{
    const UsdPrim& usdPrim = GetUsdPrim<AL_usd_MayaReference>(*context);
    const MObject& parentNode = GetMayaObject();

    UsdMayaTranslatorMayaReference::UnloadMayaReference(parentNode);
}

PXR_NAMESPACE_CLOSE_SCOPE
