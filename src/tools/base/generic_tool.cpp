/***************************************************************************
 *   Copyright (C) 2005-2007 Nicolas Hadacek <hadacek@kde.org>             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "generic_tool.h"

#include "devices/list/device_list.h"
#include "common/global/process.h"
#include "tools/list/compile_config.h"

//----------------------------------------------------------------------------
const Tool::Category::Data Tool::Category::DATA[Nb_Types] = {
  { "compiler",     I18N_NOOP("Compiler")             },
  { "assembler",    I18N_NOOP("Assembler")            },
  { "linker",       I18N_NOOP("Linker")               },
  { "bin_to_hex",   I18N_NOOP("Bin-to-Hex Convertor") },
  { "librarian",    I18N_NOOP("Librarian")            }
};

const Tool::ExecutableType::Data Tool::ExecutableType::DATA[Nb_Types] = {
  { "unix",    I18N_NOOP("Unix"),    PURL::UnixSeparator    },
  { "windows", I18N_NOOP("Windows"), PURL::WindowsSeparator }
};

const Compile::DirectoryType::Data Compile::DirectoryType::DATA[Nb_Types] = {
  { "executable",    I18N_NOOP("Executable directory")    },
  { "include",       I18N_NOOP("Header directory")        },
  { "linker_script", I18N_NOOP("Linker Script Directory") },
  { "library",       I18N_NOOP("Library Directory")       },
  { "source",        I18N_NOOP("Source Directory")        }
};

const Tool::OutputExecutableType::Data Tool::OutputExecutableType::DATA[Nb_Types] = {
  { I18N_NOOP("COFF"), "coff", PURL::Coff },
  { I18N_NOOP("ELF"),  "elf",  PURL::Elf  }
};

//-----------------------------------------------------------------------------
PURL::Directory Tool::Base::executableDirectory() const
{
  return Compile::Config::directory(*_group, Compile::DirectoryType::Executable);
}

::Process::LineOutput *Tool::Base::checkExecutableProcess(const PURL::Directory &dir, ExecutableType execType, OutputExecutableType outType) const
{
  ::Process::LineOutput *process = new ::Process::LineOutput;
  process->setup(dir.path() + baseExecutable(execType, outType), checkExecutableOptions(execType), execType==ExecutableType::Windows);
  PURL::Directory wdir = checkExecutableWorkingDirectory();
  if ( !wdir.isEmpty() ) process->setWorkingDirectory(wdir.path());
  return process;
}
