/***************************************************************************
 *   Copyright (C) 2006 Nicolas Hadacek <hadacek@kde.org>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "compile_config.h"

#include "common/global/generic_config.h"
#include "devices/list/device_list.h"
#include "libgui/project.h"
#include "tool_list.h"
#include "progs/list/prog_list.h"

//----------------------------------------------------------------------------
PURL::Directory Compile::Config::directory(const Tool::Group &group, DirectoryType type)
{
  QString def;
  if ( type!=DirectoryType::Executable )
    def = group.autodetectDirectory(type, directory(group, DirectoryType::Executable), executableType(group)).path();
  return PURL::Directory(value(group.name(), QString::null, type.key() + QString("_path"), def));
}
void Compile::Config::setDirectory(const Tool::Group &group, DirectoryType type, const PURL::Directory &dir)
{
  setValue(group.name(), QString::null, type.key() + QString("_path"), dir.path());
  if ( type==DirectoryType::Executable ) const_cast<Tool::Group &>(group).init();
}

Tool::ExecutableType Compile::Config::executableType(const Tool::Group &group)
{
  QString def = (group.preferedExecutableType()==Tool::ExecutableType::Unix ? "false" : "true");
  if ( value(group.name(), QString::null, "with_wine", def)=="true" ) return Tool::ExecutableType::Windows;
  return Tool::ExecutableType::Unix;
}
void Compile::Config::setExecutableType(const Tool::Group &group, Tool::ExecutableType type)
{
  setValue(group.name(), QString::null, "with_wine", type==Tool::ExecutableType::Windows ? "true" : "false");
  const_cast<Tool::Group &>(group).init();
}

Tool::OutputExecutableType Compile::Config::outputExecutableType(const Tool::Group &group)
{
  QString s = value(group.name(), QString::null, "output_type", Tool::OutputExecutableType(Tool::OutputExecutableType::Coff).key());
  return Tool::OutputExecutableType::fromKey(s);
}
void Compile::Config::setOutputExecutableType(const Tool::Group &group, Tool::OutputExecutableType type)
{
  setValue(group.name(), QString::null, "output_type", type.key());
  const_cast<Tool::Group &>(group).init();
}

QString Compile::Config::value(const QString &group, const QString &subGroup, const QString &key, const QString &defaultValue)
{
  QString grp = (subGroup.isEmpty() ? group : group + '-' + subGroup);
  GenericConfig gc(grp);
  return gc.readEntry(key, defaultValue);
}
void Compile::Config::setValue(const QString &group, const QString &subGroup, const QString &key, const QString &value)
{
  QString grp = (subGroup.isEmpty() ? group : group + '-' + subGroup);
  GenericConfig gc(grp);
  gc.writeEntry(key, value);
}
QStringList Compile::Config::listValues(const QString &group, const QString &subGroup, const QString &key, const QStringList &defaultValues)
{
  QString grp = (subGroup.isEmpty() ? group : group + '-' + subGroup);
  GenericConfig gc(grp);
  return gc.readListEntry(key, defaultValues);
}
void Compile::Config::setListValues(const QString &group, const QString &subGroup, const QString &key, const QStringList &values)
{
  QString grp = (subGroup.isEmpty() ? group : group + '-' + subGroup);
  GenericConfig gc(grp);
  gc.writeEntry(key, values);
}

QStringList Compile::Config::includeDirs(Tool::Category category, const QString &prefix, const QString &suffix, const QString &separator) const
{
  QStringList list;
  QStringList raw = rawIncludeDirs(category);
  for (int i=0; i<raw.size(); i++) {
    if ( separator.isEmpty() ) list.append(prefix + raw[i] + suffix);
    else if ( i==0 ) list.append(prefix + raw[i]);
    else list[0] += separator + raw[i];
  }
  if ( !separator.isEmpty() && list.count()!=0 ) list[0] += suffix;
  return list;
}

HexBuffer::Format Compile::Config::hexFormat() const
{
  QString s = value(Tool::Category::Linker, "format", HexBuffer::FORMATS[HexBuffer::IHX32]);
  for (uint i=0; i<HexBuffer::Nb_Formats; i++)
    if ( s==HexBuffer::FORMATS[i] ) return HexBuffer::Format(i);
  return HexBuffer::Nb_Formats;
}
void Compile::Config::setHexFormat(HexBuffer::Format f)
{
  QString s = (f==HexBuffer::Nb_Formats ? 0 : HexBuffer::FORMATS[f]);
  setValue(Tool::Category::Linker, "format", s);
}

const Device::Data& Compile::Config::device(const Project *project)
{
  QString name = globalValue(project, "device", QString::null);
  return Device::Lister::instance().checkedData(name);
}

const Tool::Group& Compile::Config::toolGroup(const Project *project)
{
  QString s = globalValue(project, "tool", QString::null);
  const Tool::Group* group = Tool::Lister::instance().group(s);
  if (group == NULL) return *Tool::Lister::instance().begin().data();
  return *group;
}

const Programmer::Group& Compile::Config::programmerGroup(const Project *project)
{
  QString s = globalValue(project, "programmer", QString::null);
  const Programmer::Group* group = Programmer::Lister::instance().group(s);
  if (group == NULL && project != NULL) group = &programmerGroup(NULL);
  if (group == NULL) return *Programmer::Lister::instance().begin().data();
  else return *group;
}

QStringList Compile::Config::customOptions(Tool::Category category) const
{
  return QStringList::split(' ', rawCustomOptions(category));
}

QStringList Compile::Config::customLibraries(Tool::Category category) const
{
  return QStringList::split(' ', rawCustomLibraries(category));
}

void Compile::Config::setValue(Tool::Category category, const QString &key, const QString &value)
{
  ASSERT( category!=Tool::Category::Nb_Types );
  ASSERT( _project || _group );
  if (_project) _project->setValue(category.key(), key, value);
  else Config::setValue(_group->name(), category.key(), key, value);
}
QString Compile::Config::value(Tool::Category category, const QString &key, const QString &defaultValue) const
{
  ASSERT( category!=Tool::Category::Nb_Types );
  ASSERT( _project || _group );
  if (_project) return _project->value(category.key(), key, defaultValue);
  return Config::value(_group->name(), category.key(), key, defaultValue);
}
void Compile::Config::setListValues(Tool::Category category, const QString &key, const QStringList &values)
{
  ASSERT( category!=Tool::Category::Nb_Types );
  ASSERT( _project || _group );
  if (_project) _project->setListValues(category.key(), key, values);
  else Config::setListValues(_group->name(), category.key(), key, values);
}
QStringList Compile::Config::listValues(Tool::Category category, const QString &key, const QStringList &defaultValues) const
{
  ASSERT( category!=Tool::Category::Nb_Types );
  ASSERT( _project || _group );
  if (_project) return _project->listValues(category.key(), key, defaultValues);
  return Config::listValues(_group->name(), category.key(), key, defaultValues);
}
bool Compile::Config::boolValue(Tool::Category category, const QString &key, bool defaultValue) const
{
  QString s = value(category, key, QString::null);
  if ( s.isNull() ) return defaultValue;
  return !( s=="false" || s=="0" );
}
uint Compile::Config::uintValue(Tool::Category category, const QString &key, uint defaultValue) const
{
  bool ok;
  uint i = value(category, key, QString::null).toUInt(&ok);
  if ( !ok ) return defaultValue;
  return i;
}

QString Compile::Config::globalValue(const Project *project, const QString &key, const QString &defaultValue)
{
  if (project) return project->value("general", key, defaultValue);
  return Config::value("general", QString::null, key, defaultValue);
}
void Compile::Config::setGlobalValue(Project *project, const QString &key, const QString &value)
{
  if (project) project->setValue("general", key, value);
  else Config::setValue("general", QString::null, key, value);
}

QStringList Compile::Config::globalListValues(const Project *project, const QString &key, const QStringList &defaultValues)
{
  if (project) return project->listValues("general", key, defaultValues);
  return Config::listValues("general", QString::null, key, defaultValues);
}
void Compile::Config::setGlobalListValues(Project *project, const QString &key, const QStringList &values)
{
  if (project) project->setListValues("general", key, values);
  else Config::setListValues("general", QString::null, key, values);
}
