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

#ifndef _NEPOMUK_COMPLETION_H_
#define _NEPOMUK_COMPLETION_H_

#include <QtGui/QCompleter>
#include <QtCore/QList>

#include "completionitem.h"

namespace Nepomuk {
    class Completion : public QCompleter
    {
        Q_OBJECT

    public:
        Completion( QObject* parent = 0 );
        virtual ~Completion();

        QList<CompletionItem> allItems() const;

    Q_SIGNALS:
        void activated( const Nepomuk::CompletionItem& );

    public Q_SLOTS:
        /**
         * Set the text to complete. This is independent 
         * from QCompleter::setCompletionPrefix which 
         * does not have to be called anymore.
         */
        void setCompletionText( const QString& );

    protected:
        virtual void makeCompletion( const QString& ) = 0;
        void addCompletion( const CompletionItem& item );

    private:
        class Private;
        Private* const d;

        Q_PRIVATE_SLOT( d, void _k_activated( const QModelIndex& ) )
    };
}

#endif
