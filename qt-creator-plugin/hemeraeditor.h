#ifndef HEMERAEDITOR_H
#define HEMERAEDITOR_H

#include "hemeraeditorfactory.h"

#include <texteditor/textdocument.h>
#include <texteditor/texteditor.h>
#include <texteditor/codeassist/completionassistprovider.h>
#include <utils/uncommentselection.h>


namespace TextEditor {
class FontSettings;
}

namespace Hemera {
namespace Internal {

class HemeraEditorWidget;
class HemeraHighlighter;
class HemeraProjectManager;

class HemeraEditor : public TextEditor::BaseTextEditor
{
    Q_OBJECT

public:
    HemeraEditor(HemeraEditorWidget *);

    Core::Id id() const;
    TextEditor::CompletionAssistProvider *completionAssistProvider();

private slots:
    void markAsChanged();
    void build();
};

class HemeraEditorWidget : public TextEditor::TextEditorWidget
{
    Q_OBJECT

public:
    HemeraEditorWidget(QWidget *parent, HemeraEditorFactory *factory, TextEditor::TextEditorActionHandler *ah);

    bool save(const QString &fileName = QString());

    HemeraEditorFactory *factory() { return m_factory; }
    TextEditor::TextEditorActionHandler *actionHandler() const { return m_ah; }

    Link findLinkAt(const QTextCursor &cursor, bool resolveTarget = true, bool inNextSplit = false);

protected:
    TextEditor::BaseTextEditor *createEditor();
    void contextMenuEvent(QContextMenuEvent *e);

public slots:
    void unCommentSelection();

private:
    HemeraEditorFactory *m_factory;
    TextEditor::TextEditorActionHandler *m_ah;
    Utils::CommentDefinition m_commentDefinition;
};

class HemeraDocument : public TextEditor::TextDocument
{
    Q_OBJECT

public:
    HemeraDocument();
    QString defaultPath() const;
    QString suggestedFileName() const;
};

} // namespace Internal
} // namespace Hemera

#endif // HEMERAEDITOR_H
