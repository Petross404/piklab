/***************************************************************************
 *   Copyright (C) 2010 Nicolas Hadacek <hadacek@kde.org>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef OP_16F_HH
#define OP_16F_HH

#include "op_16.h"

namespace OpenProg
{
//-----------------------------------------------------------------------------
class P16F : public pic16
{
public:
  P16F(::Programmer::Base &base) : pic16(base) {}

protected:
  void gotoTestMemoryArray(Array& cmd) const;
  virtual bool gotoMemory(Pic::MemoryRangeType type);
};

//-----------------------------------------------------------------------------
class P16F8X : public P16F
{
public:
  P16F8X(::Programmer::Base &base) : P16F(base) {}
  virtual bool doErase(bool isProtected);
  virtual bool doEraseRange(Pic::MemoryRangeType type);
  virtual uint waitProgTime(Pic::MemoryRangeType) const { return 20000; }
  void eraseArray(Array& cmd) const;
};

//-----------------------------------------------------------------------------
class P16CR8X : public P16F8X
{
public:
  P16CR8X(::Programmer::Base &base) : P16F8X(base) {}
  virtual bool doErase(bool) { return false; } // #### can't the eeprom be bulk erased ???
  virtual bool doEraseRange(Pic::MemoryRangeType) { return false; }
  // #### eeprom and config only can be programmed...
};

//-----------------------------------------------------------------------------
class P16F84A : public P16F8X
{
public:
  P16F84A(::Programmer::Base &base) : P16F8X(base) {}
  virtual bool doEraseRange(Pic::MemoryRangeType type);
  virtual void startProgArray(Array&, Pic::MemoryRangeType type) const;
  virtual uint waitProgTime(Pic::MemoryRangeType type) const;
};

typedef class P16F84A P16F87X;

//-----------------------------------------------------------------------------
class P16F7X : public P16F
{
public:
  P16F7X(::Programmer::Base &base) : P16F(base) {}
  virtual bool doEraseRange(Pic::MemoryRangeType type);
  virtual bool doErase(bool isProtected);
  virtual uint nbWordsCodeProg() const { return 2; }
  virtual uint waitProgTime(Pic::MemoryRangeType type) const;
  virtual void endProgArray(Array&, Pic::MemoryRangeType) const;
  virtual bool setPowerOn() { return setPowerOnVppFirst(); }
};

//-----------------------------------------------------------------------------
class P16F7X7 : public P16F7X
{
public:
    P16F7X7(::Programmer::Base &base) : P16F7X(base) {}
    virtual bool setPowerOn() { return setPowerOnVddFirst(false); }
};

//-----------------------------------------------------------------------------
class P16F62X : public P16F84A
{
public:
  P16F62X(::Programmer::Base &base) : P16F84A(base) {}
  virtual bool doErase(bool isProtected);
  virtual uint waitProgTime(Pic::MemoryRangeType) const { return 8000; }
  virtual bool setPowerOn() { return setPowerOnVppFirst(); }
};

//-----------------------------------------------------------------------------
class P16F81X : public P16F
{
public:
  P16F81X(::Programmer::Base &base) : P16F(base) {}
  virtual bool setPowerOn() { return setPowerOnVddFirst(false); }
  virtual bool doEraseRange(Pic::MemoryRangeType type);
  virtual bool doErase(bool isProtected);
  virtual uint nbWordsCodeProg() const { return 4; }
  virtual void startProgArray(Array&, Pic::MemoryRangeType) const;
  virtual uint waitProgTime(Pic::MemoryRangeType) const { return 2000; }
  virtual void endProgArray(Array&, Pic::MemoryRangeType) const;
};

//-----------------------------------------------------------------------------
class P12F675 : public P16F
{
public:
  P12F675(::Programmer::Base &base) : P16F(base) {}
  virtual bool doEraseRange(Pic::MemoryRangeType type);
  virtual bool doErase(bool isProtected);
  virtual uint waitProgTime(Pic::MemoryRangeType type) const;
  virtual bool setPowerOn() { return setPowerOnVppFirst(); }
};

//-----------------------------------------------------------------------------
class P16F62XA : public P16F
{
public:
  P16F62XA(::Programmer::Base &base) : P16F(base) {}
  virtual bool doEraseRange(Pic::MemoryRangeType type);
  virtual bool doErase(bool isProtected);
  virtual uint waitProgTime(Pic::MemoryRangeType type) const;
  virtual bool setPowerOn() { return setPowerOnVppFirst(); }
};

//-----------------------------------------------------------------------------
class P16F87XA : public P16F
{
public:
  P16F87XA(::Programmer::Base &base) : P16F(base) {}
  virtual bool doEraseRange(Pic::MemoryRangeType type);
  virtual bool doErase(bool isProtected);
  virtual uint nbWordsCodeProg() const { return 8; }
  virtual uint waitProgTime(Pic::MemoryRangeType) const { return 8000; }
};

//-----------------------------------------------------------------------------
class P16F88X : public P16F87XA
{
public:
    P16F88X(::Programmer::Base &base) : P16F87XA(base) {}
    virtual bool doErase(bool isProtected);
};

//-----------------------------------------------------------------------------
class P16F913 : public P16F
{
public:
  P16F913(::Programmer::Base &base) : P16F(base) {}
  virtual bool doEraseRange(Pic::MemoryRangeType type);
  virtual bool doErase(bool isProtected);
  virtual uint nbWordsCodeProg() const { return 4; }
  virtual void startProgArray(Array&, Pic::MemoryRangeType) const;
  virtual uint waitProgTime(Pic::MemoryRangeType) const { return 2500; }
  virtual void endProgArray(Array&, Pic::MemoryRangeType) const;
  virtual bool setPowerOn() { return setPowerOnVppFirst(); }
};

class P16F916 : public P16F913
{
public:
  P16F916(::Programmer::Base &base) : P16F913(base) {}
  virtual uint nbWordsCodeProg() const { return 8; }
};

typedef class P16F913 P12F6XX_16F6XX;

class P16F785 : public P16F913
{
public:
  P16F785(::Programmer::Base &base) : P16F913(base) {}
  virtual bool doEraseRange(Pic::MemoryRangeType type);
  virtual bool doErase(bool isProtected);
};

} // namespace

#endif
