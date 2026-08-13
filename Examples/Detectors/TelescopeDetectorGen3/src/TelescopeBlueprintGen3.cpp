// This file is part of the ACTS project.
//
// Copyright (C) 2016 CERN for the benefit of the ACTS project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "ActsExamples/TelescopeDetectorGen3/TelescopeBlueprintGen3.hpp"

#include "Acts/Definitions/Algebra.hpp"
#include "Acts/Geometry/Blueprint.hpp"
#include "Acts/Geometry/BlueprintNode.hpp"
#include "Acts/Geometry/BlueprintOptions.hpp"
#include "Acts/Geometry/ContainerBlueprintNode.hpp"
#include "Acts/Geometry/Extent.hpp"
#include "Acts/Geometry/LayerBlueprintNode.hpp"
#include "Acts/Geometry/NavigationPolicyFactory.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Geometry/VolumeAttachmentStrategy.hpp"
#include "Acts/Geometry/VolumeResizeStrategy.hpp"
#include "Acts/Navigation/CylinderNavigationPolicy.hpp"
#include "Acts/Navigation/SurfaceArrayNavigationPolicy.hpp"
#include "Acts/Navigation/TryAllNavigationPolicy.hpp"
#include "Acts/Utilities/AxisDefinitions.hpp"
#include "Acts/Utilities/Logger.hpp"

#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

namespace ActsExamples {
namespace {

std::unique_ptr<Acts::NavigationPolicyFactory> discNavigationFactory() {
  using SurfaceArrayPolicy = Acts::SurfaceArrayNavigationPolicy;

  return Acts::NavigationPolicyFactory{}
      .add<Acts::CylinderNavigationPolicy>()
      .add<SurfaceArrayPolicy>(SurfaceArrayPolicy::Config{
          .layerType = SurfaceArrayPolicy::LayerType::Disc,
          .bins = {1u, 1u},
      })
      .asUniquePtr();
}

std::unique_ptr<Acts::NavigationPolicyFactory> planeNavigationFactory() {
  return Acts::NavigationPolicyFactory{}
      .add<Acts::TryAllNavigationPolicy>(Acts::TryAllNavigationPolicy::Config{
          .portals = true,
          .sensitives = true,
          .passives = true,
      })
      .asUniquePtr();
}

}  // namespace

std::unique_ptr<const Acts::TrackingGeometry> buildTelescopeBlueprintGen3(
    const Acts::GeometryContext& gctx,
    const std::vector<Acts::MutableProtoLayer>& protoLayers,
    const TelescopeDetectorGen3::Config& config, const Acts::Logger& logger) {
  using enum Acts::AxisDirection;
  using namespace Acts::Experimental;
  using AttachmentStrategy = Acts::VolumeAttachmentStrategy;
  using ResizeStrategy = Acts::VolumeResizeStrategy;

  const bool isDisc =
      config.surfaceType == TelescopeDetectorGen3::SurfaceType::Disc;

  // All child volumes are expressed in the telescope-local frame. Its local Z
  // direction is mapped onto the selected global axis by the layer transforms
  // stored in the proto-layers. Consequently the envelopes remain local Z/R or
  // local X/Y/Z envelopes, independent of config.axis.
  Blueprint::Config blueprintConfig;
  if (isDisc) {
    blueprintConfig.envelope = Acts::ExtentEnvelope{{
        .z = {config.layerEnvelope, config.layerEnvelope},
        .r = {config.discBounds[0], config.layerEnvelope},
    }};
  } else {
    blueprintConfig.envelope = Acts::ExtentEnvelope{{
        .x = {config.layerEnvelope, config.layerEnvelope},
        .y = {config.layerEnvelope, config.layerEnvelope},
        .z = {config.layerEnvelope, config.layerEnvelope},
    }};
  }

  Blueprint root{blueprintConfig};

  const auto addLayers = [&](auto& telescope) {
    telescope.setAttachmentStrategy(AttachmentStrategy::Gap);
    telescope.setResizeStrategy(ResizeStrategy::Gap);

    for (std::size_t index = 0; index < protoLayers.size(); ++index) {
      const std::string layerName =
          "TelescopeGen3_L" + std::to_string(index);
      const auto& protoLayer = protoLayers.at(index);

      // TelescopeBuilderGen3 stores the global-to-local rotation in the
      // proto-layer metadata. Its Cartesian extent midpoints contain the
      // desired global layer centre.
      const Acts::Transform3 layerRotation = protoLayer.transform.inverse();

      telescope.addLayer(layerName, [&](auto& layer) {
        layer.setProtoLayer(protoLayer);
        layer.setTransform(layerRotation);

        // Keep centre-of-gravity placement enabled. LayerBlueprintNode uses
        // these extent midpoints as the global translation of the wrapping
        // volume. Disabling this would reset every volume translation to zero
        // and make all layers overlap in the container's local AxisZ.
        layer.setUseCenterOfGravity(true, true, true);

        if (isDisc) {
          layer.setLayerType(LayerBlueprintNode::LayerType::Disc);
          layer.setEnvelope(Acts::ExtentEnvelope{{
              .z = {config.layerEnvelope, config.layerEnvelope},
              .r = {0., 0.},
          }});
          layer.setNavigationPolicyFactory(discNavigationFactory());
        } else {
          layer.setLayerType(LayerBlueprintNode::LayerType::Plane);
          layer.setEnvelope(Acts::ExtentEnvelope{{
              .x = {0., 0.},
              .y = {0., 0.},
              .z = {config.layerEnvelope, config.layerEnvelope},
          }});
          layer.setNavigationPolicyFactory(planeNavigationFactory());
        }
      });
    }
  };

  // The selected global axis is encoded in the common layer transform, not in
  // the stack direction. In the common telescope coordinate system all layers
  // are stacked along local Z. This is required for CylinderVolumeStack, which
  // supports Z/R stacks rather than global X/Y cylinder stacks, and it also
  // gives CuboidVolumeStack one common rotated coordinate system.
  if (isDisc) {
    auto& telescope = root.addCylinderContainer("TelescopeGen3", AxisZ);
    addLayers(telescope);
  } else {
    auto& telescope = root.addCuboidContainer("TelescopeGen3", AxisZ);
    addLayers(telescope);
  }

  if (config.graphvizFile.has_value()) {
    std::ofstream graphviz{*config.graphvizFile};
    if (!graphviz) {
      throw std::runtime_error(
          "Could not open TelescopeDetectorGen3 Graphviz file: " +
          config.graphvizFile->string());
    }
    root.graphviz(graphviz);
  }

  BlueprintOptions options;
  return root.construct(options, gctx, logger);
}

}  // namespace ActsExamples