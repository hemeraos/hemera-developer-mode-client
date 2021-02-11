#ifndef HEMERAEDITORFACTORY_H
#define HEMERAEDITORFACTORY_H

#include "hemeraprojectmanager.h"

#include <coreplugin/editormanager/ieditorfactory.h>

namespace TextEditor { class TextEditorActionHandler; }

namespace Hemera {
namespace Internal {

class HemeraEditorFactory : public Core::IEditorFactory
{
    Q_OBJECT

public:
    HemeraEditorFactory(HemeraProjectManager *parent);
};

} // namespace Internal
} // namespace Hemera

#endif // HEMERAEDITORFACTORY_H
