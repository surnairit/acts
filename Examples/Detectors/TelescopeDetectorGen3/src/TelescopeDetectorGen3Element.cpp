// This file is part of the ACTS project.
//
// Copyright (C) 2016 CERN for the benefit of the ACTS project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "ActsExamples/TelescopeDetectorGen3/TelescopeDetectorGen3Element.hpp"

#include "Acts/Surfaces/DiscSurface.hpp"
#include "Acts/Surfaces/RadialBounds.hpp"

#include <utility>

namespace ActsExamples {

TelescopeDetectorGen3Element::TelescopeDetectorGen3Element(
    Identifier identifier, std::shared_ptr<Acts::Transform3> transform,
    std::shared_ptr<Acts::RadialBounds> bounds, double thickness)
    : m_identifier(identifier),
      m_transform(std::move(transform)),
      m_surface(Acts::Surface::makeShared<Acts::DiscSurface>(bounds, *this)),
      m_bounds(std::move(bounds)),
      m_thickness(thickness) {
  m_surface->assignThickness(thickness);
}

}  // namespace ActsExamples
