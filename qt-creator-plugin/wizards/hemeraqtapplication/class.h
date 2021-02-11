%{Cpp:LicenseTemplate}\
#ifndef %{JS: Cpp.headerGuard('%{HdrClassFile}')}
#define %{JS: Cpp.headerGuard('%{HdrClassFile}')}

#include <%{HemeraIncludeHeader}>

class %{BaseClassName} : public Hemera::%{HemeraBaseClass}
{
    Q_OBJECT
    Q_DISABLE_COPY(%{BaseClassName})

public:
    explicit %{BaseClassName}();
    virtual ~%{BaseClassName}();

protected:
    virtual void initImpl() override final;
    virtual void startImpl() override final;
    virtual void stopImpl() override final;
    virtual void prepareForShutdown() override final;
};

#endif // %{JS: Cpp.headerGuard('%{HdrClassFile}')}

