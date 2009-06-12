/*
   Copyright (C) 2008-2009 by Sebastian Trueg <trueg at kde.org>

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

#ifndef _NEPOMUK_TYPE_COMPLETION_H_
#define _NEPOMUK_TYPE_COMPLETION_H_

#include "kcompleter.h"

namespace Nepomuk {
    namespace Types {
        class Class;
    }

    class TypeCompletion : public KCompleter
    {
        Q_OBJECT

    public:
        TypeCompletion( QObject* parent = 0 );
        ~TypeCompletion();

        Types::Class baseType() const;

    public Q_SLOTS:
        void setBaseType( const Nepomuk::Types::Class& basetype );

    protected:
        void makeCompletion( const QString& string );

    private:
        class Private;
        Private* const d;
    };
}

#endif
