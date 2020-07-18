/***************************************************************************
 *   Copyright (C) 2005-2006 Nicolas Hadacek <hadacek@kde.org>             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef PROG_LIST_H
#define PROG_LIST_H

#include "common/common/lister.h"
#include "progs/base/prog_group.h"

namespace Programmer
{

class Lister : public ::Group::Lister<Group>
{
public:
  static const Lister& instance();

private:
  Lister();
};

} // namespace

#endif
