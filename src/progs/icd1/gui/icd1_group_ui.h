/***************************************************************************
 *   Copyright (C) 2006 Nicolas Hadacek <hadacek@kde.org>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef ICD1_GROUP_UI_H
#define ICD1_GROUP_UI_H

#include "devices/pic/gui/pic_prog_group_ui.h"
#include "progs/icd1/base/icd1_prog.h"

namespace Icd1
{
class Group;

//----------------------------------------------------------------------------
class AdvancedDialog : public ::Programmer::PicAdvancedDialog
{
Q_OBJECT
public:
  AdvancedDialog(ProgrammerBase &base, QWidget *parent);
  virtual void updateDisplay();

private:
  QLabel *_selfTestLabel;
  ProgrammerBase &base() { return static_cast<ProgrammerBase &>(_base); }
};

//----------------------------------------------------------------------------
class GroupUI : public ::Programmer::GroupUI
{
public:
  virtual ::Programmer::ConfigWidget *createConfigWidget(QWidget *parent) const;
  virtual bool hasAdvancedDialog() const { return true; }
  virtual ::Programmer::AdvancedDialog *createAdvancedDialog(::Programmer::Base &base, QWidget *parent) const;
};

} // namespace

#endif
