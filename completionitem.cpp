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

#include "completionitem.h"

#include <QtCore/QSharedData>
#include <QtCore/QVariant>
#include <QtGui/QIcon>


class Nepomuk::CompletionItem::Private : public QSharedData
{
public:
    QString completionText;
    QString text;
    QString toolTip;
    QIcon icon;
    QVariant userData;
};


Nepomuk::CompletionItem::CompletionItem()
    : d( new Private() )
{
}

Nepomuk::CompletionItem::CompletionItem( const CompletionItem& other )
    : d( other.d )
{
}


Nepomuk::CompletionItem::CompletionItem( const QString& cText,
                                         const QString& text,
                                         const QString& desc,
                                         const QIcon& icon,
                                         const QVariant& userData )
    : d( new Private() )
{
    d->completionText = cText;
    d->text = text;
    d->toolTip = desc;
    d->icon = icon;
    d->userData = userData;
}


Nepomuk::CompletionItem::~CompletionItem()
{
}


Nepomuk::CompletionItem& Nepomuk::CompletionItem::operator=( const CompletionItem& other )
{
    d = other.d;
    return *this;
}


QString Nepomuk::CompletionItem::completionText() const
{
    return d->completionText;
}


QString Nepomuk::CompletionItem::text() const
{
    return d->text;
}


QString Nepomuk::CompletionItem::toolTip() const
{
    return d->toolTip;
}


QIcon Nepomuk::CompletionItem::icon() const
{
    return d->icon;
}


QVariant Nepomuk::CompletionItem::userData() const
{
    return d->userData;
}
