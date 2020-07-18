/***************************************************************************
 *   Copyright (C) 2006 Nicolas Hadacek <hadacek@kde.org>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef GPSIM_DEBUG_H
#define GPSIM_DEBUG_H

#include "gpsim.h"
#include "devices/pic/prog/pic_prog.h"
#include "devices/pic/prog/pic_debug.h"

namespace GPSim
{
//-----------------------------------------------------------------------------
class Config : public GenericConfig
{
public:
  Config() : GenericConfig("gpsim") {}
  static PURL::Directory executableDirectory();
  static void writeExecutableDirectory(const PURL::Directory& dir);
};

//-----------------------------------------------------------------------------
class Programmer : public ::Programmer::PicBase
{
public:
  Programmer(const ::Programmer::Group &group, const Pic::Data *data)
   : Programmer::PicBase(group, data, "gpsim_programmer") {}

private:
  virtual VersionData minVersion() const { return VersionData(0, 21, 0); }
  virtual bool verifyDeviceId() { return true; }
  virtual bool checkErase() { return false; }
  virtual bool internalErase(const Device::MemoryRange &) { return false; }
  virtual bool checkRead() { return false; }
  virtual bool internalRead(Device::Memory *, const Device::MemoryRange &, const ::Programmer::VerifyData *) { return false; }
  virtual bool checkProgram(const Device::Memory &) { return false; }
  virtual bool internalProgram(const Device::Memory &, const Device::MemoryRange &) { return false; }
  virtual bool checkVerify() { return false; }
  virtual bool internalVerify(const Device::Memory &, const Device::MemoryRange &, ::Programmer::VerifyActions) { return false; }
};

//-----------------------------------------------------------------------------
class Debugger : public ::Debugger::PicBase
{
public:
  Debugger(Programmer &programmer);
  virtual bool setBreakpoints(const QValueList<Address> &list);
  virtual bool readRegister(const Register::TypeData &data, BitValue &value);
  virtual bool writeRegister(const Register::TypeData &data, BitValue value);
  virtual bool updatePortStatus(uint index, QMap<uint, Device::PortBitData> &bits);

private:
  uint _nbBreakpoints;

  bool findRegExp(const QStringList &lines, const QString &pattern,
                  const QString &label, QString &value) const;
  bool getRegister(const QString &name, BitValue &value);
  bool setRegister(const QString &name, BitValue value);
  bool getRegister(Address address, BitValue &value);
  bool setRegister(Address address, BitValue value);
  Hardware *hardware() { return static_cast<Hardware *>(_programmer.hardware()); }
  const Pic::Data *device() const { return static_cast<const Pic::Data *>(_programmer.device()); }
  virtual bool internalInit();
  virtual bool updateState();
  virtual bool internalRun();
  virtual bool internalStep();
  virtual bool softHalt(bool &success);
  virtual bool hardHalt();
  virtual bool internalReset();
  virtual bool waitForTargetMode(Device::TargetMode) { return true; }
  bool readWreg(BitValue &value);
  bool writeWreg(BitValue value);
};

//-----------------------------------------------------------------------------
class Group : public ::Programmer::PicGroup
{
public:
  virtual QString name() const { return "gpsim"; }
  virtual QString label() const { return i18n("GPSim"); }
  virtual QString statusLabel(PortType type) const;
  virtual ::Programmer::Properties properties() const { return ::Programmer::Debugger | ::Programmer::HasConnectedState; }
  virtual ::Programmer::TargetPowerMode targetPowerMode() const { return ::Programmer::TargetSelfPowered; }
  virtual bool isPortSupported(PortType) const { return false; }
  virtual uint maxNbBreakpoints(const Device::Data *) const { return 100; }
  virtual bool isInputFileTypeSupported(PURL::FileType type) const { return ( type==PURL::Cod || type==PURL::Hex ); }
  virtual bool canReadVoltage(Device::VoltageType) const { return false; }

protected:
  virtual void initSupported();
  virtual ::Programmer::Base *createBase(const Device::Data *data) const { return new Programmer(*this, static_cast<const Pic::Data *>(data)); }
  virtual ::Programmer::Hardware *createHardware(::Programmer::Base &base, const ::Programmer::HardwareDescription &hd) const;
  virtual ::Programmer::DeviceSpecific *createDeviceSpecific(::Programmer::Base &base) const;
  virtual ::Debugger::Base *createDebuggerBase(::Programmer::Base &base) const { return new Debugger(static_cast<Programmer &>(base)); }
  virtual ::Debugger::Specific *createDebuggerSpecific(::Debugger::Base &) const { return NULL; }
};

} // namespace

#endif
