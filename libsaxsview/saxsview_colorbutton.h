#ifndef SAXSVIEW_COLORBUTTON_H
#define SAXSVIEW_COLORBUTTON_H

#include <QPushButton>
#include <QColor>

namespace Saxsview {

class ColorButton : public QPushButton {
  Q_OBJECT

public:
  ColorButton(QWidget *parent = 0L);
  ~ColorButton();

  QColor color() const;

public slots:
  void getColor();
  void setColor(const QColor&);

signals:
  void colorChanged(const QColor&);

protected:
  void resizeEvent(QResizeEvent*);
  void updateIcon();

private:
  QColor mColor;
};

} // end of namespace Saxsview

#endif // !SAXSVIEW_COLORBUTTON_H

