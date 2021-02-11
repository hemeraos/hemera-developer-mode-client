#include "hemeraeditor.h"

#include "hemerafilecompletionassist.h"
#include "hemerahighlighter.h"
#include "hemeraeditorfactory.h"
#include "hemeraconstants.h"
#include "hemeraproject.h"

#include <coreplugin/icore.h>
#include <coreplugin/infobar.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <extensionsystem/pluginmanager.h>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/session.h>
#include <texteditor/texteditoractionhandler.h>
#include <texteditor/texteditorconstants.h>
#include <texteditor/texteditorsettings.h>

#include <QFileInfo>
#include <QSharedPointer>
#include <QTextBlock>

using namespace Hemera;
using namespace Hemera::Internal;

//
// ProFileEditorEditable
//

HemeraEditor::HemeraEditor(HemeraEditorWidget *editor)
  : BaseTextEditor()
{
    setContext(Core::Context(Hemera::Constants::C_HEMERAEDITOR,
              TextEditor::Constants::C_TEXTEDITOR));
    connect(document(), SIGNAL(changed()), this, SLOT(markAsChanged()));
}

Core::Id HemeraEditor::id() const
{
    return Core::Id(Hemera::Constants::HEMERA_EDITOR_ID);
}

TextEditor::CompletionAssistProvider *HemeraEditor::completionAssistProvider()
{
    return ExtensionSystem::PluginManager::getObject<HemeraFileCompletionAssistProvider>();
}

void HemeraEditor::markAsChanged()
{
    if (!document()->isModified())
        return;
    Core::InfoBar *infoBar = document()->infoBar();
    Core::Id infoRunCmake("HemeraEditor.RunHemera");
    if (!infoBar->canInfoBeAdded(infoRunCmake))
        return;
    Core::InfoBarEntry info(infoRunCmake,
                            tr("Changes to hemera files are shown in the project tree after building."),
                            Core::InfoBarEntry::GlobalSuppressionEnabled);
    //info.setCustomButtonInfo(tr("Build now"), this, SLOT(build()));
    infoBar->addInfo(info);
}

void HemeraEditor::build()
{
    foreach (ProjectExplorer::Project *p, ProjectExplorer::SessionManager::projects()) {
        HemeraProject *hemeraProject = qobject_cast<HemeraProject *>(p);
        if (hemeraProject) {
            if (hemeraProject->isProjectFile(document()->filePath())) {
                ProjectExplorer::ProjectExplorerPlugin::instance()->buildProject(hemeraProject);
                break;
            }
        }
    }
}

//
// HemeraEditor
//

HemeraEditorWidget::HemeraEditorWidget(QWidget *parent, HemeraEditorFactory *factory, TextEditor::TextEditorActionHandler *ah)
    : TextEditorWidget(parent), m_factory(factory), m_ah(ah)
{
    QSharedPointer<HemeraDocument> doc(new HemeraDocument);
    doc->setMimeType(QLatin1String(Hemera::Constants::MIME_TYPE));
    setTextDocument(doc);

    textDocument()->setSyntaxHighlighter(new HemeraHighlighter);

    m_commentDefinition.setStyle(Utils::CommentDefinition::CppStyle);

    //ah->setupActions(this);
}

TextEditor::BaseTextEditor *HemeraEditorWidget::createEditor()
{
    return new HemeraEditor(this);
}

void HemeraEditorWidget::unCommentSelection()
{
    Utils::unCommentSelection(this, m_commentDefinition);
}

void HemeraEditorWidget::contextMenuEvent(QContextMenuEvent *e)
{
    showDefaultContextMenu(e, Constants::M_CONTEXT);
}

static bool isValidFileNameChar(const QChar &c)
{
    if (c.isLetterOrNumber()
            || c == QLatin1Char('.')
            || c == QLatin1Char('_')
            || c == QLatin1Char('-')
            || c == QLatin1Char('/')
            || c == QLatin1Char('\\'))
        return true;
    return false;
}

HemeraEditorWidget::Link HemeraEditorWidget::findLinkAt(const QTextCursor &cursor,
                                                      bool/* resolveTarget*/, bool /*inNextSplit*/)
{
    Link link;

    int lineNumber = 0, positionInBlock = 0;
    convertPosition(cursor.position(), &lineNumber, &positionInBlock);

    const QString block = cursor.block().text();

    // check if the current position is commented out
    const int hashPos = block.indexOf(QLatin1Char('#'));
    if (hashPos >= 0 && hashPos < positionInBlock)
        return link;

    // find the beginning of a filename
    QString buffer;
    int beginPos = positionInBlock - 1;
    while (beginPos >= 0) {
        QChar c = block.at(beginPos);
        if (isValidFileNameChar(c)) {
            buffer.prepend(c);
            beginPos--;
        } else {
            break;
        }
    }

    // find the end of a filename
    int endPos = positionInBlock;
    while (endPos < block.count()) {
        QChar c = block.at(endPos);
        if (isValidFileNameChar(c)) {
            buffer.append(c);
            endPos++;
        } else {
            break;
        }
    }

    if (buffer.isEmpty())
        return link;

    // TODO: Resolve variables

    QDir dir(QFileInfo(document()->baseUrl().toLocalFile()).absolutePath());
    QString fileName = dir.filePath(buffer);
    QFileInfo fi(fileName);
    if (fi.exists()) {
        if (fi.isDir()) {
            QDir subDir(fi.absoluteFilePath());
            QString subProject = subDir.filePath(QLatin1String("HemeraLists.txt"));
            if (QFileInfo(subProject).exists())
                fileName = subProject;
            else
                return link;
        }
        link.targetFileName = fileName;
        link.linkTextStart = cursor.position() - positionInBlock + beginPos + 1;
        link.linkTextEnd = cursor.position() - positionInBlock + endPos;
    }
    return link;
}


//
// HemeraDocument
//

HemeraDocument::HemeraDocument()
    : TextDocument()
{
}

QString HemeraDocument::defaultPath() const
{
    QFileInfo fi(filePath());
    return fi.absolutePath();
}

QString HemeraDocument::suggestedFileName() const
{
    QFileInfo fi(filePath());
    return fi.fileName();
}
