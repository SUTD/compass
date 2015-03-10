#include <KApplication>
#include <KAboutData>
#include <KCmdLineArgs>
#include <KMessageBox>
#include <KLocale>
#include <QtGui>
#include "compass_ui.h"
#include <iostream>
using namespace std;
 
int main (int argc, char *argv[])
{
    KAboutData aboutData(
                         // The program name used internally.
                         "compass-ui",
                         // The message catalog name
                         // If null, program name is used instead.
                         0,
                         // A displayable program name string.
                         ki18n("Compass"),
                         // The program version string.
                         "1.0",
                         // Short description of what the app does.
                         ki18n("Displays a KMessageBox popup"),
                         // The license this code is released under
                         KAboutData::License_Custom,
                         // Copyright Statement
                         ki18n("(c) 2009"),
                         // Optional text shown in the About box.
                         // Can contain any information desired.
                         ki18n("Some text..."),
                         // The program homepage string.
                         "http://tutorial.com/",
                         // The bug report email address
                         "isil@stanford.edu");
 
    KCmdLineArgs::init( argc, argv, &aboutData );
    KApplication app;


    compass_ui* _ui  = new compass_ui();
    QObject::connect( &app, SIGNAL(lastWindowClosed()),
                          &app, SLOT(quit()) );

    QObject::connect( &app, SIGNAL(aboutToQuit()),
                          _ui, SLOT(app_quit()) );

    _ui->show();
    return app.exec();
    /*
    QLabel l("sss");
    KGuiItem yesButton( i18n( "123" ), QString(),
                        i18n( "This is a tooltip" ),
                        i18n( "This is a WhatsThis help text." ) );
    KMessageBox::questionYesNo(&l, i18n( "Hello World" ),
                                i18n( "Hello" ), yesButton );

    */

}



