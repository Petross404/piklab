/***************************************************************************
 *   Copyright (C) 2005-2006 Nicolas Hadacek <hadacek@kde.org>             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef PIC_PROG_H
#define PIC_PROG_H

#include "pic_prog_specific.h"
#include "progs/base/prog_group.h"
#include "devices/base/device_group.h"

//-----------------------------------------------------------------------------
namespace Pic
{
class MemoryRange : public Device::MemoryRange {
public:
  MemoryRange(MemoryRangeType type) : _type(type) {}
  virtual bool all() const { return _type==MemoryRangeType::Nb_Types; }
  MemoryRangeType _type;
};
} //namespace

namespace Programmer
{
//-----------------------------------------------------------------------------
class PicGroup : public Group
{
public:
  virtual ::Debugger::DeviceSpecific* createDebuggerDeviceSpecific(::Debugger::Base &base) const;
};

//-----------------------------------------------------------------------------
class PicBase : public Base
{
public:
  PicBase(const Group &group, const Pic::Data *data, const char *name);
  virtual ~PicBase();
  PicSpecific *deviceSpecific() const { return static_cast<PicSpecific *>(_deviceSpecific.get()); }
  const Pic::Data *device() const { return static_cast<const Pic::Data *>(_device); }
  const Pic::Memory &deviceMemory() const { return *_deviceMemory; }
  const PicGroup &group() const { return static_cast<const PicGroup &>(_group); }

  bool eraseSingle(Pic::MemoryRangeType type);
  bool readSingle(Pic::MemoryRangeType type, Pic::Memory &memory);
  bool programSingle(Pic::MemoryRangeType type, const Pic::Memory &memory);
  bool verifySingle(Pic::MemoryRangeType type, const Pic::Memory &memory);
  bool blankCheckSingle(Pic::MemoryRangeType type);
  bool readCalibration();
  bool programCalibration(const Device::Array &data);

protected:
  PicHardware *hardware() const { return static_cast<PicHardware *>(_hardware.get()); }
  virtual bool verifyDeviceId();
  virtual uint nbSteps(Task task, const Device::MemoryRange &range) const;
  using Base::initProgramming;
  bool initProgramming(Task task);

  virtual bool checkErase();
  virtual bool internalErase(const Device::MemoryRange &range);

  virtual bool checkRead();
  virtual bool internalRead(Device::Memory *memory, const Device::MemoryRange &range, const VerifyData *data);
  bool readRange(Pic::MemoryRangeType type, Pic::Memory *memory, const VerifyData *data);

  virtual bool checkProgram(const Device::Memory &memory);
  virtual bool internalProgram(const Device::Memory &memory, const Device::MemoryRange &range);
  virtual bool programAll(const Pic::Memory &memory);
  bool programAndVerifyRange(Pic::MemoryRangeType type, const Pic::Memory &memory);
  bool programRange(Pic::MemoryRangeType type, const Device::Array &array);

private:
  Pic::Memory *_deviceMemory;
  bool _hasProtectedCode, _hasProtectedEeprom;

  virtual BitValue readDeviceId();
  bool eraseAll();
  bool eraseRange(Pic::MemoryRangeType type);
  bool restoreCalibration();
  bool restoreBandGapBits();
  bool doProgramCalibration(const Device::Array &data);
  bool checkProgramCalibration(const Device::Array &data);
  bool internalProgramCalibration(const Device::Array &data);
  QString prettyCalibration(const Device::Array &data) const;
  bool tryProgramCalibration(const Device::Array &data, bool &success);
  bool preserveCode();
  bool preserveEeprom();
  bool internalEraseRange(Pic::MemoryRangeType type);
  void prepareMemoryForWriting(Pic::Memory& memory, bool issueWarnings);
};

} // namespace

#endif
