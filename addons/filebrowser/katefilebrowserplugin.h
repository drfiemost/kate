/* This file is part of the KDE project
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2001 Anders Lund <anders.lund@lund.tdcadsl.dk>
   Copyright (C) 2007 Mirko Stocker <me@misto.ch>
   Copyright (C) 2009 Dominik Haumann <dhaumann kde org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KATE_FILEBROWSER_PLUGIN_H
#define KATE_FILEBROWSER_PLUGIN_H

#include <ktexteditor/document.h>
#include <ktexteditor/plugin.h>
#include <ktexteditor/mainwindow.h>
#include <ktexteditor/configpageinterface.h>
#include <ktexteditor/configpage.h>
#include <KTextEditor/SessionConfigInterface>

#include <kurlcombobox.h>
#include <KFile>

class KActionCollection;
class KActionSelector;
class KDirOperator;
class KFileItem;
class KHistoryComboBox;
class KToolBar;

class QToolButton;
class QCheckBox;
class QSpinBox;

class KateFileBrowser;
class KateFileBrowserPluginView;

class KateFileBrowserPlugin: public KTextEditor::Plugin, public KTextEditor::ConfigPageInterface
{
    Q_OBJECT
    Q_INTERFACES(KTextEditor::ConfigPageInterface)

  public:
    explicit KateFileBrowserPlugin( QObject* parent = 0, const QList<QVariant>& = QList<QVariant>() );
    virtual ~KateFileBrowserPlugin()
    {}

    QObject *createView (KTextEditor::MainWindow *mainWindow);

    virtual int configPages() const;
    virtual KTextEditor::ConfigPage *configPage (int number = 0, QWidget *parent = 0);
    
  public Q_SLOTS:
    void viewDestroyed(QObject* view);

  private:
    QList<KateFileBrowserPluginView *> m_views;
};

class KateFileBrowserPluginView : public QObject, public KTextEditor::SessionConfigInterface
{
    Q_OBJECT
    Q_INTERFACES(KTextEditor::SessionConfigInterface)

  public:
    /**
      * Constructor.
      */
    KateFileBrowserPluginView (KTextEditor::Plugin *plugin, KTextEditor::MainWindow *mainWindow);

    /**
     * Virtual destructor.
     */
    ~KateFileBrowserPluginView ();

    void readSessionConfig (const KConfigGroup& config);
    void writeSessionConfig (KConfigGroup& config);

  private:
    bool eventFilter(QObject*, QEvent*);

    QWidget *m_toolView;
    KateFileBrowser *m_fileBrowser;
    KTextEditor::MainWindow *m_mainWindow;
    friend class KateFileBrowserPlugin;
};

#endif //KATE_FILEBROWSER_PLUGIN_H

// kate: space-indent on; indent-width 2; replace-tabs on;
