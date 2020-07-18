/***************************************************************************
 *   Copyright (C) 2006 Nicolas Hadacek <hadacek@kde.org>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "generic_debug.h"

#include "generic_prog.h"
#include "devices/base/register.h"
#include "devices/base/device_group.h"
#include "devices/pic/base/pic.h"

//----------------------------------------------------------------------------
Debugger::Base::Base(Programmer::Base &programmer)
: Log::Base(&programmer), _programmer(programmer),
  _deviceSpecific(NULL),
  _specific(NULL),
  _inputType(PURL::Nb_FileTypes),
  _coff(NULL)
{}

void Debugger::Base::init(DeviceSpecific *deviceSpecific, Specific *specific)
{
  delete _deviceSpecific;
  _deviceSpecific = deviceSpecific;
  delete _specific;
  _specific = specific;
}

Debugger::Base::~Base()
{
  delete _deviceSpecific;
  _deviceSpecific = NULL;
  delete _specific;
  _specific = NULL;
}

const Device::Data *Debugger::Base::device() const
{
  return _programmer.device();
}

bool Debugger::Base::init()
{
  _programmer.setState(Programmer::Stopped);
  log(Log::LineType::Information, i18n("Setting up debugging session."));
  if ( !internalInit() ) {
    log(Log::LineType::Error, i18n("Failed to initialize device for debugging."));
    return false;
  }
  log(Log::LineType::Information, i18n("Ready to start debugging."));
  _programmer.setState(Programmer::Halted);
  return update();
}

bool Debugger::Base::update()
{
  if ( !updateState() ) return false;
  if ( _programmer.state()==::Programmer::Halted ) return _deviceSpecific->updateStatus();
  return true;
}

bool Debugger::Base::run()
{
  if ( !internalRun() ) return false;
  _programmer.setState(::Programmer::Running);
  return update();
}

bool Debugger::Base::step()
{
  if ( !internalStep() ) return false;
  if ( !waitForTargetMode(Device::TargetMode::Stopped) ) return false;
  return update();
}

bool Debugger::Base::halt()
{
  bool success;
  if ( !softHalt(success) ) return false;
  if ( !success ) return hardHalt();
  if ( !update() ) return false;
  log(Log::LineType::Information, QString("Halted at %1").arg(toHexLabel(pc(), _programmer.device()->nbCharsAddress())));
  _programmer.setState(::Programmer::Halted);
  return true;
}

bool Debugger::Base::reset()
{
  if ( !internalReset() ) return false;
  return update();
}

QString Debugger::Base::statusString() const
{
  if ( _programmer.state()!=::Programmer::Halted ) return QString::null;
  return _deviceSpecific->statusString();
}

void Debugger::Base::setupInput(PURL::FileType type, const QString &directory, const QString &filename)
{
  _inputType = type;
  _directory = directory;
  _filename = filename;
}

BitValue Debugger::Base::pc() const
{
  return Register::list().value(pcTypeData());
}

Register::TypeData Debugger::Base::pcTypeData() const
{
  return Register::TypeData("PC", 2*device()->nbBytesAddress());
}
