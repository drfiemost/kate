/*
    SPDX-FileCopyrightText: 2021 Kåre Särs <kare.sars@iki.fi>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/


#ifndef KateGitBlamePlugin_h
#define KateGitBlamePlugin_h

#include "gitblametooltip.h"

#include <KTextEditor/ConfigPage>
#include <KTextEditor/InlineNoteProvider>
#include <KTextEditor/MainWindow>
#include <KTextEditor/Plugin>

#include <QProcess>

#include <QHash>
#include <QList>
#include <QRegularExpression>
#include <QVariant>
#include <QVector>
#include <QDateTime>
#include <QLocale>

struct KateGitBlameInfo {
    QString commitHash;
    QString name;
    QDateTime date;
    QString line;
};

class KateGitBlamePlugin;
class GitBlameTooltip;

class GitBlameInlineNoteProvider : public KTextEditor::InlineNoteProvider
{
    Q_OBJECT
public:

    GitBlameInlineNoteProvider(KTextEditor::Document *doc, KateGitBlamePlugin *plugin);
    ~GitBlameInlineNoteProvider();

    QVector<int> inlineNotes(int line) const override;
    QSize inlineNoteSize(const KTextEditor::InlineNote &note) const override;
    void paintInlineNote(const KTextEditor::InlineNote &note, QPainter &painter) const override;
    void inlineNoteActivated(const KTextEditor::InlineNote &note, Qt::MouseButtons buttons, const QPoint &globalPos) override;

private:
    KTextEditor::Document *m_doc;
    KateGitBlamePlugin *m_plugin;
    QLocale m_locale;
};

class KateGitBlamePlugin : public KTextEditor::Plugin
{
    Q_OBJECT
public:
    explicit KateGitBlamePlugin(QObject *parent = nullptr, const QList<QVariant> & = QList<QVariant>());
    ~KateGitBlamePlugin() override;

    QObject *createView(KTextEditor::MainWindow *mainWindow) override;

    const KateGitBlameInfo &blameInfo(int lineNr, const QStringView &lineText);

    bool hasBlameInfo() const;

    void readConfig();

    void showCommitInfo(const QString &hash);

private Q_SLOTS:
    void viewChanged(KTextEditor::View *view);

    void blameFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void showFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    void addDocument(KTextEditor::Document *doc);

    void startBlameProcess(const QUrl &url);

    KTextEditor::MainWindow *m_mainWindow;
    QHash<KTextEditor::Document *, GitBlameInlineNoteProvider *> m_inlineNoteProviders;

    QProcess m_showProc;
    QProcess m_blameInfoProc;
    QVector<KateGitBlameInfo> m_blameInfo;
    KTextEditor::View *m_blameInfoView = nullptr;
    int m_lineOffset{0};

    GitBlameTooltip m_tooltip;
};

#endif // KateGitBlamePlugin_h