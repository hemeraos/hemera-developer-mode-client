#ifndef HEMERAFILECOMPLETIONASSIST_H
#define HEMERAFILECOMPLETIONASSIST_H

#include <texteditor/codeassist/completionassistprovider.h>

namespace Hemera {
namespace Internal {

class HemeraSettingsPage;

class HemeraFileCompletionAssistProvider : public TextEditor::CompletionAssistProvider
{
    Q_OBJECT

public:
    HemeraFileCompletionAssistProvider(HemeraSettingsPage *settingsPage);
    ~HemeraFileCompletionAssistProvider();

    bool supportsEditor(const Core::Id &editorId) const;
    TextEditor::IAssistProcessor *createProcessor() const;

private:
    HemeraSettingsPage *m_settingsPage;
};

} // Internal
} // Hemera

#endif // HEMERAFILECOMPLETIONASSIST_H
