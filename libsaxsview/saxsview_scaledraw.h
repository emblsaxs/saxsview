#ifndef SAXSVIEW_SCALEDRAW_H
#define SAXSVIEW_SCALEDRAW_H

#include "qwt_scale_draw.h"
#include <QColor>

//
// By default, QwtScaleDraw uses the same color for axis titles and labels.
// This class is here to allow separate them.
//
// See also:
//   http://sourceforge.net/mailarchive/message.php?msg_id=28994567
//
class SaxsviewScaleDraw : public QwtScaleDraw {
public:
  SaxsviewScaleDraw();

  QwtText label(double value) const;

  QColor labelColor() const;
  void setLabelColor(const QColor& c);

private:
  QColor mLabelColor;
};

#endif // !SAXSVIEW_SCALEDRAW_H
