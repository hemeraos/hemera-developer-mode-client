#include "hemerafilecompletionassist.h"
#include "hemeraconstants.h"
#include "hemeraprojectmanager.h"

#include <texteditor/codeassist/keywordscompletionassist.h>

using namespace Hemera::Internal;
using namespace TextEditor;

// -------------------------------
// HemeraFileCompletionAssistProvider
// -------------------------------
HemeraFileCompletionAssistProvider::HemeraFileCompletionAssistProvider(HemeraSettingsPage *settingsPage)
    : m_settingsPage(settingsPage)
{}

HemeraFileCompletionAssistProvider::~HemeraFileCompletionAssistProvider()
{}

bool HemeraFileCompletionAssistProvider::supportsEditor(const Core::Id &editorId) const
{
    return editorId == Hemera::Constants::HEMERA_EDITOR_ID;
}

IAssistProcessor *HemeraFileCompletionAssistProvider::createProcessor() const
{
    return new KeywordsCompletionAssistProcessor(m_settingsPage->keywords());
}
