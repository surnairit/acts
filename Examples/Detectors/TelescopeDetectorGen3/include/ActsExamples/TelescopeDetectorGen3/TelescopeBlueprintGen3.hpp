// This file is part of the ACTS project.
//
// Copyright (C) 2016 CERN for the benefit of the ACTS project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "Acts/Geometry/ProtoLayer.hpp"
#include "ActsExamples/TelescopeDetectorGen3/TelescopeDetectorGen3.hpp"

#include <memory>
#include <vector>

namespace Acts {
class GeometryContext;
class Logger;
class TrackingGeometry;
}

namespace ActsExamples {

std::unique_ptr<const Acts::TrackingGeometry> buildTelescopeBlueprintGen3(
    const Acts::GeometryContext& gctx,
    const std::vector<Acts::MutableProtoLayer>& protoLayers,
    const TelescopeDetectorGen3::Config& config, const Acts::Logger& logger);

}  // namespace ActsExamples
