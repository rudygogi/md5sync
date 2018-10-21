#include <QApplication>

#include "Widget.h"
#include "Md5SyncWidget.h"

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   a.setOrganizationDomain("org");
   a.setOrganizationName("g-bits");
   a.setApplicationName("Md5Sync");

   //Widget w;
   Md5SyncWidget w;
   w.show();

   return a.exec();
}
