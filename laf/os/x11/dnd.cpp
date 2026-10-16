// LAF OS Library
// Copyright (C) 2024  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include "os/x11/dnd.h"

namespace os {

DragDataProviderX11::DragDataProviderX11(::Display* display,
                                         ::Window target,
                                         const base::paths& urls)
  : m_display(display)
  , m_target(target)
  , m_urls(urls)
{
}

base::paths DragDataProviderX11::getPaths()
{
  return m_urls;
}

#if CLIP_ENABLE_IMAGE

SurfaceRef DragDataProviderX11::getImage()
{
  return {}; // TODO add support to drop images
}

#endif // CLIP_ENABLE_IMAGE

std::string DragDataProviderX11::getUrl()
{
  return {};
}

bool DragDataProviderX11::contains(DragDataItemType type)
{
  return (type == DragDataItemType::Paths && !m_urls.empty());
}

} // namespace os
