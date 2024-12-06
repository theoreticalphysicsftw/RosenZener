// Modified from the original OpenUSD file.
// Copyright 2024 Mihail Mladenov

//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//

#include "pxr/pxr.h"
#include "pxr/base/gf/interval.h"
#include "pxr/base/gf/ostreamHelpers.h"
#include "pxr/base/tf/type.h"

#include <ostream>

PXR_NAMESPACE_OPEN_SCOPE

// CODE_COVERAGE_OFF_GCOV_BUG  you know the drill.
TF_REGISTRY_FUNCTION(TfType) {
    TfType::Define<Interval>();
}
// CODE_COVERAGE_ON_GCOV_BUG

std::ostream &
operator<<(std::ostream &out, const Interval &i)
{
    out << (i.IsMinClosed() ? "[" : "(");
    out << _OstreamHelperP(i.GetMin()) << ", ";
    out << _OstreamHelperP(i.GetMax());
    out << (i.IsMaxClosed() ? "]" : ")");
    return out;
}

PXR_NAMESPACE_CLOSE_SCOPE
