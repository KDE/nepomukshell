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

#ifndef _NEPOMUK_FILE_SAVE_DIALOG_
#define _NEPOMUK_FILE_SAVE_DIALOG_

#include <KDialog>

namespace Nepomuk {

    namespace Types {
        class Class;
    }

    class FileDialog : public KDialog
    {
        Q_OBJECT

    public:
        FileDialog( QWidget* parent = 0 );
        ~FileDialog();

        /**
         * Returns the full path of the selected file in the local filesystem.
         * (Local files only)
         */
        KUrl selectedFile() const;

        Nepomuk::Types::Class baseType() const;
        void setBaseType( const Types::Class& type );

        /**
         * Set the file extension to use. This is kept
         * to provide filenames that are backwards
         * compatible when not used with the semantic
         * desktop.
         */
        void setExtension( const QString& ext );

        void saveMetaData();

        /**
         * Creates a model dialog that allows to choose a name
         * and PIMO types for a file to save.
         *
         * The metadata will be saved automatically.
         *
         * \return A unique filename in the user's Documents
         * folder to save the file.
         */
        static KUrl getSaveUrl( const QString& extension,
                                QWidget* parent = 0,
                                const QString& caption = QString() );

    private:
        class Private;
        Private* const d;

        Q_PRIVATE_SLOT( d, void _k_typeActivated( const KCompletionItem& ) )
        Q_PRIVATE_SLOT( d, void _k_dialogFinished() )
    };
}

#endif
