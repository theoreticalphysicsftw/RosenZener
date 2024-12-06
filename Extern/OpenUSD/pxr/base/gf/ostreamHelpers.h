//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#ifndef PXR_BASE_GF_OSTREAM_HELPERS_H
#define PXR_BASE_GF_OSTREAM_HELPERS_H

/// \file gf/ostreamHelpers.h
/// \ingroup group_gf_DebuggingOutput
///
/// Helpers for  stream operators.
///
/// These functions are useful to help with writing stream operators for
///  types.  Please do not include this file in any header.

#include "pxr/pxr.h"
#include "pxr/base/tf/stringUtils.h"

PXR_NAMESPACE_OPEN_SCOPE

// Make the template class general so that we can use the same class
// (_OstreamHelperP) on all variables and not worry about making a mistake
template <class T>
T _OstreamHelperP(T v) { return v; }

inline TfStreamFloat _OstreamHelperP(float v) { 
    return TfStreamFloat(v); 
}
inline TfStreamDouble _OstreamHelperP(double v) { 
    return TfStreamDouble(v); 
}

PXR_NAMESPACE_CLOSE_SCOPE

#endif // GF_OSTREAM_HELPERS 
