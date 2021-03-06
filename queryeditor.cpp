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


#include "queryeditor.h"
#include "sparqlsyntaxhighlighter.h"

#include <QtGui/QStringListModel>
#include <QtGui/QCursor>
#include <QtGui/QKeyEvent>
#include <QtGui/QScrollBar>

#include <KGlobalSettings>
#include <KDebug>

#include <Soprano/Model>
#include <Soprano/QueryResultIterator>

#include <Nepomuk2/ResourceManager>

QueryEditor::QueryEditor(QWidget* parent): KTextEdit(parent)
{
    setFont( KGlobalSettings::fixedFont() );
    setAcceptRichText( false );
    setCheckSpellingEnabled( false );
    setFocus();

    m_highlighter = new Nepomuk2::SparqlSyntaxHighlighter( document() );

    //
    // Completion
    //
    QString query = QString::fromLatin1("select ?pre ?r where { graph ?g {?r a ?t.} ?g nao:hasDefaultNamespaceAbbreviation ?pre. }");

    Soprano::Model * model = Nepomuk2::ResourceManager::instance()->mainModel();
    Soprano::QueryResultIterator it = model->executeQuery( query, Soprano::Query::QueryLanguageSparql );

    QStringList candidates;
    while( it.next() ) {
        QString prefix = it[QLatin1String("pre")].toString();
        QString res = it[QLatin1String("r")].uri().toString();
        res = res.mid( res.lastIndexOf(QLatin1String( "#" )) + 1 );

        candidates << QString( prefix + QLatin1String( ":" ) + res );
    }

    // Keywords
    candidates << QLatin1String( "prefix" ) << QLatin1String( "select" ) << QLatin1String( "distinct" ) << QLatin1String( "reduced" )
               << QLatin1String( "construct" ) << QLatin1String( "describe" ) << QLatin1String( "ask" ) << QLatin1String( "from" )
               << QLatin1String( "named" ) << QLatin1String( "where" ) << QLatin1String( "order" ) << QLatin1String( "by" ) << QLatin1String( "asc" )
               << QLatin1String( "desc" ) << QLatin1String( "limit" ) << QLatin1String( "offset" ) << QLatin1String( "optional" )
               << QLatin1String( "graph" ) << QLatin1String( "union" ) << QLatin1String( "filter" ) << QLatin1String( "str" )
               << QLatin1String( "lang" ) << QLatin1String( "langmatches" ) << QLatin1String( "datatype" ) << QLatin1String( "bound" )
               << QLatin1String( "sameTerm" ) << QLatin1String( "isIRI" ) << QLatin1String( "isURI" ) << QLatin1String( "isLiteral" )
               << QLatin1String( "isBlank" ) << QLatin1String( "regex" ) << QLatin1String( "true" ) << QLatin1String( "false" );

    m_completer = new QCompleter( this );
    m_completer->setModel( new QStringListModel( candidates, m_completer ) );
    m_completer->setModelSorting( QCompleter::CaseSensitivelySortedModel );
    m_completer->setCaseSensitivity( Qt::CaseInsensitive );

    m_completer->setWidget( this );
    m_completer->setCompletionMode( QCompleter::PopupCompletion );

    connect( m_completer, SIGNAL(activated(QString)), this, SLOT(insertCompletion(QString)) );
}

void QueryEditor::keyPressEvent(QKeyEvent* e)
{
    if( m_completer->popup()->isVisible() ) {
        switch (e->key()) {
            case Qt::Key_Enter:
            case Qt::Key_Return:
            case Qt::Key_Escape:
            case Qt::Key_Tab:
            case Qt::Key_Backtab:
                e->ignore();
                return; // let the completer do default behavior
            default:
                break;
        }
    }
    QTextEdit::keyPressEvent(e);

    QString text = wordUnderCursor();
    //Minimum 2 characters to show completion list. Hide list when moving around between text.
    if (e->text().isEmpty()|| text.length() < 2) {
        m_completer->popup()->hide();
        return;
    }
    // Select first completion as default completion in the list.
    if (text != m_completer->completionPrefix()) {
        m_completer->setCompletionPrefix(text);
        m_completer->popup()->setCurrentIndex(m_completer->completionModel()->index(0, 0));
    }

    QRect cr = cursorRect();
    cr.setWidth( m_completer->popup()->sizeHintForColumn(0)
                 + m_completer->popup()->verticalScrollBar()->sizeHint().width() );
    m_completer->complete( cr );
}


QString QueryEditor::wordUnderCursor()
{
    static QString eow = QLatin1String( "~!@#$%^&*()+{}|\"<>,./;'[]\\-= " ); // everything without ':', '?' and '_'
    QTextCursor tc = textCursor();

    tc.anchor();
    while( 1 ) {
        // vHanda: I don't understand why the cursor seems to give a pos 1 past the last char instead
        // of just the last char.
        int pos = tc.position() - 1;
        if( pos < 0 || eow.contains( document()->characterAt(pos) ) )
            break;
        tc.movePosition( QTextCursor::Left, QTextCursor::KeepAnchor );
    }
    return tc.selectedText();
}

void QueryEditor::insertCompletion(const QString& completion)
{
    QTextCursor tc = textCursor();
    int extra = completion.length() - m_completer->completionPrefix().length();
    tc.movePosition(QTextCursor::Left);
    tc.movePosition(QTextCursor::EndOfWord);
    tc.insertText(completion.right(extra));
    setTextCursor(tc);
}



