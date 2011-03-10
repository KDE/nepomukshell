/*
   Copyright (c) 2010-11 Vishesh Handa <handa.vish@gmail.com>

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


#ifndef QUERYEDITOR_H
#define QUERYEDITOR_H

#include <QtGui/QTextEdit>
#include <QtGui/QCompleter>

namespace Nepomuk {
    class SparqlSyntaxHighlighter;
}

class QueryEditor : public QTextEdit
{
    Q_OBJECT
public:
    explicit QueryEditor(QWidget* parent = 0);

    virtual void keyPressEvent(QKeyEvent* e);

private slots:
    void insertCompletion( const QString & text );
    
private:
    QCompleter * m_completer;
    Nepomuk::SparqlSyntaxHighlighter* m_highlighter;

    QString wordUnderCursor();
};

#endif // QUERYEDITOR_H
