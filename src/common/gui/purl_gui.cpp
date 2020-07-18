/***************************************************************************
 *   Copyright (C) 2006 Nicolas Hadacek <hadacek@kde.org>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "purl_gui.h"
#include "purl_gui.moc"

#include "misc_gui.h"

//-----------------------------------------------------------------------------
PURL::Url PURL::getOpenUrl(const QString &startDir, const QString &filter,
                           QWidget *widget, const QString &caption)
{
  return KFileDialog::getOpenUrl(startDir, filter, widget, caption);
}

PURL::UrlList PURL::getOpenUrls(const QString &startDir, const QString &filter,
                                QWidget *widget, const QString &caption)
{
  return KFileDialog::getOpenUrls(startDir, filter, widget, caption);
}

PURL::Url PURL::getSaveUrl(const QString &startDir, const QString &filter,
                           QWidget *widget, const QString &caption,
                           SaveAction action)
{
  Url url = KFileDialog::getSaveUrl(startDir, filter, widget, caption);
  if ( url.isEmpty() ) return Url();
  switch (action) {
    case NoSaveAction: break;
    case AskOverwrite:
      if ( url.exists() ) {
        if ( !MessageBox::askContinue(i18n("File \"%1\" already exists. Overwrite ?").arg(url.pretty())) ) return Url();
      }
      break;
    case CancelIfExists:
      if ( url.exists() ) return Url();
      break;
  }
  return url;
}

PURL::Directory PURL::getExistingDirectory(const QString &startDir, QWidget *widget,
                                           const QString &caption)
{
  KUrl kurl = KDirSelectDialog::selectDirectory(startDir, false, widget, caption);
  if ( kurl.isEmpty() ) return Directory();
  return Directory(kurl.path(KUrl::AddTrailingSlash));
}

//-----------------------------------------------------------------------------
PURL::BaseWidget::BaseWidget(QWidget *parent, const char *name)
  : QWidget(parent, name)
{
  init();
}

PURL::BaseWidget::BaseWidget(const QString &defaultDir, QWidget *parent, const char *name)
  : QWidget(parent, name), _defaultDir(defaultDir)
{
  init();
}

void PURL::BaseWidget::init()
{
  QHBoxLayout *top = new QHBoxLayout(this, 0, 10);

  _edit = new KLineEdit(this);
  connect(_edit, SIGNAL(textChanged(const QString &)), SIGNAL(changed()));
  top->addWidget(_edit);
  KIcon icon("document-open");
  QPushButton *button = new  KPushButton(icon, QString::null, this);
  connect(button, SIGNAL(clicked()), SLOT(buttonClicked()));
  top->addWidget(button);
}

//----------------------------------------------------------------------------
void PURL::DirectoryWidget::buttonClicked()
{
  Directory dir = getExistingDirectory(_defaultDir, this, i18n("Select Directory"));
  if ( dir.isEmpty() ) return;
  _edit->setText(dir.path());
  emit changed();
}

//----------------------------------------------------------------------------
PURL::DirectoriesWidget::DirectoriesWidget(const QString &title, QWidget *parent, const char *name)
  : Q3VGroupBox(title, parent, name)
{
  init(QString::null);
}

PURL::DirectoriesWidget::DirectoriesWidget(const QString &title, const QString &defaultDir, QWidget *parent, const char *name)
  : Q3VGroupBox(title, parent, name)
{
  init(defaultDir);
}

void PURL::DirectoriesWidget::init(const QString &defaultDir)
{
  DirectoryWidget *edit = new DirectoryWidget(defaultDir);
  _editListBox = new EditListBox(1, edit, edit->lineEdit(), this, "directories_editlistbox");
  connect(_editListBox, SIGNAL(changed()), SIGNAL(changed()));
}

//----------------------------------------------------------------------------
void PURL::UrlWidget::buttonClicked()
{
  Url url = getOpenUrl(_defaultDir, _filter, this, i18n("Select File"));
  if ( url.isEmpty() ) return;
  _edit->setText(url.filepath());
  emit changed();
}
