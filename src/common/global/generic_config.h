/***************************************************************************
 *   Copyright (C) 2006 Nicolas Hadacek <hadacek@kde.org>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef GENERIC_CONFIG_H
#define GENERIC_CONFIG_H

#include <qvariant.h>
#include <qsize.h>

#include "common/common/global.h"
#include "common/common/misc.h"

class GenericConfigPrivate;

class GenericConfig : public NoCopyClass
{
public:
  GenericConfig(const QString &group);
  ~GenericConfig();
  QString group() const;
  static void deleteGroup(const QString &group);

  QString readEntry(const QString &key, const QString &def = QString::null) const;
  void writeEntry(const QString &key, const QString &value);

  QStringList readListEntry(const QString &key, const QStringList &defaultValues) const;
  void writeEntry(const QString &key, const QStringList &value);

  QList<int> readIntListEntry(const QString &key) const;
  void writeEntry(const QString &key, const QList<int> &value);

  QSize readSizeEntry(const QString &key, QSize def = QSize()) const;
  void writeEntry(const QString &key, const QSize &value);

  bool readBoolEntry(const QString &key, bool def) const;
  void writeEntry(const QString &key, bool value);

  int readIntEntry(const QString &key, int def = 0) const;
  void writeEntry(const QString &key, int value);

  uint readUIntEntry(const QString &key, uint def = 0) const { return qMax(0, readIntEntry(key, def)); }
  void writeEntry(const QString &key, uint value) { writeEntry(key, int(value)); }

  template <typename Enum>
  Enum readEnumEntry(const QString &key, Enum def = Enum::Nb_Types) const { return Enum::fromKey(readEntry(key, def.key())); }
  template <typename Enum>
  void writeEnumEntry(const QString &key, Enum v) { writeEntry(key, v.key()); }

  QVariant readVariantEntry(const QString &key, const QVariant &defValue) const;
  void writeVariantEntry(const QString &key, const QVariant &value);

  struct ItemData {
    const char *key, *label;
    QVariant defValue;
  };
  template <typename Type>
  QVariant readVariantEntry(Type type) const {
    return readVariantEntry(type.key(), type.data().defValue);
  }
  template <typename Type>
  void writeVariantEntry(Type type, const QVariant &value) {
    ASSERT( value.type()==type.data().defValue.type() );
    writeVariantEntry(type.key(), value);
  }

private:
  GenericConfigPrivate *_d;
};

#define BEGIN_DECLARE_CONFIG(Type) \
  BEGIN_DECLARE_ENUM(Type)

#define END_DECLARE_CONFIG(Type, group) \
  END_DECLARE_ENUM(Type, GenericConfig::ItemData) \
  inline QVariant readConfigEntry(Type type) { \
    GenericConfig config(group); \
    return config.readVariantEntry<Type>(type); \
  } \
  inline void writeConfigEntry(Type type, const QVariant &v) { \
    GenericConfig config(group); \
    config.writeVariantEntry<Type>(type, v); \
  }

#endif
