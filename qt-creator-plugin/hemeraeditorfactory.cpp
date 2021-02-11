#include "hemeraeditorfactory.h"
#include "hemeraconstants.h"
#include "hemeraeditor.h"

#include <coreplugin/icore.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/editormanager/editormanager.h>
#include <texteditor/texteditoractionhandler.h>
#include <texteditor/texteditorconstants.h>
#include <texteditor/texteditorsettings.h>

using namespace Hemera;
using namespace Hemera::Internal;

HemeraEditorFactory::HemeraEditorFactory(HemeraProjectManager *manager)
{
    using namespace Core;
    using namespace TextEditor;

    setId(Constants::HEMERA_EDITOR_ID);
    setDisplayName(tr(Constants::HEMERA_EDITOR_DISPLAY_NAME));
    addMimeType(Constants::MIME_TYPE);

    /*setEditorCreator([]() { return new HemeraEditor; });
    setEditorWidgetCreator([]() { return new HemeraEditorWidget; });
    setDocumentCreator([]() { return new HemeraDocument; });
    setGenericSyntaxHighlighter(QLatin1String(Constants::MIME_TYPE));
    setCommentStyle(Utils::CommentDefinition::CppStyle);
    setCodeFoldingSupported(true);*/

    //setCompletionAssistProvider(new HemeraFileCompletionAssistProvider(settingsPage));

    /*setEditorActionHandlers(TextEditorActionHandler::UnCommentSelection
            | TextEditorActionHandler::JumpToFileUnderCursor);*/

    ActionContainer *contextMenu = ActionManager::createMenu(Constants::M_CONTEXT);
    contextMenu->addAction(ActionManager::command(TextEditor::Constants::JUMP_TO_FILE_UNDER_CURSOR));
    contextMenu->addSeparator(Context(Constants::HEMERA_EDITOR_ID));
    contextMenu->addAction(ActionManager::command(TextEditor::Constants::UN_COMMENT_SELECTION));
}
