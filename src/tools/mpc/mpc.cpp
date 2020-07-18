/***************************************************************************
 *   Copyright (C) 2007 Nicolas Hadacek <hadacek@kde.org>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "mpc.h"

#include <qregexp.h>

#include "mpc_compile.h"
#include "mpc_config.h"
#include "devices/pic/pic/pic_memory.h"
#include "devices/list/device_list.h"
#include "devices/base/device_group.h"

//----------------------------------------------------------------------------
QValueList<const Device::Data *> MPC::Group::getSupportedDevices(const QString &) const
{
  QValueList<const Device::Data*> list;
  QStringList devices = Device::Lister::instance().group("pic")->supportedDevices();
  for (QStringList::const_iterator it = devices.begin(); it != devices.end(); ++it) {
    const Device::Data& data = *Device::Lister::instance().data(*it);
    Pic::Architecture arch = static_cast<const Pic::Data&>(data).architecture();
    if (arch != Pic::Architecture::MidRange
        && arch != Pic::Architecture::EMidRange
        && arch != Pic::Architecture::P17C) continue;
    list.append(&data);
  }
  return list;
}

Compile::Process *MPC::Group::processFactory(const Compile::Data &data) const
{
  switch (data.category.type()) {
    case Tool::Category::Compiler: return new MPC::CompileFile;
    default: break;
  }
  ASSERT(false);
  return 0;
}

Compile::Config *MPC::Group::configFactory(::Project& project) const
{
  return new Config(project);
}

QString MPC::Group::informationText() const
{
  return i18n("<a href=\"%1\">MPC Compiler</a> is a C compiler distributed by Byte Craft.").arg("http://www.bytecraft.com/mpccaps.html");
}

Tool::Base* MPC::Group::baseFactory(Tool::Category category) const
{
  if ( category==Tool::Category::Compiler ) return new MPC::Base;
  return NULL;
}
