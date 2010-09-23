/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2010 Vishesh Handa <handa.vish@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "sparqlsyntaxhighlighter.h"
#include <KDebug>

Nepomuk::SparqlSyntaxHighlighter::SparqlSyntaxHighlighter(QTextDocument* parent): QSyntaxHighlighter(parent)
{
    init();
}

void Nepomuk::SparqlSyntaxHighlighter::init()
{
    // Keywords
    QTextCharFormat keywordFormat;
    keywordFormat.setForeground( Qt::darkMagenta );
    keywordFormat.setFontWeight( QFont::Bold );
    QStringList keywords;

    //FIXME: Separate special inbuilt keywords
    keywords << "\\bprefix\\b" << "\\bselect\\b" << "\\bdistinct\\b" << "\\breduced\\b"
             << "\\bconstruct\\b" << "\\bdescribe\\b" << "\\bask\\b" << "\\bfrom\\b"
             << "\\bnamed\\b" << "\\bwhere\\b" << "\\border\\b" << "\\bby\\b" << "\\basc\\b"
             << "\\bdesc\\b" << "\\blimit\\b" << "\\boffset\\b" << "\\boptional\\b"
             << "\\bgraph\\b" << "\\bunion\\b" << "\\bfilter\\b" << "\\bstr\\b"
             << "\\blang\\b" << "\\blangmatches\\b" << "\\bdatatype\\b" << "\\bbound\\b"
             << "\\bsameTerm\\b" << "\\bisIRI\\b" << "\\bisURI\\b" << "\\bisLiteral\\b"
             << "\\bisBlank\\b" << "\\bregex\\b" << "\\btrue\\b" << "\\bfalse\\b" << "\\ba\\b";

    foreach( const QString & s, keywords ) {
        QRegExp regex( s, Qt::CaseInsensitive );
        m_rules.append( Rule( regex, keywordFormat ) );
    }

    // Variables
    QTextCharFormat varFormat;
    varFormat.setForeground( Qt::blue );
    QRegExp varRegex( "\\?\\w+" );
    m_rules.append( Rule( varRegex, varFormat ) );

    // URI
    QTextCharFormat uriFormat;
    uriFormat.setForeground( Qt::darkGreen );
    QRegExp uriRegex( "<.*>" );
    m_rules.append( Rule( uriRegex, uriFormat ) );

    // Abbreviated uris --> uri:word
    //TODO: Highlight uri and word with different colours
    QTextCharFormat abrUriFormat;
    abrUriFormat.setForeground( Qt::darkGray );
    QRegExp abrUriRegex( "\\b\\w+:\\w*\\b" );
    m_rules.append( Rule( abrUriRegex, abrUriFormat ) );
    
    // Literals
    QTextCharFormat literalFormat;
    literalFormat.setForeground( Qt::red );
    QRegExp literalRegex( "\".*\"" );
    m_rules.append( Rule( literalRegex, literalFormat ) );

    // Comments
    QTextCharFormat commentFormat;
    commentFormat.setForeground( Qt::darkYellow );
    QRegExp commentRegex( "^#.*$" );
    m_rules.append( Rule( commentRegex, commentFormat ) );
}

void Nepomuk::SparqlSyntaxHighlighter::highlightBlock(const QString& text)
{
    //kDebug();
    //int i=0;
    foreach (const Rule &rule, m_rules) {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        int length = 0;
        while (index >= 0 && ( length = expression.matchedLength() ) > 0 ) {
            //kDebug() << "Setting " << i;
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
        //i++;
    }
}
