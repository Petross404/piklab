/***************************************************************************
 *   Copyright (C) 2006-2007 Nicolas Hadacek <hadacek@kde.org>             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef PURL_GUI_H
#define PURL_GUI_H

#include "common/gui/dialog.h"
#include "common/global/purl.h"
#include "editlistbox.h"

namespace PURL
{
//-----------------------------------------------------------------------------
bool hasMimetype(FileType type);
QPixmap icon(FileType type);
Directory getExistingDirectory(const QString &startDir, QWidget *widget, const QString &caption);
Url getOpenUrl(const QString &startDir, const QString &filter, QWidget *widget,
               const QString &caption);
UrlList getOpenUrls(const QString &startDir, const QString &filter, QWidget *widget,
                          const QString &caption);
enum SaveAction { NoSaveAction, AskOverwrite, CancelIfExists };
Url getSaveUrl(const QString &startDir, const QString &filter, QWidget *widget,
               const QString &caption, SaveAction action);

//-----------------------------------------------------------------------------
class Label : public KUrlLabel
{
Q_OBJECT
public:
  Label(const QString &url, const QString &text, QWidget *parent = 0, const char *name = 0);

private slots:
  void urlClickedSlot();
};

//-----------------------------------------------------------------------------
class BaseWidget : public QWidget
{
Q_OBJECT
public:
  BaseWidget(QWidget *parent = 0, const char *name = 0);
  BaseWidget(const QString &defaultDir, QWidget *parent = 0, const char *name = 0);
  KLineEdit *lineEdit() { return _edit; }

signals:
  void changed();

protected slots:
  virtual void buttonClicked() = 0;

protected:
  QString    _defaultDir;
  KLineEdit *_edit;

  void init();
};

//-----------------------------------------------------------------------------
class DirectoryWidget : public BaseWidget
{
Q_OBJECT
public:
  DirectoryWidget(QWidget *parent = 0, const char *name = 0) : BaseWidget(parent, name) {}
  DirectoryWidget(const QString &defaultDir, QWidget *parent = 0, const char *name = 0) : BaseWidget(defaultDir, parent, name) {}
  void setDirectory(const Directory &dir) { _edit->setText(dir.path()); }
  Directory directory() const { return _edit->text(); }

protected slots:
  virtual void buttonClicked();
};

//-----------------------------------------------------------------------------
class DirectoriesWidget : public Q3VGroupBox
{
Q_OBJECT
public:
  DirectoriesWidget(const QString &title, QWidget *parent = 0, const char *name = 0);
  DirectoriesWidget(const QString &title, const QString &defaultDir, QWidget *parent = 0, const char *name = 0);
  void setDirectories(const QStringList &dirs) { _editListBox->setTexts(dirs); }
  QStringList directories() const { return _editListBox->texts(); }

signals:
  void changed();

private:
  EditListBox *_editListBox;
  void init(const QString &defaultDir);
};

//-----------------------------------------------------------------------------
class UrlWidget : public BaseWidget
{
Q_OBJECT
public:
  UrlWidget(const QString &filter, QWidget *parent = 0, const char *name = 0)
    : BaseWidget(parent, name), _filter(filter) {}
  UrlWidget(const QString &defaultDir, const QString &filter, QWidget *parent = 0, const char *name = 0)
    : BaseWidget(defaultDir, parent, name), _filter(filter) {}
  Url url() const { return PURL::fromPathOrUrl(_edit->text(), false); }
  void setUrl(const Url &url) { _edit->setText(url.filepath()); }

protected slots:
  virtual void buttonClicked();

private:
  QString _filter;
};

} // namespace

#endif
