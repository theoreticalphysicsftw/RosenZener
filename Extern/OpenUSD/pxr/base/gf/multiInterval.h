//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#ifndef PXR_BASE_GF_MULTI_INTERVAL_H
#define PXR_BASE_GF_MULTI_INTERVAL_H

/// \file gf/multiInterval.h
/// \ingroup group_gf_BasicMath

#include "pxr/pxr.h"
#include "pxr/base/gf/interval.h"
#include "pxr/base/gf/api.h"

#include <iosfwd>
#include <set>
#include <vector>

PXR_NAMESPACE_OPEN_SCOPE

/// \class MultiInterval
/// \ingroup group_gf_BasicMath
///
/// MultiInterval represents a subset of the real number line as an
/// ordered set of non-intersecting Intervals.
///
class MultiInterval
{
public:
    typedef std::set<Interval> Set;
    typedef Set::const_iterator const_iterator;
    typedef Set::const_iterator iterator;

    /// \name Constructors
    /// @{
    /// Constructs an empty multi-interval.
    MultiInterval() = default;
    /// Constructs an multi-interval with the single given interval.
    GF_API explicit MultiInterval(const Interval &i);
    /// Constructs an multi-interval containing the given input intervals.
    GF_API explicit MultiInterval(const std::vector<Interval> &intervals);
    /// @}

    GF_API bool operator==(const MultiInterval &that) const { return _set == that._set; }
    GF_API bool operator!=(const MultiInterval &that) const { return !(*this == that); }
    GF_API bool operator<(const MultiInterval &that) const { return _set < that._set; }
    GF_API bool operator>=(const MultiInterval &that) const { return !(*this < that); }
    GF_API bool operator>(const MultiInterval &that) const { return (that < *this); }
    GF_API bool operator<=(const MultiInterval &that) const { return !(that < *this); }
    

    /// Hash value.
    /// Just a basic hash function, not particularly high quality.
    GF_API size_t Hash() const;

    friend inline size_t hash_value(const MultiInterval &mi) {
        return mi.Hash();
    }

    /// \name Accessors
    /// @{

    /// Returns true if the multi-interval is empty.
    GF_API bool IsEmpty() const { return _set.empty(); }

    /// Returns the number of intervals in the set.
    GF_API size_t GetSize() const { return _set.size(); }

    /// Returns an interval bounding the entire multi-interval.
    /// Returns an empty interval if the multi-interval is empty.
    GF_API Interval GetBounds() const;

    /// Returns true if the multi-interval contains the given value.
    GF_API bool Contains(double d) const;

    /// Returns true if the multi-interval contains the given interval.
    GF_API bool Contains(const Interval & i) const;

    /// Returns true if the multi-interval contains all the intervals in the 
    /// given multi-interval.
    GF_API bool Contains(const MultiInterval & s) const;

    /// @}

    /// \name Mutation
    /// @{

    /// Clear the multi-interval.
    GF_API void Clear() { _set.clear(); }

    /// Add the given interval to the multi-interval.
    GF_API void Add( const Interval & i );
    /// Add the given multi-interval to the multi-interval.
    /// Sets this object to the union of the two sets.
    GF_API void Add( const MultiInterval &s );

    /// Uses the given interval to extend the multi-interval in
    /// the interval arithmetic sense.
    GF_API void ArithmeticAdd( const Interval &i );

    /// Remove the given interval from this multi-interval.
    GF_API void Remove( const Interval & i );
    /// Remove the given multi-interval from this multi-interval.
    GF_API void Remove( const MultiInterval &s );

    GF_API void Intersect( const Interval & i );
    GF_API void Intersect( const MultiInterval &s );

    /// Return the complement of this set.
    GF_API MultiInterval GetComplement() const;

    /// @}

    /// \name Iteration
    /// Only const iterators are returned.  To maintain the invariants of
    /// the multi-interval, changes must be made via the public mutation API.
    /// @{

    GF_API const_iterator begin() const { return _set.begin(); }
    GF_API const_iterator end() const { return _set.end(); }

    /// Returns an iterator identifying the first (lowest) interval whose
    /// minimum value is >= x.  If no such interval exists, returns end().
    GF_API const_iterator lower_bound( double x ) const;

    /// Returns an iterator identifying the first (lowest) interval whose
    /// minimum value is > x.  If no such interval exists, returns end().
    GF_API const_iterator upper_bound( double x ) const;

    /// Returns an iterator identifying the first (lowest) interval whose
    /// minimum value is > x.  If no such interval exists, returns end().
    GF_API const_iterator GetNextNonContainingInterval( double x ) const;

    /// Returns an iterator identifying the last (highest) interval whose
    /// maximum value is < x.  If no such interval exists, returns end().
    GF_API const_iterator GetPriorNonContainingInterval( double x ) const;

    /// Returns an iterator identifying the interval that contains x.  If
    /// no interval contains x, then it returns end()
    GF_API const_iterator GetContainingInterval( double x ) const;

    /// @}

    /// Returns the full interval (-inf, inf).
    static MultiInterval GetFullInterval() {
        return MultiInterval(Interval::GetFullInterval());
    }

    /// Swap two multi-intervals
    void swap(MultiInterval &other) { _set.swap(other._set); }

private:
    void _AssertInvariants() const;

    Set _set;
};

/// Output a MultiInterval
/// \ingroup group_gf_DebuggingOutput
GF_API std::ostream & operator<<(std::ostream &out, const MultiInterval &s);

PXR_NAMESPACE_CLOSE_SCOPE

#endif // PXR_BASE_GF_MULTI_INTERVAL_H
