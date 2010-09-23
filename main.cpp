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

#include <KApplication>
#include <KCmdLineArgs>
#include <KAboutData>

#include "mainwindow.h"
#include "infosplash.h"

int main( int argc, char *argv[] )
{
    KAboutData aboutData( "nepomukshell",
                          "nepomukshell",
                          ki18n("Nepomuk Shell"),
                          "0.6",
                          ki18n("NepSak - The Nepomuk Shell"),
                          KAboutData::License_GPL,
                          ki18n("(c) 2008-2010, Sebastian Trüg"),
                          KLocalizedString(),
                          "http://nepomuk.kde.org" );
    aboutData.addAuthor(ki18n("Sebastian Trüg"), ki18n("Maintainer"), "trueg@kde.org");
    aboutData.addAuthor(ki18n("Vishesh Handa"), ki18n( "Developer" ), "handa.vish@gmail.com" );

    aboutData.setProgramIconName( "nepomuk" );

    KCmdLineArgs::init( argc, argv, &aboutData );
    KCmdLineOptions options;
	options.add("+[uri]", ki18n("An optional URI of a file or resource to edit"));
    KCmdLineArgs::addCmdLineOptions( options );
    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

    KApplication app;

    MainWindow* mainWin = new MainWindow();
    mainWin->show();
    if ( args->count() )
        mainWin->openResource( args->url( 0 ) );

    InfoSplash* splash = new InfoSplash( mainWin );
    splash->exec();
    delete splash;

    return app.exec();
}
