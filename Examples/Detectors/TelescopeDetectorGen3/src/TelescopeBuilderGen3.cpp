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
#include "Acts/Surfaces/RadialBounds.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "ActsExamples/TelescopeDetectorGen3/TelescopeBlueprintGen3.hpp"
#include "ActsExamples/TelescopeDetectorGen3/TelescopeDetectorGen3Element.hpp"

#include <memory>
#include <vector>

namespace ActsExamples {

std::unique_ptr<const Acts::TrackingGeometry> buildTelescopeDetectorGen3(
    const Acts::GeometryContext& gctx,
    std::vector<std::shared_ptr<const Acts::SurfacePlacementBase>>&
        detectorStore,
    const TelescopeDetectorGen3::Config& config, const Acts::Logger& logger) {
  const auto bounds =
      std::make_shared<Acts::RadialBounds>(config.bounds[0], config.bounds[1]);

  std::vector<Acts::MutableProtoLayer> protoLayers;
  protoLayers.reserve(config.positions.size());

  for (double z : config.positions) {
    auto transform = std::make_shared<Acts::Transform3>(
        Acts::Transform3::Identity());
    transform->translation() = Acts::Vector3(0., 0., z);

    const auto identifier =
        static_cast<TelescopeDetectorGen3Element::Identifier>(
            detectorStore.size());
    auto element = std::make_shared<TelescopeDetectorGen3Element>(
        identifier, std::move(transform), bounds, config.thickness);

    std::vector<Acts::Surface*> surfaces{&element->surface()};
    protoLayers.emplace_back(gctx, surfaces);
    detectorStore.push_back(std::move(element));
  }

  return buildTelescopeBlueprintGen3(gctx, protoLayers, config, logger);
}

}  // namespace ActsExamples
