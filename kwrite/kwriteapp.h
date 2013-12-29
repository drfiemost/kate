/* This file is part of the KDE project
Copyright (C) 2010 Dominik Haumann <dhaumann kde org>

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

#ifndef __KWRITE_APP_H__
#define __KWRITE_APP_H__

#include "kwritemain.h"

#include <ktexteditor/editor.h>

#include <QList>

class QCommandLineParser;

namespace KTextEditor
{
    class Document;
}

/**
 * KWrite Application
 * This class represents the core kwrite application object
 */
class KWriteApp : public QObject
{
  Q_OBJECT

  //
  // constructors & accessor to app object
  //
  public:
    /**
     * application constructor
     * @param args parsed command line args
     */
    KWriteApp(const QCommandLineParser &args);

    /**
     * application destructor
     */
    ~KWriteApp();

    /**
     * static accessor to avoid casting ;)
     * @return app instance
     */
    static KWriteApp *self ();

    /**
     * further initialization
     */
    void init();

    /**
     * Editor instance
     */
    KTextEditor::Editor *editor () { return m_editor; }

  private:
    /**
     * Static instance of KWriteApp
     */
    static KWriteApp *s_self;
    
    /**
     * kate's command line args
     */
    const QCommandLineParser &m_args;

    /**
     * known main windows
     */
    QList<KWrite*> m_mainWindows;

    /**
     * editor instance
     */
    KTextEditor::Editor *m_editor;
};

#endif
// kate: space-indent on; indent-width 2; replace-tabs on;

