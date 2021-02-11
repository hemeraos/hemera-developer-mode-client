#include "hemerahighlighter.h"

#include <QRegExp>
#include <QTextDocument>

using namespace Hemera::Internal;


static bool isVariable(const QByteArray &word)
{
    if (word.length() < 4) // must be at least "${.}"
        return false;
    return word.startsWith("${") && word.endsWith('}');
}


HemeraHighlighter::HemeraHighlighter(QTextDocument *document) :
    TextEditor::SyntaxHighlighter(document)
{
    static QVector<TextEditor::TextStyle> categories;
    if (categories.isEmpty()) {
        categories << TextEditor::C_LABEL  // variables
                   << TextEditor::C_KEYWORD   // functions
                   << TextEditor::C_COMMENT
                   << TextEditor::C_STRING
                   << TextEditor::C_VISUAL_WHITESPACE;
    }
    setTextFormatCategories(categories);
}


void HemeraHighlighter::highlightBlock(const QString &text)
{
    QByteArray buf;
    bool inCommentMode = false;
    bool inStringMode = (previousBlockState() == 1);

    QTextCharFormat emptyFormat;
    int i=0;
    for (i=0; i < text.length(); i++) {
        char c = text.at(i).toLatin1();
        if (inCommentMode) {
            setFormat(i, 1, formatForCategory(HemeraCommentFormat));
        } else {
            if (c == '#') {
                if (!inStringMode) {
                    inCommentMode = true;
                    setFormat(i, 1, formatForCategory(HemeraCommentFormat));
                    buf.clear();
                } else {
                    buf += c;
                }
            } else if (c == '(') {
                if (!inStringMode) {
                    if (!buf.isEmpty())
                        setFormat(i - buf.length(), buf.length(), formatForCategory(HemeraFunctionFormat));
                    buf.clear();
                } else {
                    buf += c;
                }
            } else if (text.at(i).isSpace()) {
                if (!inStringMode)
                    buf.clear();
                else
                    buf += c;
            } else if (c == '\"') {
                buf += c;
                if (inStringMode) {
                    setFormat(i + 1 - buf.length(), buf.length(), formatForCategory(HemeraStringFormat));
                    buf.clear();
                } else {
                    setFormat(i, 1, formatForCategory(HemeraStringFormat));
                }
                inStringMode = !inStringMode;
            } else if (c == '\\') {
                setFormat(i, 1, emptyFormat);
                buf += c;
                i++;
                if (i < text.length()) {
                    text.at(i);
                    setFormat(i, 1, emptyFormat);
                    buf += c;
                }
            } else if (c == '$') {
                if (inStringMode)
                    setFormat(i - buf.length(), buf.length(), formatForCategory(HemeraStringFormat));
                buf.clear();
                buf += c;
                setFormat(i, 1, emptyFormat);
            } else if (c == '}') {
                buf += c;
                if (isVariable(buf)) {
                    setFormat(i + 1 - buf.length(), buf.length(), formatForCategory(HemeraVariableFormat));
                    buf.clear();
                }
            } else {
                buf += c;
                setFormat(i, 1, emptyFormat);
            }
        }
    }

    if (inStringMode) {
        setFormat(i - buf.length(), buf.length(), formatForCategory(HemeraStringFormat));
        setCurrentBlockState(1);
    } else {
        setCurrentBlockState(0);
    }

    applyFormatToSpaces(text, formatForCategory(HemeraVisualWhiteSpaceFormat));
}

