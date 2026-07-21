// This file is part of the ACTS project.
//
// Copyright (C) 2016 CERN for the benefit of the ACTS project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "ActsExamples/TelescopeDetectorGen3/TelescopeBlueprintGen3.hpp"

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
#include "Acts/Utilities/AxisDefinitions.hpp"
#include "Acts/Utilities/Logger.hpp"

#include <fstream>
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

}  // namespace

std::unique_ptr<const Acts::TrackingGeometry> buildTelescopeBlueprintGen3(
    const Acts::GeometryContext& gctx,
    const std::vector<Acts::MutableProtoLayer>& protoLayers,
    const TelescopeDetectorGen3::Config& config, const Acts::Logger& logger) {
  using enum Acts::AxisDirection;
  using namespace Acts::Experimental;
  using AttachmentStrategy = Acts::VolumeAttachmentStrategy;
  using ResizeStrategy = Acts::VolumeResizeStrategy;

  Blueprint::Config blueprintConfig;
  blueprintConfig.envelope = Acts::ExtentEnvelope{{
      .z = {config.layerEnvelope, config.layerEnvelope},
      .r = {config.bounds[0], config.layerEnvelope},
  }};
  Blueprint root{blueprintConfig};

  auto& telescope = root.addCylinderContainer("TelescopeGen3", AxisZ);
  telescope.setAttachmentStrategy(AttachmentStrategy::Gap);
  telescope.setResizeStrategy(ResizeStrategy::Gap);

  for (std::size_t index = 0; index < protoLayers.size(); ++index) {
    const std::string layerName = "TelescopeGen3_L" + std::to_string(index);
    telescope.addLayer(layerName, [&](auto& layer) {
      layer.setLayerType(LayerBlueprintNode::LayerType::Disc);
      layer.setProtoLayer(protoLayers.at(index));
      layer.setEnvelope(Acts::ExtentEnvelope{{
          .z = {config.layerEnvelope, config.layerEnvelope},
          .r = {0., 0.},
      }});
      layer.setNavigationPolicyFactory(discNavigationFactory());
    });
  }

  if (config.graphvizFile.has_value()) {
    std::ofstream graphviz{*config.graphvizFile};
    if (!graphviz) {
      throw std::runtime_error("Could not open TelescopeDetectorGen3 Graphviz file: " +
                               config.graphvizFile->string());
    }
    root.graphviz(graphviz);
  }

  BlueprintOptions options;
  return root.construct(options, gctx, logger);
}

}  // namespace ActsExamples
