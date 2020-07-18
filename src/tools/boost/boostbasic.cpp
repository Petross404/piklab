/***************************************************************************
 *   Copyright (C) 2006 Nicolas Hadacek <hadacek@kde.org>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "boostbasic.h"

#include "boost_generator.h"

//----------------------------------------------------------------------------
bool Boost::CompilerBasic::checkExecutableResult(Tool::ExecutableType, QStringList &lines) const
{
  return ( lines.count()>0 && lines[0].startsWith("BoostBasic ") );
}

//----------------------------------------------------------------------------
QString Boost::GroupBasic::informationText() const
{
  return i18n("<a href=\"%1\">BoostBasic Compiler</a> is a Basic compiler distributed by SourceBoost Technologies.").arg("http://www.sourceboost.com/Products/BoostBasic/Overview.html");
}

Tool::SourceGenerator *Boost::GroupBasic::sourceGeneratorFactory() const
{
  return new BasicSourceGenerator;
}

Tool::Base* Boost::GroupBasic16::baseFactory(Tool::Category category) const
{
  if ( category==Tool::Category::Compiler ) return new Boost::CompilerBasic16;
  if ( category==Tool::Category::Linker )   return new Boost::Linker16;
  return NULL;
}

Tool::Base* Boost::GroupBasic18::baseFactory(Tool::Category category) const
{
  if ( category==Tool::Category::Linker )   return new Boost::Linker18;
  if ( category==Tool::Category::Compiler ) return new Boost::CompilerBasic18;
  return NULL;
}
