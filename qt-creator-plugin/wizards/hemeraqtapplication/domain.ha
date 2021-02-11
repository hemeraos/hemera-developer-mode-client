import com.ispirata.Hemera.Settings 1.0

%{BaseHaClass} {
    applicationId: "%{ApplicationId}"

    name: "%{ApplicationName}"
    description: "%{ApplicationDescription}"
    version: "%{ApplicationVersion}"
    organization: "%{OrganizationName}"

    features: Features.Video | Features.Audio | Features.Network | Features.Hyperspace

    %{SourceFiles}
    %{ResourceFiles}

    qtModules: QtModules.Core | QtModules.DBus | %{AdditionalQtModules}
    hemeraModules: HemeraModules.Core | %{AdditionalHemeraModules}
}
