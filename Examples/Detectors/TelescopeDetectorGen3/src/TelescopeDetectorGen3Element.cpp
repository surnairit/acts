// This file is part of the ACTS project.
//
// Copyright (C) 2016 CERN for the benefit of the ACTS project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "ActsExamples/TelescopeDetectorGen3/TelescopeDetectorGen3Element.hpp"

#include "Acts/Surfaces/DiscSurface.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"
#include "Acts/Surfaces/RadialBounds.hpp"
#include "Acts/Surfaces/RectangleBounds.hpp"

#include <stdexcept>
#include <utility>

namespace ActsExamples {

TelescopeDetectorGen3Element::TelescopeDetectorGen3Element(
    Identifier identifier, std::shared_ptr<Acts::Transform3> transform,
    TelescopeDetectorGen3::SurfaceType surfaceType,
    const std::array<double, 2>& bounds, double thickness)
    : m_identifier(identifier),
      m_transform(std::move(transform)),
      m_thickness(thickness) {
  switch (surfaceType) {
    case TelescopeDetectorGen3::SurfaceType::Disc: {
      auto radialBounds =
          std::make_shared<Acts::RadialBounds>(bounds[0], bounds[1]);
      m_surface =
          Acts::Surface::makeShared<Acts::DiscSurface>(radialBounds, *this);
      m_bounds = std::move(radialBounds);
      break;
    }
    case TelescopeDetectorGen3::SurfaceType::Plane: {
      auto rectangleBounds =
          std::make_shared<Acts::RectangleBounds>(bounds[0], bounds[1]);
      m_surface =
          Acts::Surface::makeShared<Acts::PlaneSurface>(rectangleBounds, *this);
      m_bounds = std::move(rectangleBounds);
      break;
    }
    default:
      throw std::invalid_argument(
          "TelescopeDetectorGen3Element received an unknown surface type");
  }

  m_surface->assignThickness(thickness);
}

}  // namespace ActsExamples
