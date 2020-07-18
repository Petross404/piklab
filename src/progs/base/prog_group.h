/***************************************************************************
 *   Copyright (C) 2005-2006 Nicolas Hadacek <hadacek@kde.org>             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef PROG_GROUP_H
#define PROG_GROUP_H

#include "common/common/group.h"
#include "common/port/port.h"
#include "common/common/purl_base.h"
#include "generic_prog.h"
namespace Device { class Data; }
namespace Debugger { class Base; class Specific; class DeviceSpecific; }
namespace Hardware { class Config; }

namespace Programmer
{
//-----------------------------------------------------------------------------
class Base;
class Hardware;
class DeviceSpecific;
class ConfigWidget;

enum Property { NoProperty = 0, Programmer = 1, Debugger = 2,
                CanReleaseReset = 4, HasFirmware = 8, CanUploadFirmware = 16,
                NeedDeviceSpecificFirmware = 32, HasSelfTest = 64,
                CanReadMemory = 128, HasConnectedState = 256, Custom = 512 };
Q_DECLARE_FLAGS(Properties, Property)
Q_DECLARE_OPERATORS_FOR_FLAGS(Properties)

enum TargetPowerMode { TargetPowerModeFromConfig, TargetSelfPowered, TargetExternallyPowered };

enum { Nb_InputFileTypes = 3 };
extern const PURL::FileType INPUT_FILE_TYPE_DATA[Nb_InputFileTypes];

//-----------------------------------------------------------------------------
class Group : public ::Group::Base
{
public:
  virtual QString xmlName() const { return name(); }
  virtual ::Hardware::Config *hardwareConfig() const { return 0; }
  virtual Properties properties() const = 0;
  virtual QString statusLabel(PortType type) const;
  virtual bool canReadVoltage(Device::VoltageType type) const = 0;
  bool canReadVoltages() const;
  virtual TargetPowerMode targetPowerMode() const = 0;
  bool isDebugger() const { return ( properties() & Debugger ); }
  bool isSoftware() const;
  virtual bool isPortSupported(PortType type) const = 0;
  virtual bool checkConnection(const HardwareDescription &hd) const;
  ::Programmer::Generic *createProgrammer(bool targetSelfPowered, const Device::Data* data,
                                          const HardwareDescription& hd, Log::Base* log) const;
  ::Debugger::Base *createDebugger(::Programmer::Base &) const;
  virtual uint maxNbBreakpoints(const Device::Data *) const { return 0; }
  virtual bool isInputFileTypeSupported(PURL::FileType) const { return false; }
  virtual ::Programmer::Generic *createBase(const Device::Data *data) const = 0;

protected:
  virtual Hardware *createHardware(::Programmer::Base &base, const HardwareDescription &hd) const = 0;
  virtual DeviceSpecific *createDeviceSpecific(::Programmer::Base &base) const = 0;
  virtual ::Debugger::Base *createDebuggerBase(::Programmer::Base &) const { return NULL; }
  virtual ::Debugger::Specific *createDebuggerSpecific(::Debugger::Base &) const { return NULL; }
  virtual ::Debugger::DeviceSpecific *createDebuggerDeviceSpecific(::Debugger::Base &base) const = 0;
};

} // namespace

#endif
