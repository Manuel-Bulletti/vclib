// VCLib - Visual Computing Library
// Copyright (C) 2026 Visual Computing Lab, ISTI - CNR.
//
// This Source Code Form is subject to the terms of the Mozilla Public License,
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at https://mozilla.org/MPL/2.0/.

#ifndef VCL_SPACE_CORE_GAUSSIAN3D_H
#define VCL_SPACE_CORE_GAUSSIAN3D_H

#include <vclib/space/core/point.h>
#include <vclib/space/core/quaternion.h>
#include <vclib/base/serialize.h>

#include <array>
#include <cmath>

namespace vcl {

/**
 * @brief Gaussian3D represents a 3D Gaussian described by a decomposed
 * covariance (log-scale + rotation), an opacity and Spherical Harmonics
 * coefficients (degrees 0..3).
 *
 * The class stores the scale in log-space (log-scale) to allow unconstrained
 * optimization. To obtain the actual scale (used at rendering / evaluation)
 * call scale(), which returns the component-wise exponential of the stored
 * log-scale.
 *
 * The stored Spherical Harmonics coefficients are arranged in a flat array
 * containing all coefficients for degrees l = 0..3 (total 1+3+5+7 = 16
 * coefficients). The ordering convention is left to the user but is usually
 * the "band-major" ordering (l increasing, m from -l to +l).
 *
 * @tparam Scalar scalar type (float/double)
 */
template<typename Scalar>
class Gaussian3D
{
public:
    using ScalarType = Scalar;
    using PointType  = Point3<Scalar>;
    using QuatType   = Quaternion<Scalar>;

    static constexpr uint SHCoeffCount = 16; // l = 0..3 -> 1+3+5+7 = 16

private:
    // stored as log-scale; use scale() to retrieve exp(logScale_)
    PointType                mLogScale{0, 0, 0};
    QuatType                 mRotation{1, 0, 0, 0}; // identity rotation
    Scalar                   mOpacity = Scalar(1.0);
    std::array<Scalar, SHCoeffCount> mSHCoeffs{}; // initialized to zeros

public:
    Gaussian3D() = default;

    /* Accessors */
    const PointType& logScale() const { return mLogScale; }
    PointType&       logScale() { return mLogScale; }

    /**
     * @brief Returns the actual scale (exp of stored log-scale), component
     * wise. This is the value that should be used in rendering / evaluation.
     */
    PointType scale() const
    {
        PointType s;
        for (uint i = 0; i < 3; ++i) {
            s[i] = std::exp(mLogScale[i]);
        }
        return s;
    }

    const QuatType& rotation() const { return mRotation; }
    QuatType&       rotation() { return mRotation; }

    const Scalar& opacity() const { return mOpacity; }
    Scalar&       opacity() { return mOpacity; }

    const std::array<Scalar, SHCoeffCount>& shCoeffs() const { return mSHCoeffs; }
    std::array<Scalar, SHCoeffCount>&       shCoeffs() { return mSHCoeffs; }

    Scalar shCoeff(uint idx) const
    {
        assert(idx < SHCoeffCount);
        return mSHCoeffs[idx];
    }

    void setSHCoeff(uint idx, Scalar v)
    {
        assert(idx < SHCoeffCount);
        mSHCoeffs[idx] = v;
    }

    /* Serialization */
    void serialize(std::ostream& os) const
    {
        // Point and Quaternion provide their own serialize
        mLogScale.serialize(os);
        mRotation.serialize(os);
        vcl::serialize(os, mOpacity);
        for (uint i = 0; i < SHCoeffCount; ++i)
            vcl::serialize(os, mSHCoeffs[i]);
    }

    void deserialize(std::istream& is)
    {
        mLogScale.deserialize(is);
        mRotation.deserialize(is);
        vcl::deserialize(is, mOpacity);
        for (uint i = 0; i < SHCoeffCount; ++i)
            vcl::deserialize(is, mSHCoeffs[i]);
    }
};

/* Aliases */
using Gaussian3Df = Gaussian3D<float>;
using Gaussian3Dd = Gaussian3D<double>;

} // namespace vcl

#endif // VCL_SPACE_CORE_GAUSSIAN3D_H
