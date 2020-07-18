/***************************************************************************
 *   Copyright (C) 2006 Nicolas Hadacek <hadacek@kde.org>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

#include "common/common/purl_base.h"
#include "devices/pic/base/pic_config.h"
namespace Device { class Data; class Memory; }
namespace Pic { class Data; class Memory; }

//-----------------------------------------------------------------------------
namespace SourceLine
{

enum Type { Indented, NotIndented, Title, Separator, Empty };

class Data {
public:
  Data(Type _type = Empty, const QString &_code = QString::null, const QString &_comment = QString::null)
    : type(_type), code(_code), comment(_comment) {}
  Type type;
  QString code, comment;
};

class List : public QValueList<Data>
{
public:
  List() {}
  void appendSeparator() { append(Separator); }
  void appendEmpty() { append(Empty); }
  void appendTitle(const QString &text) { append(Data(Title, QString::null, text)); }
  void appendIndentedCode(const QString &code, const QString &comment = QString::null) { append(Data(Indented, code, comment)); }
  void appendNotIndentedCode(const QString &code, const QString &comment = QString::null) { append(Data(NotIndented, code, comment)); }
};

extern QString comment(PURL::SourceFamily family, const QString &text);
extern QStringList lines(PURL::SourceFamily family, const List &list, uint nbSpaces);
extern QString text(PURL::SourceFamily family, const List &list, uint nbSpaces);
extern QString transformConfigName(const Pic::Data &data, uint wordIndex, const QString &name);
extern QStringList ignoredConfigNames(const Pic::Data &data, uint wordIndex);
extern QStringList extraConfigNames(Pic::ConfigNameType type, const Pic::Data &data, uint word, const Pic::Config::Value &value);
extern QStringList configNames(Pic::ConfigNameType type, const Pic::Memory &memory, uint word, bool &ok);

} // namespace

//-----------------------------------------------------------------------------
namespace GPUtils
{

extern QString toDeviceName(const QString &device);
extern SourceLine::List includeLines(const Device::Data &data);
extern SourceLine::List generateConfigLines(const Pic::Memory &memory, bool &ok);
extern SourceLine::List disassemble(const Pic::Memory &memory);

} // namespace

//-----------------------------------------------------------------------------
namespace Tool
{

class SourceGenerator
{
public:
  SourceGenerator() {}
  virtual ~SourceGenerator() {}
  virtual SourceLine::List configLines(PURL::ToolType type, const Device::Memory &memory, bool &ok) const = 0;
  SourceLine::List templateSourceFile(PURL::ToolType type, const Device::Data &data, bool &ok) const;
  virtual SourceLine::List sourceFileContent(PURL::ToolType type, const Device::Data &data, bool &ok) const = 0;
  virtual SourceLine::List includeLines(PURL::ToolType type, const Device::Data &data) const = 0;
};

} // namespace

#endif
