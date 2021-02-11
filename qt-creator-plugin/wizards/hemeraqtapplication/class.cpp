%{Cpp:LicenseTemplate}\

#include "%{FileClassName}.%{JS: Util.preferredSuffix('text/x-c++hdr')}"
#include "simpleapplicationproperties.h"        // generated @ build time

#include <QtCore/QUrl>

%{BaseClassName}::%{BaseClassName}()
    : Hemera::%{HemeraBaseClass}(new SimpleApplicationProperties%{AdditionalConstructorArguments})
{
}

%{BaseClassName}::~%{BaseClassName}()
{
}

void %{BaseClassName}::initImpl()
{
    setReady();
}

void %{BaseClassName}::startImpl()
{
    setStarted();
}

void %{BaseClassName}::stopImpl()
{
    setStopped();
}

void %{BaseClassName}::prepareForShutdown()
{
    setReadyForShutdown();
}

%{HemeraMainMacro}(%{BaseClassName})
