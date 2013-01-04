/*
   Copyright (C) 2008-2010 by Sebastian Trueg <trueg at kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _MAIN_WINDOW_H_
#define _MAIN_WINDOW_H_

#include <KXmlGuiWindow>

#include <Nepomuk2/Resource>

#include "ui_mainwindow.h"

class KAction;
class ResourceBrowserWidget;
class ResourceEditorWidget;
class ResourceQueryWidget;

class MainWindow : public KXmlGuiWindow, private Ui::MainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

    QList<Nepomuk2::Resource> selectedResources() const;

    static MainWindow* nepomukShellMain();

public Q_SLOTS:
    void openResource( const Nepomuk2::Resource& res );
    void openResource( const KUrl& url );

private Q_SLOTS:
    void slotModeBrowse();
    void slotModeQuery();
    void slotModeEdit();
    void slotResourcesSelected( const QList<Nepomuk2::Resource>& res );
    void slotResourceActivated( const Nepomuk2::Resource& res );
    void slotDeleteResource();
    void slotSettings();
    void slotOpen();
    void slotShowSource();
    void slotAutoIndentQuery();

private:
    bool queryClose();

    void setupActions();
    void readSettings();
    void saveSettings();

    ResourceBrowserWidget* m_resourceBrowser;
    ResourceQueryWidget* m_resourceQueryWidget;
    ResourceEditorWidget* m_resourceEditor;

    KAction* m_actionNewSubClass;
    KAction* m_actionNewProperty;
    KAction* m_actionNewResource;
    KAction* m_actionAutoIndentQuery;
    
    KAction* m_actionModeBrowse;
    KAction* m_actionModeQuery;
    KAction* m_actionModeEdit;

    KAction* m_actionDelete;
    KAction* m_actionShowSource;
    QActionGroup* m_resourceActionGroup;

    static MainWindow* s_self;
};

#endif
