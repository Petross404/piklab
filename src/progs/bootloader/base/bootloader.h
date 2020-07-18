/***************************************************************************
 *   Copyright (C) 2007 Nicolas Hadacek <hadacek@kde.org>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include "devices/pic/prog/pic_prog.h"

namespace Bootloader
{
//-----------------------------------------------------------------------------
class Hardware : public ::Programmer::PicHardware
{
public:
  Hardware(::Programmer::Base &base, Port::Base *port) : ::Programmer::PicHardware(base, port, QString::null) {}
  virtual bool write(Pic::MemoryRangeType type, const Device::Array &data) = 0;
  virtual bool read(Pic::MemoryRangeType type, Device::Array &data, const ::Programmer::VerifyData *vdata) = 0;
  virtual bool internalConnectHardware() = 0;
  virtual bool openPort() { return _port->open(); }
};

//-----------------------------------------------------------------------------
class DeviceSpecific : public ::Programmer::PicSpecific
{
public:
  DeviceSpecific(::Programmer::Base &base) : ::Programmer::PicSpecific(base) {}
  Hardware &hardware() { return static_cast<Hardware &>(*_base.hardware()); }
  virtual bool setPowerOff() { return false; }
  virtual bool setPowerOn() { return false; }
  virtual bool setTargetPowerOn(bool) { return true; }
  virtual bool doRead(Pic::MemoryRangeType type, uint, Device::Array &data, const ::Programmer::VerifyData *vdata) { return hardware().read(type, data, vdata); }
  virtual bool doWrite(Pic::MemoryRangeType type, uint, const Device::Array &data, bool) { return hardware().write(type, data); }
};

} // namespace

#endif
