/***************************************************************************
 *   Copyright (C) 2006 Nicolas Hadacek <hadacek@kde.org>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "cli_debug_manager.h"
#include "cli_debug_manager.moc"

#include "progs/base/generic_prog.h"
#include "progs/base/generic_debug.h"

void Debugger::CliManager::internalUpdateView(bool)
{
  if ( Programmer::manager->programmer() && debugger() ) log(Log::LineType::Normal, debugger()->statusString());
  // ### TODO: show current and next PC lines
}
