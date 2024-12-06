// Modified from the original OpenUSD file.
// Copyright 2024 Mihail Mladenov

// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#ifndef PXR_BASE_GF_TRAITS_H
#define PXR_BASE_GF_TRAITS_H

#include "pxr/pxr.h"

#include <type_traits>

PXR_NAMESPACE_OPEN_SCOPE

/// A metafunction with a static const bool member 'value' that is true for
/// Vec types, like Vec2i, Vec4d, etc and false for all other types.
template <class T>
struct IsVec { static const bool value = false; };

/// A metafunction with a static const bool member 'value' that is true for
/// Matrix types, like Matrix3d, Matrix4f, etc and false for all other
/// types.
template <class T>
struct IsMatrix { static const bool value = false; };

/// A metafunction with a static const bool member 'value' that is true for
/// Quat types and false for all other types.
template <class T>
struct IsQuat { static const bool value = false; };

/// A metafunction with a static const bool member 'value' that is true for
/// DualQuat types and false for all other types.
template <class T>
struct IsDualQuat { static const bool value = false; };

/// A metafunction with a static const bool member 'value' that is true for
/// Range types and false for all other types.
template <class T>
struct IsRange { static const bool value = false; };

/// A metafunction which is equivalent to std::is_floating_point but
/// allows for additional specialization for types like Half
template <class T>
struct IsFloatingPoint : public std::is_floating_point<T>{};

/// A metafunction which is equivalent to std::arithmetic but
/// also includes any specializations from IsFloatingPoint (like Half)
template <class T>
struct IsArithmetic : public std::integral_constant<
    bool, IsFloatingPoint<T>::value || std::is_arithmetic<T>::value>{};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // PXR_BASE_GF_TRAITS_H
