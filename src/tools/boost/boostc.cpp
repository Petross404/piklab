/***************************************************************************
 *   Copyright (C) 2006 Nicolas Hadacek <hadacek@kde.org>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "boostc.h"

#include "boost_generator.h"

//----------------------------------------------------------------------------
bool Boost::CompilerC::checkExecutableResult(Tool::ExecutableType, QStringList &lines) const
{
  return ( lines.count()>0 && lines[0].startsWith("BoostC ") );
}

//----------------------------------------------------------------------------
QString Boost::GroupC::informationText() const
{
  return i18n("<a href=\"%1\">BoostC Compiler</a> is a C compiler distributed by SourceBoost Technologies.").arg("http://www.sourceboost.com/Products/BoostC/Overview.html");
}

Tool::SourceGenerator *Boost::GroupC::sourceGeneratorFactory() const
{
  return new CSourceGenerator;
}

Tool::Base* Boost::GroupC16::baseFactory(Tool::Category category) const
{
  if ( category==Tool::Category::Compiler ) return new Boost::CompilerC16;
  if ( category==Tool::Category::Linker )   return new Boost::Linker16;
  return NULL;
}

Tool::Base* Boost::GroupC18::baseFactory(Tool::Category category) const
{
  if ( category==Tool::Category::Compiler ) return new Boost::CompilerC18;
  if ( category==Tool::Category::Linker )   return new Boost::Linker18;
  return NULL;
}
