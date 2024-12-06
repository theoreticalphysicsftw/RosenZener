// Modified from the original OpenUSD file.
// Copyright 2024 Mihail Mladenov

//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
////////////////////////////////////////////////////////////////////////
// This file is generated by a script.  Do not edit directly.  Edit the
// range.template.cpp file to make changes.

#include "pxr/base/gf/range1f.h"
#include "pxr/base/gf/range1d.h"

#include "pxr/base/gf/math.h"
#include "pxr/base/gf/ostreamHelpers.h"
#include "pxr/base/tf/type.h"

#include <cfloat>
#include <ostream>

PXR_NAMESPACE_OPEN_SCOPE

TF_REGISTRY_FUNCTION(TfType) {
    TfType::Define<Range1f>();
}

std::ostream& 
operator<<(std::ostream &out, Range1f const &r)
{
    return out << '[' 
               << _OstreamHelperP(r.GetMin()) << "..." 
               << _OstreamHelperP(r.GetMax())
               << ']';
}

Range1f::Range1f(class Range1d const &other)
    : _min( float(other.GetMin()))
    , _max( float(other.GetMax()))
{
}

double
Range1f::GetDistanceSquared(float p) const
{
    double dist = 0.0;

    if (p < _min) {
	// p is left of box
	dist += Sqr(_min - p);
    }
    else if (p > _max) {
	// p is right of box
	dist += Sqr(p - _max);
    }

    return dist;
}

PXR_NAMESPACE_CLOSE_SCOPE
