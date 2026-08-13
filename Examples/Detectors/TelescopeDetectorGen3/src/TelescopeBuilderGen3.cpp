// This file is part of the ACTS project.
//
// Copyright (C) 2016 CERN for the benefit of the ACTS project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "ActsExamples/TelescopeDetectorGen3/TelescopeBuilderGen3.hpp"

#include "Acts/Definitions/Algebra.hpp"
#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/Geometry/ProtoLayer.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "ActsExamples/TelescopeDetectorGen3/TelescopeBlueprintGen3.hpp"
#include "ActsExamples/TelescopeDetectorGen3/TelescopeDetectorGen3Element.hpp"

#include <array>
#include <cstddef>
#include <memory>
#include <vector>

namespace ActsExamples {
namespace {

/// Return the rotation that maps the surface-local +Z direction to the
/// configured global telescope axis.
Acts::RotationMatrix3 telescopeRotation(int axis) {
  Acts::RotationMatrix3 rotation = Acts::RotationMatrix3::Identity();

  if (axis == 0) {
    // local X -> global -Z, local Y -> global Y, local Z -> global X
    rotation.col(0) = Acts::Vector3(0., 0., -1.);
    rotation.col(1) = Acts::Vector3(0., 1., 0.);
    rotation.col(2) = Acts::Vector3(1., 0., 0.);
  } else if (axis == 1) {
    // local X -> global X, local Y -> global -Z, local Z -> global Y
    rotation.col(0) = Acts::Vector3(1., 0., 0.);
    rotation.col(1) = Acts::Vector3(0., 0., -1.);
    rotation.col(2) = Acts::Vector3(0., 1., 0.);
  }

  // axis == 2 uses the identity rotation: local Z -> global Z.
  return rotation;
}

}  // namespace

std::unique_ptr<const Acts::TrackingGeometry> buildTelescopeDetectorGen3(
    const Acts::GeometryContext& gctx,
    std::vector<std::shared_ptr<const Acts::SurfacePlacementBase>>&
        detectorStore,
    const TelescopeDetectorGen3::Config& config, const Acts::Logger& logger) {
  const std::array<double, 2>& selectedBounds =
      config.surfaceType == TelescopeDetectorGen3::SurfaceType::Disc
          ? config.discBounds
          : config.planeBounds;

  const Acts::RotationMatrix3 rotation = telescopeRotation(config.axis);

  std::vector<Acts::MutableProtoLayer> protoLayers;
  protoLayers.reserve(config.positions.size());

  for (std::size_t index = 0; index < config.positions.size(); ++index) {
    // The Blueprint layers use a common local coordinate system whose local Z
    // axis is the telescope axis. So, the translation is first expressed
    // along local Z and then rotated onto the selected global axis.
    const Acts::Transform3 layerTransform(
        rotation * Acts::Translation3(0., 0., config.positions[index]));

    // Stereo is an in-plane rotation around the local axis of the surface (by default Z ).
    // layerTransform already maps local Z onto the selected telescope axis,
    // so this is equivalent to a rotation around that global axis.
    Acts::Transform3 surfaceTransform = layerTransform;
    surfaceTransform *= Acts::AngleAxis3(config.stereos[index],
                                         Acts::Vector3::UnitZ());

    const auto identifier =
        static_cast<TelescopeDetectorGen3Element::Identifier>(
            detectorStore.size());
    auto element = std::make_shared<TelescopeDetectorGen3Element>(
        identifier, std::make_shared<Acts::Transform3>(surfaceTransform),
        config.surfaceType, selectedBounds, config.thickness);

    std::vector<Acts::Surface*> surfaces{&element->surface()};

    // Removes the global layer position and the 
    // telescope-axis rotation, while retaining the
    // in-plane stereo rotation in the measured X/Y extent.
    protoLayers.emplace_back(gctx, surfaces, layerTransform.inverse());
    auto& protoLayer = protoLayers.back();

    // LayerBlueprintNode uses the X/Y/Z extent midpoints directly as the
    // GLOBAL translation of the wrapping volume, while the extent intervals
    // determine the LOCAL volume half-lengths. Recenter only the Cartesian
    // ranges on the desired global layer position and preserve their measured
    // local intervals. Cylindrical R information is intentionally untouched.
    using enum Acts::AxisDirection;
    const Acts::Vector3 globalCenter = layerTransform.translation();

    const double halfX = 0.5 * protoLayer.extent.interval(AxisX);
    const double halfY = 0.5 * protoLayer.extent.interval(AxisY);
    const double halfZ = 0.5 * protoLayer.extent.interval(AxisZ);

    protoLayer.extent.set(AxisX, globalCenter.x() - halfX,
                          globalCenter.x() + halfX);
    protoLayer.extent.set(AxisY, globalCenter.y() - halfY,
                          globalCenter.y() + halfY);
    protoLayer.extent.set(AxisZ, globalCenter.z() - halfZ,
                          globalCenter.z() + halfZ);

    // Store only the global-to-local ROTATION as ProtoLayer metadata. The
    // Blueprint builder inverts this to orient every layer volume, while its
    // translation is supplied by the recentered extent above.
    Acts::Transform3 localFrame = Acts::Transform3::Identity();
    localFrame.linear() = rotation.transpose();
    protoLayer.transform = localFrame;

    detectorStore.push_back(std::move(element));
  }

  return buildTelescopeBlueprintGen3(gctx, protoLayers, config, logger);
}

}  // namespace ActsExamples
