/***************************************************************************
 *   Copyright (C) 2006 Nicolas Hadacek <hadacek@kde.org>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "jal.h"

#include "jal_compile.h"
#include "jal_config.h"
#include "devices/pic/pic/pic_memory.h"
#include "devices/list/device_list.h"
#include "common/global/process.h"
#include "jal_generator.h"

//----------------------------------------------------------------------------
bool JAL::Base::checkExecutableResult(Tool::ExecutableType, QStringList &lines) const
{
  QStringList tmp;
  for (int i=0; i<lines.count(); i++)
    if ( !lines[i].contains('\r') ) tmp += lines[i]; // ??
  lines = tmp;
  return ( lines.count()>0 && lines[0].startsWith("jal") );
}

//----------------------------------------------------------------------------
QString JAL::Group::informationText() const
{
  return i18n("<a href=\"%1\">JAL</a> is a high-level language for PIC microcontrollers.").arg("http://jal.sourceforge.net");
}

Tool::Base* JAL::Group::baseFactory(Tool::Category category) const
{
  if ( category==Tool::Category::Compiler ) return new ::JAL::Base;
  return NULL;
}

const char * const SUPPORTED_DEVICES[] = {
  "12C508", "12C509A", "12CE674", "12F629", "12F675",
  "16C84", "16F84", "16F88", "16F873", "16F876", "16F877", "16F628",
  "18F242", "18F252", "18F442", "18F452",
  NULL
};

QValueList<const Device::Data *> JAL::Group::getSupportedDevices(const QString &) const
{
  QValueList<const Device::Data*> list;
  for (uint i=0; SUPPORTED_DEVICES[i]; i++) {
    const Device::Data* data = Device::Lister::instance().data(SUPPORTED_DEVICES[i]);
    ASSERT(data != NULL);
    list.append(data);
  }
  return list;
}

Compile::Process *JAL::Group::processFactory(const Compile::Data &data) const
{
  ASSERT( data.category==Tool::Category::Compiler );
  return new CompileFile;
}

Compile::Config *JAL::Group::configFactory(::Project& project) const
{
  return new Config(project);
}

Tool::SourceGenerator *JAL::Group::sourceGeneratorFactory() const
{
  return new SourceGenerator;
}
