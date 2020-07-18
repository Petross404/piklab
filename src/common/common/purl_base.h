/***************************************************************************
 *   Copyright (C) 2005-2006 Nicolas Hadacek <hadacek@kde.org>             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef PURL_BASE_H
#define PURL_BASE_H

#include "global.h"
#include "key_enum.h"

//----------------------------------------------------------------------------
namespace PURL
{
BEGIN_DECLARE_ENUM(ToolType)
  Assembler = 0, Compiler
END_DECLARE_ENUM_KEY_LABEL(ToolType)

enum FileTypeEnum {
  AsmGPAsm = 0, AsmPIC30, AsmPICC, Inc, CSource, CppSource, CHeader, JalSource, BasicSource,
  Object, Library, Lkr, Gld, Hex, Elf, Project, Lst, Map, Cod, Coff,
  Unknown, PikdevProject,
  Nb_FileTypes
};

struct SourceFamilyData {
  ToolType     toolType;
  const char  *key, *label;
  FileTypeEnum headerType;
};
BEGIN_DECLARE_ENUM(SourceFamily)
  Asm, C, JAL, Cpp, Basic
END_DECLARE_ENUM(SourceFamily, SourceFamilyData)

enum FileGroup { Source = 0, Header, LinkerScript, LinkerObject, Nb_FileGroups };

enum FileProperty { NoProperty = 0, Editable = 1, ReadOnly = 2 };
Q_DECLARE_FLAGS(FileProperties, FileProperty)
Q_DECLARE_OPERATORS_FOR_FLAGS(FileProperties)

struct FileTypeData {
  const char *key;
  FileGroup group;
  FileProperties properties;
  SourceFamily sourceFamily;
  const char *label;
  const char *extensions[10];
  const char **xpm_icon;
  const char *mimetype;
  const char *highlightModeName;
};
#ifndef Q_MOC_RUN // needed because MOC does not expand defines...
class FileType : public GenericEnum
{
public:
  typedef FileTypeEnum Type;
  enum { Nb_Types = Nb_FileTypes };
  typedef FileTypeData Data;
DECLARE_DATA
DECLARE_KEY_METHODS
DECLARE_LABEL_METHODS
DECLARE_ENUM_CLASS(FileType)
#endif

// add correct extension if filename has no extension
extern QString addExtension(const QString &filename, FileType type);
extern QString extension(FileType type);
extern QString extensions(FileType type);
extern QString filter(FileType type);
enum FilterType { SimpleFilter, CompleteFilter };
extern QString sourceFilter(FilterType type);
extern QString objectFilter(FilterType type);
extern QString projectFilter(FilterType type);
extern QString extensions(FileGroup group);

} // namespace

#endif
