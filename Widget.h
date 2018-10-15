#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
   Q_OBJECT

public:
   explicit Widget(QWidget *parent = 0);
   ~Widget();

protected:
   bool eventFilter(QObject *watched, QEvent *event) override;

private:
   void compare();
   void updateConnections();
   void selectMissing();

   QSet<int> getRightRowSet(const QString& md5Hash) const;   
   QSet<int> getLeftRowSet(const QString& md5Hash) const;

private:
   Ui::Widget *ui;
};

#endif // WIDGET_H
