/***************************************************************************
 *   Copyright (C) 2007 Nicolas Hadacek <hadacek@kde.org>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "cc5x_compile.h"
#include "cc5x_compile.moc"

#include "cc5x_config.h"

QStringList CC5X::CompileFile::genericArguments(const Compile::Config &config) const
{
  QStringList args;
  args += "-a";   // produce asm file
  args += "-CC";  // produce cod c file
  args += "-L";   // produce list file
  args += "-eL";  // error details
  //args += "-FM";  // error format for MPLAB
  args += "-o%O"; // set output file
  args += config.includeDirs(Tool::Category::Compiler, "-I");
  args += config.customOptions(Tool::Category::Compiler);
  args += "%I";
  return args;
}

void CC5X::CompileFile::logLine(::Process::OutputType, const QString &line)
{
  if ( parseErrorLine(line, Compile::ParseErrorData("(.*):([0-9]+):(.+)\\[([0-9]+)\\](.+)", 1, 2, 5, 3, Log::LineType::Error)) ) return;
  doLog(Log::LineType::Normal, line, QString::null, 0); // unrecognized
}

QString CC5X::CompileFile::outputFiles() const
{
  return "PURL::Lst PURL::AsmGPAsm PURL::Hex PURL::Cod occ";
}
