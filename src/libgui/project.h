/***************************************************************************
 *   Copyright (C) 2005 Nicolas Hadacek <hadacek@kde.org>                  *
 *   Copyright (C) 2003 Alain Gibaud <alain.gibaud@free.fr>                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef PROJECT_H
#define PROJECT_H

#include "devices/base/register.h"
#include "common/global/xml_data_file.h"
#include "tools/base/generic_tool.h"
#include "tools/list/compile_process.h"

class Project : public XmlDataFile
{
public:
  Project(const PURL::Url &url);
  virtual bool load(QString &error);

  PURL::Directory directory() const { return url().directory(); }
  QString name() const { return url().basename(); }
  PURL::UrlList absoluteFiles() const;
  QString version() const;
  QString description() const;
  Tool::OutputType outputType() const;
  PURL::UrlList openedFiles() const;
  PURL::Url customLinkerScript() const;
  QValueList<Register::TypeData> watchedRegisters() const;
  QStringList objectsForLinker(const QString &extension, Tool::ExecutableType execType) const;
  QStringList librariesForLinker(const QString &prefix, Tool::ExecutableType execType) const;
  QValueList<uint> bookmarkLines(const PURL::Url &url) const; // absolute filepath

  void removeFile(const PURL::Url &url); // take absolute filepath (but inside project dir)
  void addFile(const PURL::Url &url); // take absolute filePath (but inside project dir)
  void clearFiles();
  void setVersion(const QString &version);
  void setDescription(const QString &description);
  void setOutputType(Tool::OutputType type);
  void setOpenedFiles(const PURL::UrlList &list);
  void setCustomLinkerScript(const PURL::Url &url);
  void setWatchedRegisters(const QValueList<Register::TypeData> &watched);
  void setBookmarkLines(const PURL::Url &url, const QValueList<uint> &lines); // absolute filepath
};

#endif
