#include "Widget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   a.setOrganizationDomain("org");
   a.setOrganizationName("g-bits");
   a.setApplicationName("Md5Sync");

   Widget w;
   w.show();

   return a.exec();
}
