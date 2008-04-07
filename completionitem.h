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

#ifndef _NEPOMUK_COMPLETION_ITEM_H_
#define _NEPOMUK_COMPLETION_ITEM_H_

#include <QtCore/QSharedDataPointer>
#include <QtCore/QVariant>

class QString;
class QIcon;


namespace Nepomuk {
    class CompletionItem
    {
    public:
        CompletionItem();
        CompletionItem( const CompletionItem& );
        CompletionItem( const QString& completionText,
                        const QString& text,
                        const QString& desc,
                        const QIcon& icon,
                        const QVariant& userData = QVariant() );
        ~CompletionItem();

        CompletionItem& operator=( const CompletionItem& );

        QString completionText() const;
        QString text() const;
        QString toolTip() const;
        QIcon icon() const;
        QVariant userData() const;

    private:
        class Private;
        QSharedDataPointer<Private> d;
    };
}

#endif
