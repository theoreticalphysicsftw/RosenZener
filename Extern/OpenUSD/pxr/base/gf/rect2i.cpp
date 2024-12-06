//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//

#include "pxr/pxr.h"
#include "pxr/base/gf/rect2i.h"

#include "pxr/base/gf/ostreamHelpers.h"

#include "pxr/base/tf/type.h"

#include <ostream>

PXR_NAMESPACE_OPEN_SCOPE

// CODE_COVERAGE_OFF_GCOV_BUG
TF_REGISTRY_FUNCTION(TfType) {
    TfType::Define<Rect2i>();
}
// CODE_COVERAGE_ON_GCOV_BUG

Rect2i
Rect2i::GetNormalized() const
{
    Vec2i min, max;

    if (_max[0] < _min[0]) {
        min[0] = _max[0];
        max[0] = _min[0];
    }
    else {
        min[0] = _min[0];
        max[0] = _max[0];
    }

    if (_max[1] < _min[1]) {
        min[1] = _max[1];
        max[1] = _min[1];
    }
    else {
        min[1] = _min[1];
        max[1] = _max[1];
    }

    return Rect2i(min, max);
}

std::ostream &
operator<<(std::ostream& out, const Rect2i& r)
{
    return out << '[' << _OstreamHelperP(r.GetMin()) << ":" 
        << _OstreamHelperP(r.GetMax()) << ']';
}

PXR_NAMESPACE_CLOSE_SCOPE
