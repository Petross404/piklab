/***************************************************************************
 * Copyright (C) 2007 Nicolas Hadacek <hadacek@kde.org>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "pickit2v2.h"

#include "progs/base/prog_config.h"
#include "progs/icd2/base/microchip.h"
#include "pickit2v2_data.h"

//-----------------------------------------------------------------------------
const Pickit2V2::FamilyData *Pickit2V2::familyData(const Pic::Data &device)
{
  for (uint i=0; FAMILY_DATA[i].architecture!=Pic::Architecture::Nb_Types; i++)
    if ( FAMILY_DATA[i].architecture==device.architecture() ) return &FAMILY_DATA[i];
  ASSERT(false);
  return 0;
}

//-----------------------------------------------------------------------------
Pickit2V2::Hardware::Hardware(::Programmer::Base &base)
  : ::Programmer::PicHardware(base, new USBPort(base), QString::null),
    _scriptBufferChecksum(0), _deviceSet(false)
{}

bool Pickit2V2::Hardware::internalConnectHardware()
{
  return port().open();
}

bool Pickit2V2::Hardware::setVoltages(double vdd, const FamilyData &fdata)
{
  if ( !setVddOn(false) ) return false;
  if ( !setVddVoltage(vdd, 0.85) ) return false;
  double vpp = fdata.vpp;
  if ( vpp==0.0 ) vpp = vdd;
  if ( !setVppVoltage(vpp, 0.7) ) return false;
  if ( !setVddOn(true) ) return false;
  return true;
}

bool Pickit2V2::Hardware::setTarget()
{
  //setIcspSpeed(0); // #### ??
  //_fastProgramming = true; // #### ??
  uint checksum;
  if ( !getScriptBufferChecksum(checksum) ) return false;
  if ( !_deviceSet || _scriptBufferChecksum!=checksum ) {
    if ( !downloadScripts() ) return false;
  }
  _deviceSet = true;
  return true;
}

bool Pickit2V2::Hardware::readStatus(ushort &status)
{
  if ( !port().command(ReadStatus) ) return false;
  ByteArray a = createArray();
  if ( !port().receive(a) ) return false;
  status = (a[1] << 8) + a[0];
  return true;
}

bool Pickit2V2::Hardware::sendScript(const ushort *script, uint length)
{
  ByteArray cmd = createArray();
  cmd[0] = ClearUploadBuffer;
  cmd[1] = ExecuteScript;
  cmd[2] = length;
  for (uint i=0; i<length; i++) cmd[3+i] = script[i];
  return port().command(cmd);
}

bool Pickit2V2::Hardware::executeScript(uint i)
{
  ASSERT( i!=0 );
  const ScriptData &sdata = SCRIPT_DATA[i-1];
  log(Log::DebugLevel::Extra, QString("execute script #%1: %2").arg(i).arg(sdata.name));
  return sendScript(sdata.data, sdata.length);
}

bool Pickit2V2::Hardware::getScriptBufferChecksum(uint &checksum)
{
  if ( !port().command(ScriptBufferChecksum) ) return false;
  ByteArray array = createArray();
  if ( !port().receive(array) ) return false;
  checksum = (array[0] << 24) + (array[1] << 16) + (array[2] << 8) + array[3];
  log(Log::DebugLevel::Extra, QString("get script buffer checksum: %1").arg(toHexLabel(checksum, 8)));
  return true;
}

bool Pickit2V2::Hardware::downloadScript(ScriptType type, uint i)
{
  if (i==0 ) return true; // empty script
  const ScriptData &sdata = SCRIPT_DATA[i-1];
  log(Log::DebugLevel::Max, QString("  download script #%1 (\"%2\") at position #%3")
                            .arg(i-1).arg(sdata.name).arg(toHexLabel(type, 2)));
  ByteArray cmd = createArray();
  cmd[0] = DownloadScript;
  cmd[1] = type;
  cmd[2] = sdata.length;
  for (uint k=0; k<sdata.length; k++) {
/* ### not needed ???
    if ( !_fastProgramming && sdata.data[k]==0xAAE7 ) {
      ushort next = sdata.data[k+1];
      if ( (next & 0xFF)<170 && next!=0 ) {
        cmd[3+k] = sdata.data[k];
        cmd[3+k+1] = next + next/2;
      } else {
        cmd[3+k] = DelayLong;
        cmd[3+k+1] = 2;
      }
      k++;
    } else if ( !_fastProgramming && sdata.data[k]==0xAAE8 ) {
      ushort next = sdata.data[k+1];
      if ( (next & 0xFF)<171 && next!=0 ) {
        cmd[3+k] = sdata.data[k];
        cmd[3+k+1] = next + next/2;
      } else {
        cmd[3+k] = DelayLong;
        cmd[3+k+1] = 0;
      }
      k++;
    } else
*/
    cmd[3+k] = sdata.data[k];
  }
  return port().command(cmd);
}

bool Pickit2V2::Hardware::downloadScripts()
{
  if ( !port().command(ClearDownloadBuffer) ) return false;
  log(Log::DebugLevel::Extra, "clear scripts buffer");
  if ( !port().command(ClearScriptBuffer) ) return false;
  log(Log::DebugLevel::Extra, "download scripts");
  const Data &d = data(device().name());
  for (uint i=0; i<Nb_ScriptTypes; i++) {
//    if ( i==TestMemoryRead || i==ConfigErase ) continue; // ### to get same checksum as pk2cmd
    if ( !downloadScript(ScriptType(i), d.scriptIndexes[i]) ) return false;
  }
  return getScriptBufferChecksum(_scriptBufferChecksum);
}

bool Pickit2V2::Hardware::setTargetReset(Device::ResetMode mode)
{
  ushort script[1];
  switch (mode.type()) {
    case Device::ResetMode::Held: script[0] = MclrGroundOn; break;
    case Device::ResetMode::Released: script[0] = MclrGroundOff; break;
    case Device::ResetMode::Nb_Types: ASSERT(false); return false;
  }
  return sendScript(script, 1);
}

bool Pickit2V2::Hardware::setVddVoltage(double value, double threshold)
{
  ushort v = ushort(qRound(32.0*value + 10.5)) << 6;
  uchar vfault = uchar(qRound(256.0 * (threshold * value) / 5.0));
  if ( vfault>210 ) vfault = 210;
  ByteArray cmd = createArray();
  cmd[0] = SetVdd;
  cmd[1] = v & 0xFF;
  cmd[2] = v >> 8;
  cmd[3] = vfault;
  return port().command(cmd);
}

bool Pickit2V2::Hardware::setVppVoltage(double value, double threshold)
{
  ByteArray cmd = createArray();
  cmd[0] = SetVpp;
  cmd[1] = 0x40;
  cmd[2] = uchar(qRound(18.61*value));
  cmd[3] = uchar(qRound(18.61*value*threshold));
  return port().command(cmd);
}

bool Pickit2V2::Hardware::setVddOn(bool on)
{
  log(Log::DebugLevel::Extra, QString("Vdd set to %1, self powered is %2").arg(on).arg(_base.isTargetSelfPowered()));
  ushort script[2];
  script[0] = (on ? VddGroundOff : VddOff);
  if ( _base.isTargetSelfPowered() ) script[1] = (on ? VddOff : VddGroundOff);
  else script[1] = (on ? VddOn : VddGroundOn);
  return sendScript(script, 2);
}

bool Pickit2V2::Hardware::setIcspSpeed(uchar speed)
{
  ushort script[2];
  script[0] = SetIcspSpeed;
  script[1] = speed;
  return sendScript(script, 2);
}

bool Pickit2V2::Hardware::getMode(VersionData &version, ::Programmer::Mode &mode)
{
  if ( !port().command(FirmwareVersion) ) return false;
  ByteArray data = createArray();
  if ( !port().receive(data) ) return false;
  if ( data[0]=='B' ) {
    mode = ::Programmer::BootloadMode;
    version = VersionData();
  } else {
    mode = ::Programmer::NormalMode;
    version = VersionData(data[0], data[1], data[2]);
  }
  return true;
}

bool Pickit2V2::Hardware::readVoltages(Device::VoltageValues &voltages)
{
  if ( !port().command(ReadVoltages) ) return false;
  ByteArray array = createArray();
  if ( !port().receive(array) ) return false;
  double vadc = 256 * array[1] + array[0];
  voltages[Device::VoltageType::TargetVdd] = Device::VoltageValue(Device::VoltageValue::Normal, 5.0 * (vadc / 65536));
  vadc = 256 * array[3] + array[2];
  voltages[Device::VoltageType::TargetVpp] = Device::VoltageValue(Device::VoltageValue::Normal, 13.7 * (vadc / 65536));
  return true;
}

bool Pickit2V2::Hardware::downloadAddress(Address address)
{
  log(Log::DebugLevel::Max, QString("download address %1").arg(toHexLabel(address, 6)));
  ByteArray cmd = createArray();
  cmd[0] = ClearDownloadBuffer;
  cmd[1] = DownloadData;
  cmd[2] = 3;
  cmd[3] = address.toUInt() & 0xFF;
  cmd[4] = (address.toUInt() >> 8) & 0xFF;
  cmd[5] = (address.toUInt() >> 16) & 0xFF;
  return port().command(cmd);
}

bool Pickit2V2::Hardware::runScript(ScriptType stype, uint repetitions, uint nbNoLens)
{
  log(Log::DebugLevel::Max, QString("run script %1: repetitions=%2 nbNoLen=%3")
                            .arg(toHexLabel(stype, 2)).arg(repetitions).arg(nbNoLens));
#if 0 // ALTERNATE METHOD
  const Data &d = data(device().name());
  for (uint i=0; i<repetitions; i++)
    if ( !executeScript(d.scriptIndexes[stype]) ) return false;
  if (nbNoLens) {
    ByteArray cmd = createArray();
    for (uint i=0; i<nbNoLens; i++) cmd[i] = UploadDataNoLen;
    if ( !port().command(cmd) ) return false;
  }
#else
  ByteArray cmd = createArray();
  cmd[0] = ClearUploadBuffer;
  cmd[1] = RunScript;
  cmd[2] = stype;
  cmd[3] = repetitions;
  for (uint i=0; i<nbNoLens; i++) cmd[4+i] = UploadDataNoLen;
  if ( !port().command(cmd) ) return false;
#endif
  //if ( stype==ProgExit && !setTargetReset(Pic::ResetMode::Released) ) return false;
  return true;
}

bool Pickit2V2::Hardware::prepareRead(Pic::MemoryRangeType type, uint wordOffset)
{
  ScriptType stype = prepareReadScript(type);
  if ( type!=Pic::MemoryRangeType::Cal && (stype==Nb_ScriptTypes || data(device().name()).scriptIndexes[stype]==0) ) return true;
  switch (type.type()) {
  case Pic::MemoryRangeType::Code:
    if ( !device().architecture().data().properties & Pic::RandomMemoryAccess ) return true;
    if ( !downloadAddress(0x10000 * (wordOffset / 0x8000)) ) return false;
    break;
  case Pic::MemoryRangeType::Eeprom: {
    Address address = (device().nbBytesWord(Pic::MemoryRangeType::Eeprom)==4 ? device().range(Pic::MemoryRangeType::Eeprom).start : 0x000000);
    if ( !downloadAddress(address) ) return false;
    break;
  }
  case Pic::MemoryRangeType::Config: return true; // ConfigPrepareRead unused (?)
  case Pic::MemoryRangeType::UserId: break;
  case Pic::MemoryRangeType::Cal: {
    Address start = device().range(Pic::MemoryRangeType::Cal).start;
    ASSERT( start==device().range(Pic::MemoryRangeType::Code).end+1 );
    return downloadAddress(start.toUInt());
  }
  default: return true;
  case Pic::MemoryRangeType::Nb_Types: ASSERT(false); return false;
  }
  return runScript(stype);
}

bool Pickit2V2::Hardware::readMemory(Pic::MemoryRangeType otype, uint wordOffset, Device::Array &data, const ::Programmer::VerifyData *vdata)
{
  ASSERT( wordOffset==0 );
  ASSERT( data.count()==device().nbWords(otype) );
  uint nbWords = device().nbWords(otype);
  log(Log::DebugLevel::Max, QString("read %1 nbWords=%2").arg(otype.label()).arg(nbWords));
  uint nbBytesWord = device().nbBytesWord(otype);
  // EEPROM is read like regular program memory in midrange
  if ( !device().is18Family() && !device().is16bitFamily() && otype==Pic::MemoryRangeType::Eeprom ) nbBytesWord = 2;
  Pic::MemoryRangeType type = otype;
  if ( otype==Pic::MemoryRangeType::Config && device().range(Pic::MemoryRangeType::Config).start==device().range(Pic::MemoryRangeType::Code).end+1 ) {  // config in code memory
    wordOffset = device().range(Pic::MemoryRangeType::Config).start.toUInt();
    type = Pic::MemoryRangeType::Code;
  }
  bool setAddress = true;
  ScriptType stype = readScript(type);
  ASSERT( stype!=Nb_ScriptTypes );
  const FamilyData *fdata = familyData(device());
  uint nbRunWords = QMIN(UploadBufferNbBytes / nbBytesWord, nbWords);
  uint nbRuns = 1;
  uint nbReceive = (nbRunWords*nbBytesWord + 63) / 64;
  switch (type.type()) {
    case Pic::MemoryRangeType::Code:   nbRuns = nbRunWords / Pickit2V2::data(device().name()).codeReadNbWords; break;
    case Pic::MemoryRangeType::Eeprom: nbRuns = nbRunWords / Pickit2V2::data(device().name()).eepromReadNbWords; break;
    default: break;
  }

  if ( !runScript(ProgEntry) ) return false;
  QValueVector<uint> words;
  for (uint i=0; i<nbWords; ) {
    if (setAddress) {
      setAddress = false;
      if ( !prepareRead(type, wordOffset + i) ) return false;
    }
    if ( type==Pic::MemoryRangeType::Config || type==Pic::MemoryRangeType::Cal ) {
      if ( !runScript(stype, 1, 0) ) return false;
      if ( !port().command(UploadData) ) return false;
      // config memory return includes a length byte that we need to ignore
      if ( !port().receiveWords(nbBytesWord, nbReceive, words, 1) ) return false;
    } else {
      if ( !runScript(stype, nbRuns, nbReceive) ) return false;
      if ( !port().receiveWords(nbBytesWord, nbReceive, words) ) return false;
    }
    log(Log::DebugLevel::Max, QString("nbRunWords=%1 readNbWords=%2 index=%3/%4").arg(nbRunWords).arg(words.count()).arg(i).arg(nbWords));
    for (uint k=0; k<uint(words.count()); k++) {
      if ( i>=nbWords ) break;
      data[i] = words[k];
      if (fdata->progMemShift) data[i] >>= 1;
      data[i] = data[i].maskWith(device().mask(type)); // ### correct ?
      if ( vdata && !verifyWord(i, data[i], type, *vdata) ) return false;
      if ( type==Pic::MemoryRangeType::Code && i!=0x0 && i%0x8000==0 ) setAddress = true;
      i++;
    }
  }
  if ( !runScript(ProgExit) ) return false;
  return true;
}

bool Pickit2V2::Hardware::eraseAll()
{
  const Data &d = data(device().name());
  if ( d.scriptIndexes[ConfigErase]!=0 && d.rowEraseNbWords==0 ) {
    if ( !runScript(ProgEntry) ) return false;
    if ( d.scriptIndexes[ConfigWritePrepare]!=0 ) {
      if ( !downloadAddress(0) ) return false;
      if ( !runScript(ConfigWritePrepare) ) return false;
    }
    if ( !runScript(ConfigErase) ) return false;
    if ( !runScript(ProgExit) ) return false;
  }
  if ( !runScript(ProgEntry) ) return false;
  if ( d.scriptIndexes[EraseChipPrepare]!=0 && !runScript(EraseChipPrepare) ) return false;
  if ( !runScript(ChipErase) ) return false;
  if ( !runScript(ProgExit) ) return false;
  return true;
}

bool Pickit2V2::Hardware::eraseRange(Pic::MemoryRangeType type)
{
  if ( type==Pic::MemoryRangeType::Code ) {
    if ( !runScript(ProgEntry) ) return false;
    if ( !runScript(ProgMemoryErase) ) return false;
    if ( !runScript(ProgExit) ) return false;
    return true;
  }
  if ( type==Pic::MemoryRangeType::Eeprom ) {
    if ( !runScript(ProgEntry) ) return false;
    if ( !runScript(EepromErase) ) return false;
    if ( !runScript(ProgExit) ) return false;
    return true;
  }
  ASSERT(false);
  return false;
}

bool Pickit2V2::Hardware::downloadData(const QValueVector<uint> &data, uint &index, bool clearBuffer)
{
  ASSERT( index<(uint)data.count() );
  ByteArray cmd = createArray();
  uint i = 0;
  if (clearBuffer) { cmd[i] = ClearDownloadBuffer; i++; }
  cmd[i] = DownloadData; i++;
  uint length = QMIN(data.count()-index, 63-i);
  cmd[i] = length; i++;
  for (uint k=0; k<length; k++) cmd[i+k] = data[index+k];
  index += length;
  return port().command(cmd);
}

bool Pickit2V2::Hardware::prepareWrite(Pic::MemoryRangeType type, uint wordOffset)
{
  ScriptType stype = prepareWriteScript(type);
  if ( stype==Nb_ScriptTypes || data(device().name()).scriptIndexes[stype]==0 ) return true;
  switch (type.type()) {
  case Pic::MemoryRangeType::Code:
  case Pic::MemoryRangeType::Config:
    if ( !device().architecture().data().properties & Pic::RandomMemoryAccess ) return true;
    if ( !downloadAddress(0x10000 * (wordOffset / 0x8000)) ) return false;
    break;
  case Pic::MemoryRangeType::Eeprom: {
    Address address = (device().nbBytesWord(Pic::MemoryRangeType::Eeprom)==4 ? device().range(Pic::MemoryRangeType::Eeprom).start : 0x000000);
    if ( !downloadAddress(address) ) return false;
    break;
  }
  case Pic::MemoryRangeType::UserId: break;
  default: return true;
  case Pic::MemoryRangeType::Nb_Types: ASSERT(false); return false;
  }
  return runScript(stype);
}

bool Pickit2V2::Hardware::writeMemory(Pic::MemoryRangeType otype, uint wordOffset, const Device::Array &data, bool force)
{
  // ### FIXME: need to be fixed for 18J family (and for speed)
  ASSERT( wordOffset==0 );
  ASSERT( data.count()==device().nbWords(otype) );
  Q_UNUSED(force);

  uint nbWords = device().nbWords(otype);
  log(Log::DebugLevel::Max, QString("write %1 nbWords=%2").arg(otype.label()).arg(nbWords));
  uint nbBytesWord = device().nbBytesWord(otype);
  // EEPROM is read like regular program memory in midrange
  if ( !device().is18Family() && !device().is16bitFamily() && otype==Pic::MemoryRangeType::Eeprom ) nbBytesWord = 2;
  Pic::MemoryRangeType type = otype;
  if ( otype==Pic::MemoryRangeType::Config && device().range(Pic::MemoryRangeType::Config).start==device().range(Pic::MemoryRangeType::Code).end+1 ) {  // config in code memory
    wordOffset = device().range(Pic::MemoryRangeType::Config).start.toUInt();
    type = Pic::MemoryRangeType::Code;
  }
  ScriptType stype = readScript(type);
  ASSERT( stype!=Nb_ScriptTypes );
  bool setAddress = true;
  const FamilyData *fdata = familyData(device());
  uint nbWordsPerWrite = 1;
  uint nbRuns = 1;
  uint nbWordsDownload = nbWords;
  switch (type.type()) {
    case Pic::MemoryRangeType::Code:
      nbWordsPerWrite = Pickit2V2::data(device().name()).codeWriteNbWords;
      nbWordsDownload = DownloadBufferNbBytes / nbWordsPerWrite;
      nbRuns = nbWordsDownload / nbBytesWord;
      break;
    case Pic::MemoryRangeType::Eeprom:
      nbWordsPerWrite = Pickit2V2::data(device().name()).eepromWriteNbWords;
      nbWordsDownload = QMAX(uint(16), nbWordsPerWrite);
      nbRuns = nbWordsDownload / nbWordsPerWrite;
      break;
    default: break;
  }
  QValueVector<uint> ddata(nbWordsDownload * nbBytesWord);

  if ( !runScript(ProgEntry) ) return false;
  for (uint i=0; i<nbWords; ) {
    if (setAddress) {
      setAddress = false;
      if ( !prepareWrite(type, i) ) return false;
    }
    uint di = 0;
    for (uint j=0; j<nbWordsDownload; j++) {
      if ( i>=nbWords ) break;
      BitValue word = data[i];
      if (fdata->progMemShift) word <<= 1;
      for (uint k=0; k<nbBytesWord; k++) {
        ddata[di] = word.byte(k);
        di++;
      }
      if ( type==Pic::MemoryRangeType::Code && i!=0x0 && i%0x8000==0 ) setAddress = true;
      i++;
    }
    if ( type==Pic::MemoryRangeType::Cal ) {
      ASSERT( ddata.count()==2 ); // calibration has 2 bytes
      ddata.resize(5);
      ddata[3] = ddata[0];
      ddata[4] = ddata[1];
      Address address = device().range(type).start;
      ddata[0] = address.byte(0);
      ddata[1] = address.byte(1);
      ddata[2] = address.byte(2);
    }
    uint ri = 0;
    while ( ri<di ) if ( !downloadData(ddata, ri, ri==0) ) return false;
    if ( !runScript(stype, nbRuns) ) return false;
  }

  if ( !runScript(ProgExit) ) return false;
  return true;
}

//-----------------------------------------------------------------------------
bool Pickit2V2::DeviceSpecific::canEraseRange(Pic::MemoryRangeType type) const
{
  const Data &d = data(device().name());
  if ( type==Pic::MemoryRangeType::Code ) return d.scriptIndexes[ProgMemoryErase];
  if ( type==Pic::MemoryRangeType::Eeprom ) return d.scriptIndexes[EepromErase];
  return false;
}

bool Pickit2V2::DeviceSpecific::canReadRange(Pic::MemoryRangeType type) const
{
  const Data &d = data(device().name());
  if ( type==Pic::MemoryRangeType::Cal ) return d.scriptIndexes[OsccalRead];
  return true;
}

bool Pickit2V2::DeviceSpecific::canWriteRange(Pic::MemoryRangeType type) const
{
  const Data &d = data(device().name());
  if ( type==Pic::MemoryRangeType::Cal ) return d.scriptIndexes[OsccalWrite];
  return true;
}
