#ifndef HEMERAHIGHLIGHTER_H
#define HEMERAHIGHLIGHTER_H

#include <texteditor/syntaxhighlighter.h>

namespace Hemera {
namespace Internal {

/* This is a simple syntax highlighter for Hemera files.
 * It highlights variables, commands, strings and comments.
 * Multi-line strings and variables inside strings are also recognized. */
class HemeraHighlighter : public TextEditor::SyntaxHighlighter
{
    Q_OBJECT
public:
    enum HemeraFormats {
        HemeraVariableFormat,
        HemeraFunctionFormat,
        HemeraCommentFormat,
        HemeraStringFormat,
        HemeraVisualWhiteSpaceFormat
    };

    HemeraHighlighter(QTextDocument *document = 0);
    virtual void highlightBlock(const QString &text);
};

} // namespace Internal
} // namespace Hemera

#endif // PROFILEHIGHLIGHTER_H
