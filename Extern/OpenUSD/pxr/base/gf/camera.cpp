//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//

#include "pxr/pxr.h"
#include "pxr/base/gf/camera.h"
#include "pxr/base/gf/frustum.h"
#include "pxr/base/tf/enum.h"
#include "pxr/base/tf/registryManager.h"

PXR_NAMESPACE_OPEN_SCOPE

TF_REGISTRY_FUNCTION(TfEnum)
{
    TF_ADD_ENUM_NAME(Camera::Perspective,   "perspective");
    TF_ADD_ENUM_NAME(Camera::Orthographic,  "orthographic");
    TF_ADD_ENUM_NAME(Camera::FOVHorizontal, "FOVHorizontal");
    TF_ADD_ENUM_NAME(Camera::FOVVertical,   "FOVVertical");
}

// Horizontal and vertical aperture is in mm whereas most stuff is in cm.
const double
Camera::APERTURE_UNIT = 0.1;

// Focal length is in mm whereas most stuff is in cm.
const double
Camera::FOCAL_LENGTH_UNIT = 0.1;

// The default filmback size is based on a 35mm spherical
// projector aperture (0.825 x 0.602 inches, converted to
// mm). Note this is slightly different than SMPTE195-2000,
// which specifies 20.96mm (not 20.955mm) and 0.825"  Also
// note that 35mm spherical and anamorphic projector aperture
// widths are the same. Lastly, we use projection aperture
// instead of camera aperture since that's what we film out
// to, and for anyone who cares, 35mm still film has a different
// size, and we don't use that at all with our virtual movie
// camera.
const double
Camera::DEFAULT_HORIZONTAL_APERTURE = (
    0.825 * 2.54 / Camera::APERTURE_UNIT);
const double
Camera::DEFAULT_VERTICAL_APERTURE = (
    0.602 * 2.54 / Camera::APERTURE_UNIT);

Camera::Camera(
    const Matrix4d &transform, Camera::Projection projection,
    float horizontalAperture, float verticalAperture,
    float horizontalApertureOffset, float verticalApertureOffset,
    float focalLength,
    const Range1f &clippingRange,
    const std::vector<Vec4f>&clippingPlanes,
    float fStop,
    float focusDistance) :
    _transform(transform), _projection(projection),
    _horizontalAperture(horizontalAperture),
    _verticalAperture(verticalAperture),
    _horizontalApertureOffset(horizontalApertureOffset),
    _verticalApertureOffset(verticalApertureOffset),
    _focalLength(focalLength),
    _clippingRange(clippingRange),
    _clippingPlanes(clippingPlanes),
    _fStop(fStop),
    _focusDistance(focusDistance)
{
}

void
Camera::SetTransform(const Matrix4d &val) {
    _transform = val;
}

void
Camera::SetProjection(const Camera::Projection &val) {
    _projection = val;
}

void
Camera::SetHorizontalAperture(const float val) {
    _horizontalAperture = val;
}

void
Camera::SetVerticalAperture(const float val) {
    _verticalAperture = val;
}

void
Camera::SetHorizontalApertureOffset(const float val) {
    _horizontalApertureOffset = val;
}

void
Camera::SetVerticalApertureOffset(const float val) {
    _verticalApertureOffset = val;
}

void
Camera::SetFocalLength(const float val) {
    _focalLength = val;
}

void
Camera::SetPerspectiveFromAspectRatioAndFieldOfView(
    float aspectRatio,
    float fieldOfView,
    Camera::FOVDirection direction,
    float horizontalAperture)
{
    // Perspective
    _projection = Perspective;

    // Set the vertical and horizontal aperture to achieve the aspect ratio
    _horizontalAperture = horizontalAperture;
    _verticalAperture =   horizontalAperture /
        (aspectRatio != 0.0 ? aspectRatio : 1.0);

    // Pick the right dimension based on the direction parameter
    const float aperture =
        (direction == Camera::FOVHorizontal) ?
                           _horizontalAperture : _verticalAperture;
    // Compute tangent for field of view
    const float tanValue = tan(0.5 * DegreesToRadians(fieldOfView));
    
    if (tanValue == 0) {
        // To avoid division by zero, just set default value
        _focalLength = 50.0;
        return;
    }
    
    // Do the math for the focal length.
    _focalLength =
        aperture * Camera::APERTURE_UNIT /
        ( 2 * tanValue) / Camera::FOCAL_LENGTH_UNIT;
}

void
Camera::SetOrthographicFromAspectRatioAndSize(
    float aspectRatio, float orthographicSize, FOVDirection direction)
{
    // Orthographic
    _projection = Orthographic;

    // Not used for orthographic cameras, but set to sane values nonetheless
    _focalLength = 50.0;

    // Set horizontal and vertial aperture
    if (direction == FOVHorizontal) {
        // We are given the width, determine height by dividing by aspect ratio
        _horizontalAperture = orthographicSize / Camera::APERTURE_UNIT;
        if (aspectRatio > 0.0) {
            _verticalAperture = _horizontalAperture / aspectRatio;
        } else {
            _verticalAperture = _horizontalAperture;
        }
    } else {
        // We are given the height, determine the width by multiplying
        _verticalAperture = orthographicSize / Camera::APERTURE_UNIT;
        _horizontalAperture = _verticalAperture * aspectRatio;
    }
}

void
Camera::SetFromViewAndProjectionMatrix(
    const Matrix4d &viewMatrix, const Matrix4d &projMatrix,
    const float focalLength)
{
    _transform = viewMatrix.GetInverse();

    _focalLength = focalLength;

    if (projMatrix[2][3] < -0.5) {
        // Use !(a<b) rather than a>=b so that NaN is caught.
        if (!(fabs(projMatrix[2][3] - (-1.0)) < 1e-6)) {
            TF_WARN("Camera: Given projection matrix does not appear to be "
                    "valid perspective matrix.");
        }

        _projection = Perspective;

        const float apertureBase =
            2.0f * focalLength * (FOCAL_LENGTH_UNIT / APERTURE_UNIT);

        _horizontalAperture =
            apertureBase / projMatrix[0][0];
        _verticalAperture =
            apertureBase / projMatrix[1][1];
        _horizontalApertureOffset =
            0.5 * _horizontalAperture * projMatrix[2][0];
        _verticalApertureOffset =
            0.5 * _verticalAperture * projMatrix[2][1];
        _clippingRange = Range1f(
            projMatrix[3][2] / (projMatrix[2][2] - 1.0),
            projMatrix[3][2] / (projMatrix[2][2] + 1.0));
    } else {
        if (!(fabs(projMatrix[2][3]) < 1e-6)) {
            TF_WARN("Camera: Given projection matrix does not appear to be "
                    "valid orthographic matrix.");
        }

        _projection = Orthographic;
        _horizontalAperture =
            (2.0 / APERTURE_UNIT) / projMatrix[0][0];
        _verticalAperture =
            (2.0 / APERTURE_UNIT) / projMatrix[1][1];
        _horizontalApertureOffset =
            -0.5 * (_horizontalAperture) * projMatrix[3][0];
        _verticalApertureOffset =
            -0.5 * (_verticalAperture) * projMatrix[3][1];
        const double nearMinusFarHalf =
            1.0 / projMatrix[2][2];
        const double nearPlusFarHalf =
            nearMinusFarHalf * projMatrix[3][2];
        _clippingRange = Range1f(
            nearPlusFarHalf + nearMinusFarHalf,
            nearPlusFarHalf - nearMinusFarHalf);
    }
}

void
Camera::SetClippingRange(const Range1f &val) {
    _clippingRange = val;
}

void
Camera::SetClippingPlanes(const std::vector<Vec4f> &val) {
    _clippingPlanes = val;
}

void
Camera::SetFStop(const float val) {
    _fStop = val;
}

void
Camera::SetFocusDistance(const float val) {
    _focusDistance = val;
}

Matrix4d
Camera::GetTransform() const {
    return _transform;
}

Camera::Projection
Camera::GetProjection() const {
    return _projection;
}

float
Camera::GetHorizontalAperture() const {
    return _horizontalAperture;
}

float
Camera::GetVerticalAperture() const {
    return _verticalAperture;
}

float
Camera::GetHorizontalApertureOffset() const {
    return _horizontalApertureOffset;
}

float
Camera::GetVerticalApertureOffset() const {
    return _verticalApertureOffset;
}

float
Camera::GetAspectRatio() const {
    return (_verticalAperture == 0.0)
               ? 0.0
               : _horizontalAperture / _verticalAperture;
}

float
Camera::GetFocalLength() const {
    return _focalLength;
}

float
Camera::GetFieldOfView(FOVDirection direction) const {
    // Pick the right aperture based on direction
    const float aperture = 
        (direction == FOVHorizontal) ? _horizontalAperture
                                     : _verticalAperture;
    
    // Do the math
    const float fovRAD = 2 * atan(
        (aperture * Camera::APERTURE_UNIT) /
        (2 * _focalLength * Camera::FOCAL_LENGTH_UNIT));

    return RadiansToDegrees(fovRAD);
}

Range1f
Camera::GetClippingRange() const {
    return _clippingRange;
}

const std::vector<Vec4f> &
Camera::GetClippingPlanes() const {
    return _clippingPlanes;
}

Frustum
Camera::GetFrustum() const {

    const Vec2d max(_horizontalAperture / 2,
                      _verticalAperture / 2);
    Range2d window(-max, max);

    // Apply the aperture offset
    const Vec2d offsetVec(
        _horizontalApertureOffset, _verticalApertureOffset);
    window += Range2d(offsetVec, offsetVec);

    // Up to now, all computations were done in mm, convert to cm.
    window *= Camera::APERTURE_UNIT;

    if (_projection != Orthographic && _focalLength != 0) {
        window /= _focalLength * Camera::FOCAL_LENGTH_UNIT;
    }

    const Range1d clippingRange(_clippingRange.GetMin(),
                                  _clippingRange.GetMax());
    
    const Frustum::ProjectionType projection = _projection == Orthographic
        ? Frustum::Orthographic
        : Frustum::Perspective;

    return Frustum(_transform, window, clippingRange, projection);
}

float
Camera::GetFStop() const {
    return _fStop;
}

float
Camera::GetFocusDistance() const {
    return _focusDistance;
}

bool
Camera::operator==(const Camera& other) const
{
    return
        _transform == other._transform && 
        _projection == other._projection && 
        _horizontalAperture == other._horizontalAperture && 
        _verticalAperture == other._verticalAperture && 
        _horizontalApertureOffset == other._horizontalApertureOffset && 
        _verticalApertureOffset == other._verticalApertureOffset && 
        _focalLength == other._focalLength && 
        _clippingRange == other._clippingRange && 
        _clippingPlanes == other._clippingPlanes &&
        _fStop == other._fStop &&
        _focusDistance == other._focusDistance;
}

bool
Camera::operator!=(const Camera& other) const
{
    return !(*this == other);
}

PXR_NAMESPACE_CLOSE_SCOPE
