// This file is part of the ACTS project.
//
// Copyright (C) 2016 CERN for the benefit of the ACTS project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "ActsExamples/TelescopeDetectorGen3/TelescopeDetectorGen3.hpp"

#include <memory>
#include <vector>

namespace Acts {
class GeometryContext;
class SurfacePlacementBase;
class TrackingGeometry;
class Logger;
}

namespace ActsExamples {

std::unique_ptr<const Acts::TrackingGeometry> buildTelescopeDetectorGen3(
    const Acts::GeometryContext& gctx,
    std::vector<std::shared_ptr<const Acts::SurfacePlacementBase>>&
        detectorStore,
    const TelescopeDetectorGen3::Config& config, const Acts::Logger& logger);

}  // namespace ActsExamples
