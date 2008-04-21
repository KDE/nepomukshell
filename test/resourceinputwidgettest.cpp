/*
   Copyright (C) 2008 by Sebastian Trueg <trueg at kde.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef _NEPOMUK_RESOURCE_INPUT_WIDGET_TEST_H_
#define _NEPOMUK_RESOURCE_INPUT_WIDGET_TEST_H_

#include <KApplication>
#include <KCmdLineArgs>
#include <KAboutData>

#include <QUrl>

#include <Nepomuk/Types/Class>

#include "../resourceinputwidget.h"

int main( int argc, char** argv )
{
    KAboutData aboutData( "test", 0,
                          ki18n("Test"),
                          "0.2",
                          ki18n("Test"),
                          KAboutData::License_GPL,
                          ki18n("(c) 2008, Sebastian Trüg"),
                          KLocalizedString(),
                          "http://nepomuk.kde.org" );
    aboutData.addAuthor(ki18n("Sebastian Trüg"),ki18n("Maintainer"), "trueg@kde.org");
    aboutData.setProgramIconName( "nepomuk" );

    KCmdLineArgs::init( argc, argv, &aboutData );

    KCmdLineOptions options;
    options.add( "+type", ki18n("The base class to use" ) );
    KCmdLineArgs::addCmdLineOptions( options );

    KApplication app;

    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

    Nepomuk::ResourceInputWidget* mainWin = new Nepomuk::ResourceInputWidget();
    if ( args->count() ) {
        mainWin->setType( QUrl( args->arg( 0 ) ) );
    }

    mainWin->show();

    return app.exec();
}

#endif
