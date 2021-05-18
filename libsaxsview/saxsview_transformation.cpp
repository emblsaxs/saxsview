/*
 * Copyright (C) 2012 Daniel Franke <dfranke@users.sourceforge.net>
 *
 * This file is part of saxsview.
 *
 * saxsview is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * saxsview is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with saxsview. If not, see <http://www.gnu.org/licenses/>.
 */

#include "saxsview_transformation.h"

#include <muParserDLL.h>

class SaxsviewTransformation::Private {
public:
  Private();
  ~Private();

  SaxsviewPlotPointData transform(const SaxsviewPlotPointData&) const;
  SaxsviewPlotIntervalData transform(const SaxsviewPlotIntervalData&) const;

  int merge;
  double scaleX, scaleY;
  QString exprX, exprY;
};

SaxsviewTransformation::Private::Private()
 : merge(1), scaleX(1.0), scaleY(1.0) {
}

SaxsviewTransformation::Private::~Private() {
}

SaxsviewPlotPointData
SaxsviewTransformation::Private::transform(const SaxsviewPlotPointData& in) const {
  SaxsviewPlotPointData out;

  double x, y, n;
  muParserHandle_t transformX = mupCreate(muBASETYPE_FLOAT);
  mupDefineVar(transformX, "s", &x);
  mupDefineVar(transformX, "I", &y);
  mupSetExpr(transformX, qPrintable(exprX));

  muParserHandle_t transformY = mupCreate(muBASETYPE_FLOAT);
  mupDefineVar(transformY, "s", &x);
  mupDefineVar(transformY, "I", &y);
  mupSetExpr(transformY, qPrintable(exprY));

  for (int i = 0 ; i < in.size(); i += merge) {
    x = y = n = 0.0;

    for (int j = i; j < i + merge && j < in.size(); ++j) {
      x += in.at(j).x();
      y += in.at(j).y();
      n += 1.0;
    }
    x = x * scaleX / n;
    y = y * scaleY / n;

    double fx = mupEval(transformX);
    double fy = mupEval(transformY);
    if (!std::isfinite(fx) || !std::isfinite(fy)) continue;   // e.g. log() of a negative or zero

    out.push_back(QPointF(fx, fy));
  }

  mupRelease(transformX);
  mupRelease(transformY);

  return out;
}

SaxsviewPlotIntervalData
SaxsviewTransformation::Private::transform(const SaxsviewPlotIntervalData& in) const {
  SaxsviewPlotIntervalData out;

  double x, y, var, ymin, ymax, n;
  muParserHandle_t transformX = mupCreate(muBASETYPE_FLOAT);
  mupDefineVar(transformX, "s", &x);
  mupDefineVar(transformX, "I", &y);
  mupSetExpr(transformX, qPrintable(exprX));

  muParserHandle_t transformY = mupCreate(muBASETYPE_FLOAT);
  mupDefineVar(transformY, "s", &x);
  mupDefineVar(transformY, "I", &y);
  mupSetExpr(transformY, qPrintable(exprY));

  muParserHandle_t transformYmin = mupCreate(muBASETYPE_FLOAT);
  mupDefineVar(transformYmin, "s", &x);
  mupDefineVar(transformYmin, "I", &ymin);
  mupSetExpr(transformYmin, qPrintable(exprY));

  muParserHandle_t transformYmax = mupCreate(muBASETYPE_FLOAT);
  mupDefineVar(transformYmax, "s", &x);
  mupDefineVar(transformYmax, "I", &ymax);
  mupSetExpr(transformYmax, qPrintable(exprY));

  //
  // Error propagation for merged points:
  //   \sigma* = \sqrt{\sum_{i=1}^{n} \sigma_i^2} / n
  //
  for (int i = 0 ; i < in.size(); i += merge) {
    x = y = var = n = 0.0;

    for (int j = i; j < i + merge && j < in.size(); ++j) {
      const QwtIntervalSample& is = in.at(j);
      const double ycenter = (is.interval.maxValue() + is.interval.minValue()) / 2.0;

      x    += is.value;
      y    += ycenter;
      var  += pow(is.interval.maxValue() - ycenter, 2);
      n    += 1.0;
    }
    x    = x * scaleX / n;
    ymin = (y - sqrt(var)) * scaleY / n;
    ymax = (y + sqrt(var)) * scaleY / n;

    double fx = mupEval(transformX);
    double fy = mupEval(transformY);
    if (!std::isfinite(fx) || !std::isfinite(fy)) continue;        // e.g. log() of a negative

    // If the upper/lower bound is bad, just drop it.
    double fymin = mupEval(transformYmin);
    if (!std::isfinite(fymin)) fymin = fy;

    double fymax = mupEval(transformYmax);
    if (!std::isfinite(fymax)) fymax = fy;

    out.push_back(QwtIntervalSample(fx, fymin, fymax));
  }

  mupRelease(transformX);
  mupRelease(transformYmin);
  mupRelease(transformYmax);

  return out;
}



SaxsviewTransformation::SaxsviewTransformation()
 : p(new Private) {
}

SaxsviewTransformation::~SaxsviewTransformation() {
  delete p;
}

int SaxsviewTransformation::merge() const {
  return p->merge;
}

void SaxsviewTransformation::setMerge(int n) {
  p->merge = n;
}

double SaxsviewTransformation::scaleX() const {
  return p->scaleX;
}

void SaxsviewTransformation::setScaleX(double s) {
  p->scaleX = s;
}

QString SaxsviewTransformation::transformationX() const {
  return p->exprX;
}

void SaxsviewTransformation::setTransformationX(const QString& t) {
  p->exprX = t;
}

double SaxsviewTransformation::scaleY() const {
  return p->scaleY;
}

void SaxsviewTransformation::setScaleY(double s) {
  p->scaleY = s;
}

QString SaxsviewTransformation::transformationY() const {
  return p->exprY;
}

void SaxsviewTransformation::setTransformationY(const QString& t) {
  p->exprY = t;
}

SaxsviewPlotPointData SaxsviewTransformation::transform(const SaxsviewPlotPointData& data) const {
  return p->transform(data);
}

SaxsviewPlotIntervalData SaxsviewTransformation::transform(const SaxsviewPlotIntervalData& data) const {
  return p->transform(data);
}

bool SaxsviewTransformation::isTransformationValid(const QString& expr) {
  double x = 10.0, y = 10.0;
  muParserHandle_t mup = mupCreate(muBASETYPE_FLOAT);
  mupDefineVar(mup, "s", &x);
  mupDefineVar(mup, "I", &y);
  mupSetExpr(mup, qPrintable(expr));

  mupEval(mup);
  bool valid = !mupError(mup);
  if (!valid)
    qDebug() << mupGetErrorMsg(mup);

  mupRelease(mup);

  return valid;
}
