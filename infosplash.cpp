/*
   Copyright (c) 2010 Sebastian Trueg <trueg@kde.org>

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

#include "infosplash.h"

#include <KLocale>
#include <KIcon>
#include <KComponentData>
#include <KAboutData>

#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>


InfoSplash::InfoSplash( QWidget* parent )
    : KDialog( parent )
{
    setButtons( Ok );
    setCaption( i18n("Welcome") );

    QWidget* w = new QWidget( this );
    QVBoxLayout* lay = new QVBoxLayout( w );

    QLabel* iconLabel = new QLabel( w );
    iconLabel->setPixmap( KIcon(QLatin1String("nepomuk")).pixmap( 64, 64 ) );
    iconLabel->setAlignment( Qt::AlignCenter );

    QLabel* label
        = new QLabel( i18n("<h2>NepSaK</h2>"
                           "<p>The <b>Nep</b>omuk <b>S</b>wiss <b>A</b>rmy <b>K</b>nife</p> %1"
                           "<p>Copyright (C) 2008-2010 Sebastian Trueg &lt;trueg@kde.org&gt;<br/>2010 Vishesh Handa &lt;handa.vish@gmail.com&gt;</p>"
                           "<p>NepSak is a maintenance and debugging tool intended for developers. "
                           "It is NOT intended for the end user.</p>"
                           "<p>NepSak allows to browse, query, and edit your Nepomuk resources. It provides the means "
                           "to modify, delete, and mess up your data in any way possible.</p>"
                           "<p><b>USE WITH CARE</b></p>",
                           KGlobal::mainComponent().aboutData()->version() ),
                      this );
    label->setAlignment( Qt::AlignCenter );
    label->setWordWrap( true );

    lay->addWidget( iconLabel );
    lay->addWidget( label );

    setMainWidget(w);
}


InfoSplash::~InfoSplash()
{
}

#include "infosplash.moc"
