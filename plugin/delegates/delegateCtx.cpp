#include "delegateCtx.h"

#include <pxr/base/gf/frustum.h>
#include <pxr/base/gf/plane.h>
#include <pxr/base/gf/range1d.h>
#include <pxr/imaging/hd/rprim.h>

#include <usdMaya/util.h>

#include <array>

PXR_NAMESPACE_OPEN_SCOPE

namespace {

SdfPath
_GetPrimPath(const SdfPath& base, const MDagPath& dg) {
    const auto mayaPath = PxrUsdMayaUtil::MDagPathToUsdPath(dg, false
#ifdef LUMA_USD_BUILD
            , false
#endif
        );
    if (mayaPath.IsEmpty()) { return {}; }
    const auto* chr = mayaPath.GetText();
    if (chr == nullptr) { return {}; };
    std::string s(chr + 1);
    if (s.empty()) { return {}; }
    return base.AppendPath(SdfPath(s));
}

}

HdMayaDelegateCtx::HdMayaDelegateCtx(
    HdRenderIndex* renderIndex,
    const SdfPath& delegateID)
    : HdMayaDelegate(renderIndex, delegateID),
     _rprimPath(delegateID.AppendPath(SdfPath(std::string("rprims")))),
     _sprimPath(delegateID.AppendPath(SdfPath(std::string("sprims")))) {
    _rprimCollection.SetName(TfToken("visible"));
    _rprimCollection.SetRootPath(_rprimPath);
    _rprimCollection.SetRenderTags({HdTokens->geometry});
    GetChangeTracker().AddCollection(TfToken("visible"));
}

void
HdMayaDelegateCtx::InsertRprim(const TfToken& typeId, const SdfPath& id, HdDirtyBits initialBits) {
    GetRenderIndex().InsertRprim(typeId, this, id);
    GetChangeTracker().RprimInserted(id, initialBits);
}

void
HdMayaDelegateCtx::InsertSprim(const TfToken& typeId, const SdfPath& id, HdDirtyBits initialBits) {
    GetRenderIndex().InsertSprim(typeId, this, id);
    GetChangeTracker().SprimInserted(id, initialBits);
}

SdfPath
HdMayaDelegateCtx::GetRPrimPath(const MDagPath& dg) {
    return _GetPrimPath(_rprimPath, dg);
}

SdfPath
HdMayaDelegateCtx::GetSPrimPath(const MDagPath& dg) {
    return _GetPrimPath(_sprimPath, dg);
}

void
HdMayaDelegateCtx::FitFrustumToRprims(GfFrustum& frustum) {
    auto getInverse = [](const GfMatrix4d& mat) {
        const double PRECISION_LIMIT = 1.0e-13;
        double det;
        auto ret = mat.GetInverse(&det, PRECISION_LIMIT);
        if (GfAbs(det) <= PRECISION_LIMIT) {
            return GfMatrix4d(1.0);
        }
        return ret;
    };
    // TODO: Cache these queries and handle dirtying.
    // TODO: Take visibility and shadow casting parameters into account.
    // This slightly differs from how you would make a calculation on a traditional frustum,
    // since we don't have a far plane. For simplicity we set the near plane to
    // 0.1 to cull anything behind the light.
    // We sum all the bounding boxes inside the open-ended frustum,
    // and use that bounding box to calculate the closest and farthest away points.

    std::array<GfPlane, 1> planes;
    GfRange1d nearFar;
    const auto& rotation = frustum.GetRotation();
    const auto direction = rotation.TransformDir(GfVec3d(0.0, 0.0, -1.0)).GetNormalized();
    const auto position = frustum.GetPosition();
    planes[0].Set(direction, position);

    auto isBoxInside = [&planes] (const GfRange3d& extent, const GfMatrix4d& worldToLocal) -> bool {
        for (const auto& plane : planes) {
            auto localPlane = plane;
            localPlane.Transform(worldToLocal);
            if (!localPlane.IntersectsPositiveHalfSpace(extent)) {
                return false;
            }
        }
        return true;
    };

    for (const auto& id : GetRenderIndex().GetRprimIds()) {
        auto* rprim = GetRenderIndex().GetRprim(id);
        if (rprim == nullptr) { continue; }
        auto* delegate = GetRenderIndex().GetSceneDelegateForRprim(id);
        if (delegate == nullptr) { continue; }
        const auto extent = rprim->GetExtent(delegate);
        const auto localToWorld = delegate->GetTransform(id);

        if (isBoxInside(extent, getInverse(localToWorld))) {
            for (size_t i = 0; i < 8u; ++i) {
                const auto corner = localToWorld.Transform(extent.GetCorner(i));
                // Project position into direction
                nearFar.ExtendBy((corner - position) * direction);
            }
        }
    }

    nearFar.SetMin(std::max(0.1, nearFar.GetMin()));
    frustum.SetNearFar(nearFar);
}

PXR_NAMESPACE_CLOSE_SCOPE
