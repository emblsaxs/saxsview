
#include "saxsview_scaledraw.h"

SaxsviewScaleDraw::SaxsviewScaleDraw() : mLabelColor(Qt::black) {
}

QwtText SaxsviewScaleDraw::label(double value) const {
  QwtText text = QwtScaleDraw::label(value);
  text.setColor(mLabelColor);
  return text;
}

QColor SaxsviewScaleDraw::labelColor() const {
  return mLabelColor;
}

void SaxsviewScaleDraw::setLabelColor(const QColor& c) {
  mLabelColor = c;

  //
  // The labels texts (including their color) are cached.
  // Invalidate this cache to rebuild them ...
  //
  invalidateCache();
}
