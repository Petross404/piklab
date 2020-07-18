/***************************************************************************
 *   Copyright (C) 2006 Nicolas Hadacek <hadacek@kde.org>                  *
 *   Copyright (C) 2003-2005 Alain Gibaud <alain.gibaud@free.fr>           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "direct_18F.h"

#include "direct_data.h"

//-----------------------------------------------------------------------------
bool Direct::P18F::skipMaskWords(Pic::MemoryRangeType type, const Device::Array &data,
                                 uint &i, uint nb, bool forceProgram)
{
  if (forceProgram) return false;
  for (uint k=0; k<nb; k++)
    if ( data[i+k]!=device().mask(type) ) return false;
  i += nb;
  return true;
}

void Direct::P18F::program(Type type)
{
  QString cmd;
  switch (type) {
    case Code:
      cmd = QString("d,C,c,C,c,C,c,Cw%1cw%2X0000,").arg(programHighTime(Code)).arg(programLowTime());
      break;
    case Erase:
      pulseEngine("k0,X0000,"); // NOP
      cmd = QString("k0;w%1;w%2;X0000").arg(programHighTime(type)).arg(programLowTime());
      break;
    case Eeprom:
      for (;;) {
        pulseEngine("k0,X50A6,");
        pulseEngine("k0,X6EF5,");
        pulseEngine("k0,X0000,"); // NOP
        BitValue b = get_byte();
        if ( !b.bit(1) ) break; // WR bit clear
      }
      cmd = QString("w%1").arg(programLowTime());
      break;
  }
  pulseEngine(cmd);
}

void Direct::P18F::setCodePointer(uint address)
{
  pulseEngine("k0,S,k0,X6EF8,", 0x0E00 | ((address & 0xFF0000) >> 16));
  pulseEngine("k0,S,k0,X6EF7,", 0x0E00 | ((address & 0x00FF00) >> 8));
  pulseEngine("k0,S,k0,X6EF6,", 0x0E00 | (address & 0x0000FF));
}

void Direct::P18F::setPointer(Pic::MemoryRangeType type, uint offset)
{
  if ( type==Pic::MemoryRangeType::Eeprom ) {
    pulseEngine("k0,S,k0,X6EA9,", 0x0E00 | (offset & 0x00FF));
    pulseEngine("k0,S,k0,X6EAA,", 0x0E00 | ((offset & 0xFF00) >> 8));
  } else setCodePointer(device().range(type).start.toUInt() + offset);
}

void Direct::P18F::directAccess(Pic::MemoryRangeType type)
{
  if ( type==Pic::MemoryRangeType::Code || type==Pic::MemoryRangeType::UserId || type==Pic::MemoryRangeType::Eeprom ) pulseEngine("k0,X9CA6"); // unset EECON1:CFGS
  else pulseEngine("k0,X8CA6"); // set EECON1:CFGS
  if ( type==Pic::MemoryRangeType::Eeprom ) pulseEngine("k0,X9EA6"); // unset EECON1::EEPGD
  else pulseEngine("k0,X8EA6"); // set EECON1::EEPGD
}

bool Direct::P18F::doRead(Pic::MemoryRangeType type, uint wordOffset, Device::Array &data, const ::Programmer::VerifyData *vdata)
{
  ASSERT( wordOffset==0 || type==Pic::MemoryRangeType::Code );
  BitValue mask = device().mask(type);
  //qDebug("read %s %i", Pic::MEMORY_RANGE_TYPE_DATA[type].label, device().nbWords(type));
  //pulseEngine("w300000"); // what for ?
  directAccess(type);
  switch (type.type()) {
    case Pic::MemoryRangeType::Eeprom:
      for (uint i = 0; i<data.count(); i++) {
        setPointer(type, i);
        pulseEngine("k0,X80A6,k0,X50A8,k0,X6EF5,k0,X0000");
        data[i] = pulseEngine("r").maskWith(mask);
        if ( vdata && !hardware().verifyWord(i, data[i], type, *vdata) ) return false;
      }
      _base.progressMonitor().addTaskProgress(data.count());
      break;
    case Pic::MemoryRangeType::Code:
      setPointer(type, wordOffset);
      //pulseEngine("w1000"); ??
      for (uint i=0; i<data.count(); i++) {
        data[i] = pulseEngine("R").maskWith(mask);
        if ( vdata && !hardware().verifyWord(wordOffset+i, data[i], type, *vdata) ) return false;
      }
      _base.progressMonitor().addTaskProgress(data.count());
      break;
    case Pic::MemoryRangeType::UserId:
    case Pic::MemoryRangeType::Config:
    case Pic::MemoryRangeType::DeviceId:
      setPointer(type, 0);
      for (uint i = 0; i<data.count(); i+=2) {
        BitValue w = pulseEngine("R");
        data[i] = w.maskWith(mask);
        if ( vdata && !hardware().verifyWord(i, data[i], type, *vdata) ) return false;
        data[i+1] = w >> 8;
        if ( vdata && !hardware().verifyWord(i+1, data[i+1], type, *vdata) ) return false;
      }
      break;
    default: ASSERT(false); break;
  }
  return true;
}

bool Direct::P18F::doWrite(Pic::MemoryRangeType type, uint wordOffset, const Device::Array &data, bool force)
{
  ASSERT( wordOffset==0 || type==Pic::MemoryRangeType::Code );
  uint inc = device().addressIncrement(type);
  //qDebug("write %s %i/%i %i", Pic::MEMORY_RANGE_TYPE_DATA[type].label, nbWords, data.count(), inc);
  //pulseEngine("w300000"); ??
  configureSinglePanel();
  directAccess(type);
  switch (type.type()) {
    case Pic::MemoryRangeType::UserId: {
      setPointer(type, 0);
      ASSERT( device().nbWords(Pic::MemoryRangeType::UserId)==8 );
      uint i = 0;
      for (uint k=0; k<4; k++) {
        BitValue word = data[i] | data[i+1] << 8;
        if ( (k+1)==4 ) pulseEngine("k15,S,", word);
        else pulseEngine("k13,S,", word);
        i += 2;
      }
      program(Code);
      break;
    }
    case Pic::MemoryRangeType::Code: {
      uint progWidth = device().nbWordsWriteAlignment(Pic::MemoryRangeType::Code);
      for (uint i=0; i<data.count(); ) {
        if ( i!=0 && i%0x100==0 ) _base.progressMonitor().addTaskProgress(0x100);
        if ( skipMaskWords(type, data, i, progWidth, force) ) continue;
        setPointer(type, inc * (wordOffset+i));
        for (uint k=0; k<progWidth; k++) {
          if ( (k+1)==progWidth ) pulseEngine("k15,S,", data[i]);
          else pulseEngine("k13,S,", data[i]);
          i++;
        }
        program(Code);
      }
      _base.progressMonitor().addTaskProgress(data.count()%0x100);
      break;
    }
    case Pic::MemoryRangeType::Config:
      pulseEngine("k0,XEF00,k0,XF800,"); // move PC somewhere else (0x100000)
      for (uint i = 0; i<data.count(); i+=2) {
        setPointer(type, inc * i);
        BitValue w = data[i] | (data[i+1] << 8);
        pulseEngine("k15,S,", w);
        program(Code);
        pulseEngine("k0,X2AF6,"); // increment address
        pulseEngine("k15,S,", w);
        program(Code);
        pulseEngine("k0,X0000,"); // NOP: some devices need 4 NOPS here...
        pulseEngine("k0,X0000,"); // NOP
        pulseEngine("k0,X0000,"); // NOP
        pulseEngine("k0,X0000,"); // NOP
      }
      break;
    case Pic::MemoryRangeType::Eeprom:
      for(uint i = 0; i<data.count(); ) {
        if ( i!=0 && i%0x100==0 ) _base.progressMonitor().addTaskProgress(0x100);
        if ( skipMaskWords(type, data, i, 1, force) ) continue;
        setPointer(type, inc * i);
        pulseEngine("k0,S,k0,X6EA8,", data[i] | 0x0E00); // load data
        pulseEngine("k0,X84A6,"); // enable memory writes
        unlockEeprom();
        pulseEngine("k0,X82A6,"); // write
        program(Eeprom);
        pulseEngine("k0,X94A6,"); // disable writes
        i++;
      }
      _base.progressMonitor().addTaskProgress(data.count()%0x100);
      break;
    default: ASSERT(false); break;
  }
  return true;
}

void Direct::P18F::unlockEeprom()
{
  pulseEngine("k0,X0E55,k0,X6EA7,k0,X0EAA,k0,X6EA7,");
}

bool Direct::P18F::doEraseCommand(uint cmd1, uint cmd2)
{
  if ( cmd1!=0 ) {
    setCodePointer(0x3C0005);
    pulseEngine("k12;X" + toHex(cmd1, 4));
  }
  setCodePointer(0x3C0004);
  pulseEngine("k12;X" + toHex(cmd2, 4));
  program(Erase);
  return true;
}

bool Direct::P18F::doEraseRange(Pic::MemoryRangeType type)
{
  if ( type==Pic::MemoryRangeType::Code ) {
    doEraseCommand(0, 0x0083); // boot
    for (uint i=0; i<device().config().protection().nbBlocks(); i++)
      doEraseCommand(0, 0x0088 + i);
    return true;
  }
  if ( type==Pic::MemoryRangeType::Eeprom ) return doEraseCommand(0, 0x0081);
  return false;
}

bool Direct::P18F::doErase(bool)
{
  return doEraseCommand(0, 0x0080);
}

//-----------------------------------------------------------------------------
void Direct::P18F1220::program(Type type)
{
  if ( type==Eeprom ) {
    pulseEngine("k0,X0000,"); // NOP
    QString cmd = QString("k0;w%1;w%2;X0000").arg(programHighTime(type)).arg(programLowTime());
    pulseEngine(cmd);
  } else P18F::program(type);
}

//-----------------------------------------------------------------------------
void Direct::P18F242::configureSinglePanel()
{
  setCodePointer(0x3C0006);
  pulseEngine("k12,X0000");
}

//-----------------------------------------------------------------------------
bool Direct::P18F2539::doErase(bool)
{
  // apparently there is no chip erase...
  return ( doEraseRange(Pic::MemoryRangeType::Code) && doEraseRange(Pic::MemoryRangeType::Eeprom) );
}

bool Direct::P18F2539::doEraseCommand(uint cmd1, uint cmd2)
{
  ASSERT( cmd1==0 );
  setCodePointer(0x3C0004);
  pulseEngine("k0;X" + toHex(cmd2, 4)); // is that correct ??
  program(Erase);
  return true;
}

//-----------------------------------------------------------------------------
bool Direct::P18F2439::doEraseRange(Pic::MemoryRangeType type)
{
  if ( type==Pic::MemoryRangeType::Code ) {
    configureSinglePanel();
    directAccess(Pic::MemoryRangeType::Code);
    for (uint i=0; i<40; i++) {
      setCodePointer(0x200000 + 64*i);
      pulseEngine("k0;X84A6;k0;X88A6"); // enable memory writes + setup erase
      unlockEeprom();
      pulseEngine("k0;X82A6"); // initiate erase
      program(Erase);
      pulseEngine("k0;X94A6"); // disable memory writes
    }
    return true;
  }
  return P18F2539::doEraseRange(type);
}
//-----------------------------------------------------------------------------
bool Direct::P18F2221::doEraseRange(Pic::MemoryRangeType type)
{
  if ( type==Pic::MemoryRangeType::Code ) {
    doEraseCommand(0, 0x8181); // boot
    for (uint i=0; i<device().config().protection().nbBlocks(); i++) {
      uint v = 1 << i;
      doEraseCommand(v + (v<<8), 0x8080);
    }
    return true;
  }
  if ( type==Pic::MemoryRangeType::Eeprom ) return doEraseCommand(0, 0x8484);
  return false;
}

bool Direct::P18F2221::doErase(bool)
{
  return doEraseCommand(0x0F0F, 0x8787); // chip erase
}

//-----------------------------------------------------------------------------
bool Direct::P18F6527::doErase(bool)
{
  return doEraseCommand(0xFFFF, 0x8787); // chip erase
}

//-----------------------------------------------------------------------------
bool Direct::P18F6310::doErase(bool)
{
  setCodePointer(0x3C0005);
  pulseEngine("k12,X010A");
  setCodePointer(0x3C0004);
  pulseEngine("k12,X018A");
  program(Erase);
  return true;
}

//-----------------------------------------------------------------------------
bool Direct::P18F23K22::doErase(bool)
{
  return doEraseCommand(0x0F0F, 0x8F8F);
}

void Direct::P18F23K22::directAccess(Pic::MemoryRangeType type)
{
  P18F::directAccess(type);
  if ( type!=Pic::MemoryRangeType::Eeprom ) pulseEngine("k0,X84A6"); // enable WREN
}

uint Direct::P18F23K22::programHighTime(Type type) const
{
  if ( type==Erase ) return 15000;
  if ( type==Code ) return 5000;
  return 1000;
}

void Direct::P18F23K22::program(Type type)
{
  if ( type==Eeprom ) {
    pulseEngine("k0,X0000,"); // NOP
    pulseEngine("k0,X0000,"); // NOP
  }
  Direct::P18F::program(type);
}
