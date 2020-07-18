/***************************************************************************
 *   Copyright (C) 2006 Nicolas Hadacek <hadacek@kde.org>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef GUI_DEBUG_MANAGER_H
#define GUI_DEBUG_MANAGER_H

#include "progs/manager/debug_manager.h"
#include "text_editor.h"
#include "main_global.h"
#include "tools/list/compile_process.h"
namespace Register { class MainView; }

namespace Debugger
{
class GuiManager : public Manager
{
Q_OBJECT
public:
  GuiManager() {}
  virtual void updateDevice();
  void addTextEditor(TextEditor &editor);
  void addRegisterView(Register::MainView &view);
  void clearEditors();
  virtual void clear();

public slots:
  virtual bool update(bool gotoPC);
  void toggleBreakpoint();
  void toggleEnableBreakpoint();
  void clearBreakpoints();
  void showPC() { internalUpdateView(true); }
  void showDisassemblyLocation();

private slots:
  void editorDestroyed();

private:
  QValueList<Editor *> _editors;

  static Breakpoint::Data currentBreakpointData();
  void updateEditorMarks(TextEditor &editor) const; // return PC line
  Register::MainView *registerView() const;
  bool addEditor(Editor &editor);
  virtual bool internalInit();
  virtual bool checkState(bool &first);
  virtual const Programmer::Group *programmerGroup() const { return &Main::programmerGroup(); }
  virtual const Device::Data* deviceData() const { return &Main::deviceData(); }
  virtual PURL::Url coffUrl() const;
  virtual Log::View *compileView() { return &Main::compileLog(); }
  virtual void internalUpdateView(bool gotoPC);
  virtual bool isProjectSource(const PURL::Url &url) const;
  virtual bool checkIfContinueStepping(bool &continueStepping);
};

} // namespace

#endif
