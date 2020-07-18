/***************************************************************************
 *   Copyright (C) 2007 Nicolas Hadacek <hadacek@kde.org>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef CHECKSUM_CHECK_H
#define CHECKSUM_CHECK_H

#include "piklab-test/base/device_test.h"
#include "common/cli/cli_log.h"
#include "common/global/pfile.h"
#include "devices/pic/pic/pic_memory.h"

class ChecksumCheck : public DeviceTest
{
public:
  virtual bool skip(const Device::Data &data) const;
  virtual bool init(const Device::Data &data);
  virtual bool executeDevice(const Device::Data &data);
  virtual void cleanup(const Device::Data &data);

private:
  std::auto_ptr<Device::Memory> _memory;

  void setProtection(const Pic::Data &data, const Pic::Checksum::Data &cdata,
                     const QString &maskName, const QString &valueName);
  bool checkChecksum(BitValue checksum, const QString &label);
  void checkChecksum(const Pic::Data &data, const QString &maskName, const QString &valueName, bool &ok);
};

#endif
