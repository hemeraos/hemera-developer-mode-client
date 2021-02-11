<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1">
<context>
    <name>BaseCommand</name>
    <message>
        <location filename="../hsdk/basecommand.cpp" line="165"/>
        <source>Unknown Hemera target &quot;%1&quot;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../hsdk/basecommand.cpp" line="171"/>
        <location filename="../hsdk/basecommand.cpp" line="216"/>
        <source>Target &quot;%1&quot; appears to be offline, or doesn&apos;t have DeviceInfo available.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../hsdk/basecommand.cpp" line="181"/>
        <source>Specified orbit handler &apos;%1&apos; does not exist on target &apos;%2&apos;. Available handlers are: %3.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../hsdk/basecommand.cpp" line="189"/>
        <source>Internal error while retrieving orbit handler &apos;%1&apos; on target &apos;%2&apos;. The handler might not exist.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../hsdk/basecommand.cpp" line="195"/>
        <source>Could not retrieve a valid orbit handler &apos;%1&apos; for target &apos;%2&apos;.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../hsdk/basecommand.cpp" line="221"/>
        <source>Target &quot;%1&quot; appears to be a production board, developer mode won&apos;t be available here.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../hsdk/basecommand.cpp" line="226"/>
        <source>Could not retrieve developer mode controller for target &apos;%1&apos;.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ConfigureDevices</name>
    <message>
        <location filename="../ui/configuredevices.ui" line="17"/>
        <source>Hemera Static Targets</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/configuredevices.ui" line="34"/>
        <source>&amp;Add</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/configuredevices.ui" line="45"/>
        <source>&amp;Edit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/configuredevices.ui" line="56"/>
        <source>&amp;Remove</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../developer-mode-client/configuredevices.cpp" line="64"/>
        <location filename="../developer-mode-client/configuredevices.cpp" line="85"/>
        <source>Warning</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../developer-mode-client/configuredevices.cpp" line="64"/>
        <location filename="../developer-mode-client/configuredevices.cpp" line="85"/>
        <source>There is already a target with the name &quot;%1&quot;.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ConfigureTarget</name>
    <message>
        <location filename="../ui/configuretarget.ui" line="17"/>
        <source>Hemera Target</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/configuretarget.ui" line="34"/>
        <source>Basic Configuration</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/configuretarget.ui" line="43"/>
        <source>&amp;Name:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/configuretarget.ui" line="53"/>
        <source>Identifier for your target.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/configuretarget.ui" line="60"/>
        <source>Hyperspace &amp;Protocol:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/configuretarget.ui" line="71"/>
        <source>TCP/IP</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/configuretarget.ui" line="76"/>
        <source>Bluetooth</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/configuretarget.ui" line="85"/>
        <source>Hyperspace TCP/IP Configuration</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/configuretarget.ui" line="94"/>
        <source>Hyperspace &amp;Host:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/configuretarget.ui" line="104"/>
        <source>The host of your target (IP, etc...)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/configuretarget.ui" line="111"/>
        <source>Hyperspace &amp;Port:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/configuretarget.ui" line="129"/>
        <source>Hyperspace Bluetooth Configuration</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/configuretarget.ui" line="135"/>
        <source>Bluetooth device:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>DeployCommand</name>
    <message>
        <location filename="../hsdk/deploycommand.cpp" line="24"/>
        <source>Operation timed out, exiting.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>LaunchCommand</name>
    <message>
        <location filename="../hsdk/launchcommand.cpp" line="32"/>
        <source>Operation timed out, exiting.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../hsdk/launchcommand.cpp" line="41"/>
        <source>Launches a standalone application onto a remote target</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../hsdk/launchcommand.cpp" line="46"/>
        <source>Launches an orbital Hemera application through developer mode onto a specific target.
If the application is launched in the standard mode, the application output is displayed in the terminal. If launched in debug mode, it enters in gdb console. To terminate the application/developer mode instance, just CTRL+C the command.

Note: do not attempt to use this command on a production board! This command relies on Hemera&apos;s Developer Mode, and there&apos;s no standard way to force an application onto a Hemera production target</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../hsdk/launchcommand.cpp" line="66"/>
        <source>No target specified. Use --help flag for usage info.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../hsdk/launchcommand.cpp" line="78"/>
        <source>Missing required parameters. Use --help flag for usage info.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../hsdk/launchcommand.cpp" line="82"/>
        <source>Too many parameters. Use --help flag for usage info.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../hsdk/launchcommand.cpp" line="90"/>
        <source>Invalid mode %1. Available debug modes are: %2.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../hsdk/launchcommand.cpp" line="101"/>
        <source>Application &apos;%1&apos; does not exist on target &apos;%2&apos;.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../hsdk/launchcommand.cpp" line="107"/>
        <source>No orbit handler specified, and target &apos;%1&apos; has more than one. Available handlers are: %2.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../hsdk/launchcommand.cpp" line="148"/>
        <source>Developer mode started, application running.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../hsdk/launchcommand.cpp" line="155"/>
        <source>Gravity could not load Orbital Application &apos;%1&apos;.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../hsdk/launchcommand.cpp" line="159"/>
        <source>Developer mode terminated successfully.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../hsdk/launchcommand.cpp" line="185"/>
        <source>Starting application &apos;%1&apos; on target &apos;%2&apos;, Orbit Handler &apos;%3&apos;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../hsdk/launchcommand.cpp" line="205"/>
        <source>Terminating developer mode...</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="../ui/mainwindow.ui" line="32"/>
        <source>Hemera Developer Mode Client</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/mainwindow.ui" line="43"/>
        <source>Target</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/mainwindow.ui" line="76"/>
        <source>Application Development</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/mainwindow.ui" line="85"/>
        <source>Application:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/mainwindow.ui" line="98"/>
        <source>Applications:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/mainwindow.ui" line="118"/>
        <source>Features:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/mainwindow.ui" line="135"/>
        <source>Advanced Mode</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/mainwindow.ui" line="168"/>
        <source>Orbit Handler:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/mainwindow.ui" line="178"/>
        <location filename="../developer-mode-client/mainwindow.cpp" line="257"/>
        <source>Start</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/mainwindow.ui" line="186"/>
        <source>Device Information</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/mainwindow.ui" line="192"/>
        <source>Board Info</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/mainwindow.ui" line="201"/>
        <source>Status:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/mainwindow.ui" line="208"/>
        <source>UNKNOWN</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/mainwindow.ui" line="221"/>
        <source>ID:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/mainwindow.ui" line="231"/>
        <location filename="../ui/mainwindow.ui" line="254"/>
        <location filename="../ui/mainwindow.ui" line="268"/>
        <location filename="../ui/mainwindow.ui" line="291"/>
        <location filename="../ui/mainwindow.ui" line="323"/>
        <location filename="../ui/mainwindow.ui" line="346"/>
        <location filename="../ui/mainwindow.ui" line="369"/>
        <location filename="../ui/mainwindow.ui" line="392"/>
        <source>TextLabel</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/mainwindow.ui" line="244"/>
        <source>Appliance Name:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/mainwindow.ui" line="261"/>
        <source>Hardware Type:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/mainwindow.ui" line="281"/>
        <source>Development Board:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/mainwindow.ui" line="301"/>
        <source>System Info</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/mainwindow.ui" line="313"/>
        <source>CPU(s):</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/mainwindow.ui" line="336"/>
        <source>Architecture:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/mainwindow.ui" line="359"/>
        <source>Memory:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/mainwindow.ui" line="382"/>
        <source>Release:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/mainwindow.ui" line="432"/>
        <source>&amp;File</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/mainwindow.ui" line="438"/>
        <source>&amp;Help</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/mainwindow.ui" line="451"/>
        <source>&amp;About Hemera Developer Mode Client</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/mainwindow.ui" line="460"/>
        <source>&amp;Quit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/mainwindow.ui" line="463"/>
        <source>Ctrl+Q</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../developer-mode-client/mainwindow.cpp" line="73"/>
        <source>Audio</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../developer-mode-client/mainwindow.cpp" line="76"/>
        <source>Video</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../developer-mode-client/mainwindow.cpp" line="79"/>
        <source>Serial Ports</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../developer-mode-client/mainwindow.cpp" line="82"/>
        <source>Console</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../developer-mode-client/mainwindow.cpp" line="85"/>
        <source>Printers</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../developer-mode-client/mainwindow.cpp" line="88"/>
        <source>Disks</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../developer-mode-client/mainwindow.cpp" line="91"/>
        <source>Hyperspace</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../developer-mode-client/mainwindow.cpp" line="94"/>
        <source>Network</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../developer-mode-client/mainwindow.cpp" line="98"/>
        <source>Check for Updates</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../developer-mode-client/mainwindow.cpp" line="101"/>
        <source>Download Updates</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../developer-mode-client/mainwindow.cpp" line="104"/>
        <source>Update Applications</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../developer-mode-client/mainwindow.cpp" line="107"/>
        <source>Install Applications</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../developer-mode-client/mainwindow.cpp" line="110"/>
        <source>Remove Applications</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../developer-mode-client/mainwindow.cpp" line="113"/>
        <source>Manage Software Repositories</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../developer-mode-client/mainwindow.cpp" line="116"/>
        <source>Update System</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../developer-mode-client/mainwindow.cpp" line="120"/>
        <source>Software Keyboard</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../developer-mode-client/mainwindow.cpp" line="124"/>
        <source>Legacy Devices</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../developer-mode-client/mainwindow.cpp" line="145"/>
        <source>&lt;h3&gt;Hemera Developer Mode Client %1.%2.%3&lt;/h3&gt;&lt;p&gt;Copyright 2013-2014 Ispirata S.r.l.. All rights reserved.&lt;/p&gt;&lt;p&gt;Hemera Developer Mode Client is part of the &lt;a href=&quot;%4&quot;&gt;Hemera&lt;/a&gt; SDK. See the &lt;a href=&quot;%5&quot;&gt;documentation&lt;/a&gt; for more information.&lt;/p&gt;&lt;p&gt;The program is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.&lt;/p&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../developer-mode-client/mainwindow.cpp" line="157"/>
        <source>About Hemera Developer Mode Client</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../developer-mode-client/mainwindow.cpp" line="257"/>
        <source>Stop</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../developer-mode-client/mainwindow.cpp" line="265"/>
        <source>Switching...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../developer-mode-client/mainwindow.cpp" line="306"/>
        <source>No</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../developer-mode-client/mainwindow.cpp" line="306"/>
        <source>Yes</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>OrbitHandlerInfoWidget</name>
    <message>
        <location filename="../ui/orbithandlerinfowidget.ui" line="15"/>
        <source>Type:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/orbithandlerinfowidget.ui" line="29"/>
        <source>Status:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/orbithandlerinfowidget.ui" line="43"/>
        <source>Active Orbit:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/orbithandlerinfowidget.ui" line="57"/>
        <source>Resident Orbit:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/orbithandlerinfowidget.ui" line="71"/>
        <source>Orbit switch Inhibition:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../ui/orbithandlerinfowidget.ui" line="85"/>
        <source>Additional Parameters:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../developer-mode-client/orbithandlerinfowidget.cpp" line="31"/>
        <source>X11 (X.org) Display Server</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../developer-mode-client/orbithandlerinfowidget.cpp" line="34"/>
        <source>EGL Full Screen (framebuffer)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../developer-mode-client/orbithandlerinfowidget.cpp" line="37"/>
        <source>Wayland Compositor</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../developer-mode-client/orbithandlerinfowidget.cpp" line="40"/>
        <source>Linux direct framebuffer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../developer-mode-client/orbithandlerinfowidget.cpp" line="43"/>
        <source>DirectFB framebuffer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../developer-mode-client/orbithandlerinfowidget.cpp" line="46"/>
        <source>Headless</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../developer-mode-client/orbithandlerinfowidget.cpp" line="53"/>
        <source>Inactive or not initialized</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../developer-mode-client/orbithandlerinfowidget.cpp" line="56"/>
        <source>Stable</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../developer-mode-client/orbithandlerinfowidget.cpp" line="59"/>
        <source>Orbit Injected (Developer mode likely active)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../developer-mode-client/orbithandlerinfowidget.cpp" line="62"/>
        <source>Unstable</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../developer-mode-client/orbithandlerinfowidget.cpp" line="66"/>
        <source>Unknown status</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../developer-mode-client/orbithandlerinfowidget.cpp" line="70"/>
        <source>Offline or not reachable</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../developer-mode-client/orbithandlerinfowidget.cpp" line="75"/>
        <source>Not available</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../developer-mode-client/orbithandlerinfowidget.cpp" line="80"/>
        <source>Active, triggered by applications: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../developer-mode-client/orbithandlerinfowidget.cpp" line="83"/>
        <source>Not active</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>OutputCommand</name>
    <message>
        <location filename="../hsdk/outputcommand.cpp" line="20"/>
        <location filename="../hsdk/outputcommand.cpp" line="25"/>
        <source>Print output of a running Hemera application.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../hsdk/outputcommand.cpp" line="49"/>
        <source>No target specified. Use --help flag for usage info.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../hsdk/outputcommand.cpp" line="77"/>
        <source>ID of the Application to monitor.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QSsh::Internal::SftpChannelPrivate</name>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sftpchannel.cpp" line="304"/>
        <source>Server could not start SFTP subsystem.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sftpchannel.cpp" line="372"/>
        <source>Unexpected packet of type %1.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sftpchannel.cpp" line="389"/>
        <source>Protocol version mismatch: Expected %1, got %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sftpchannel.cpp" line="507"/>
        <source>Unknown error.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sftpchannel.cpp" line="531"/>
        <source>Created remote directory &apos;%1&apos;.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sftpchannel.cpp" line="534"/>
        <source>Remote directory &apos;%1&apos; already exists.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sftpchannel.cpp" line="538"/>
        <source>Error creating directory &apos;%1&apos;: %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sftpchannel.cpp" line="562"/>
        <source>Could not open local file &apos;%1&apos;: %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sftpchannel.cpp" line="595"/>
        <source>Remote directory could not be opened for reading.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sftpchannel.cpp" line="601"/>
        <source>Failed to list remote directory contents.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sftpchannel.cpp" line="614"/>
        <source>Failed to close remote directory.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sftpchannel.cpp" line="654"/>
        <source>Failed to open remote file for reading.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sftpchannel.cpp" line="660"/>
        <source>Failed to retrieve information on the remote file (&apos;stat&apos; failed).</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sftpchannel.cpp" line="666"/>
        <source>Failed to read remote file.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sftpchannel.cpp" line="684"/>
        <location filename="../3rdparty/qssh/src/libs/ssh/sftpchannel.cpp" line="755"/>
        <source>Failed to close remote file.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sftpchannel.cpp" line="715"/>
        <source>Failed to open remote file for writing.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sftpchannel.cpp" line="733"/>
        <source>Failed to write remote file.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sftpchannel.cpp" line="826"/>
        <source>Cannot open file </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sftpchannel.cpp" line="900"/>
        <source>Cannot append to remote file: Server does not support the file size attribute.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sftpchannel.cpp" line="934"/>
        <source>Cannot create directory </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sftpchannel.cpp" line="964"/>
        <source>SFTP channel closed unexpectedly.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sftpchannel.cpp" line="986"/>
        <source>Server could not start session: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sftpchannel.cpp" line="1096"/>
        <source>Error reading local file: %1</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QSsh::Internal::SshChannelManager</name>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sshchannelmanager.cpp" line="138"/>
        <source>Invalid channel id %1</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QSsh::Internal::SshConnectionPrivate</name>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sshconnection.cpp" line="329"/>
        <source>SSH Protocol error: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sshconnection.cpp" line="335"/>
        <source>Botan library exception: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sshconnection.cpp" line="360"/>
        <source>Server identification string is %1 characters long, but the maximum allowed length is 255.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sshconnection.cpp" line="373"/>
        <source>Server identification string contains illegal NUL character.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sshconnection.cpp" line="383"/>
        <source>Server Identification string &apos;%1&apos; is invalid.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sshconnection.cpp" line="390"/>
        <source>Server protocol version is &apos;%1&apos;, but needs to be 2.0 or 1.99.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sshconnection.cpp" line="398"/>
        <source>Server identification string is invalid (missing carriage return).</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sshconnection.cpp" line="404"/>
        <source>Server reports protocol version 1.99, but sends data before the identification string, which is not allowed.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sshconnection.cpp" line="441"/>
        <location filename="../3rdparty/qssh/src/libs/ssh/sshconnection.cpp" line="452"/>
        <location filename="../3rdparty/qssh/src/libs/ssh/sshconnection.cpp" line="476"/>
        <location filename="../3rdparty/qssh/src/libs/ssh/sshconnection.cpp" line="490"/>
        <source>Unexpected packet of type %1.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sshconnection.cpp" line="523"/>
        <source>Password expired.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sshconnection.cpp" line="550"/>
        <source>Server rejected password.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sshconnection.cpp" line="550"/>
        <source>Server rejected key.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sshconnection.cpp" line="565"/>
        <source>The server sent an unexpected SSH packet of type SSH_MSG_UNIMPLEMENTED.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sshconnection.cpp" line="632"/>
        <source>Server closed connection: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sshconnection.cpp" line="647"/>
        <source>Connection closed unexpectedly.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sshconnection.cpp" line="661"/>
        <source>Timeout waiting for reply from server.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sshconnection.cpp" line="751"/>
        <source>No private key file given.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sshconnection.cpp" line="755"/>
        <source>Private key file error: %1</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QSsh::Internal::SshRemoteProcessPrivate</name>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sshremoteprocess.cpp" line="372"/>
        <source>Process killed by signal</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sshremoteprocess.cpp" line="378"/>
        <source>Server sent invalid signal &apos;%1&apos;</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QSsh::SftpFileSystemModel</name>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sftpfilesystemmodel.cpp" line="201"/>
        <source>File Type</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sftpfilesystemmodel.cpp" line="203"/>
        <source>File Name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sftpfilesystemmodel.cpp" line="362"/>
        <source>Error getting &apos;stat&apos; info about &apos;%1&apos;: %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sftpfilesystemmodel.cpp" line="372"/>
        <source>Error listing contents of directory &apos;%1&apos;: %2</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QSsh::Ssh</name>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sshkeypasswordretriever.cpp" line="48"/>
        <source>Password Required</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sshkeypasswordretriever.cpp" line="49"/>
        <source>Please enter the password for your private key.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>SimpleCommands</name>
    <message>
        <location filename="../hsdk/hsdkcommand.h" line="36"/>
        <source>Wipe all build files from a Hemera target.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../hsdk/hsdkcommand.h" line="37"/>
        <source>Wipe all build files from a Hemera target.
The target must be a Hemera SDK VM for this command to work.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../hsdk/hsdkcommand.h" line="39"/>
        <location filename="../hsdk/hsdkcommand.h" line="40"/>
        <source>Generates support files for a Hemera project.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../hsdk/hsdkcommand.h" line="44"/>
        <location filename="../hsdk/hsdkcommand.h" line="45"/>
        <source>Call &apos;cmake&apos; on a Hemera project.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../hsdk/hsdkcommand.h" line="46"/>
        <source>Performs a full build on a Hemera project.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../hsdk/hsdkcommand.h" line="47"/>
        <source>Performs a full build on a Hemera project.
It performs these steps in the following order:
cmake, make, make install, rpmbuild.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>SshConnection</name>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sshcapabilities.cpp" line="96"/>
        <source>Server and client capabilities don&apos;t match. Client list was: %1.
Server list was %2.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>SshKeyGenerator</name>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sshkeygenerator.cpp" line="80"/>
        <source>Error generating key: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sshkeygenerator.cpp" line="178"/>
        <source>Password for Private Key</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sshkeygenerator.cpp" line="179"/>
        <source>It is recommended that you secure your private key
with a password, which you can enter below.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sshkeygenerator.cpp" line="181"/>
        <source>Encrypt Key File</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../3rdparty/qssh/src/libs/ssh/sshkeygenerator.cpp" line="182"/>
        <source>Do Not Encrypt Key File</source>
        <translation type="unfinished"></translation>
    </message>
</context>
</TS>
