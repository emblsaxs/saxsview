/*
 * Copyright (C) 2009 Daniel Franke <dfranke@users.sourceforge.net>
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

#include "saxsview_plotconfigdialog.h"
#include "saxsview_plot.h"
#include "saxsview_plotcurve.h"
#include "saxsview_colorbutton.h"

#include <QCheckBox>
#include <QDebug>
#include <QDialogButtonBox>
#include <QFontComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMap>
#include <QPainter>
#include <QPen>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QStackedWidget>
#include <QStringList>
#include <QWidget>

#include <qwt_symbol.h>
#include <qwt_text.h>


namespace Saxsview {

//-------------------------------------------------------------------------
class ConfigPage : public QWidget {
public:
  ConfigPage(QWidget *parent = 0L) : QWidget(parent) {}

  virtual void apply(Plot *plot) = 0;
  virtual void reset(Plot *plot) = 0;
};


//-------------------------------------------------------------------------
class PlotConfigPage : public ConfigPage {
public:
  PlotConfigPage(Plot *plot, QWidget *parent = 0L);
  void apply(Plot *plot);
  void reset(Plot *plot);

private:
  QLineEdit *editTitle;
};

PlotConfigPage::PlotConfigPage(Plot *plot, QWidget *parent)
 : ConfigPage(parent) {
  QVBoxLayout *layout;

  QGroupBox *group = new QGroupBox("Plot Title", this);
  editTitle = new QLineEdit(group);

  layout = new QVBoxLayout;
  layout->addWidget(editTitle);
  layout->addStretch();
  group->setLayout(layout);

  layout = new QVBoxLayout;
  layout->addWidget(group);
  setLayout(layout);
}

void PlotConfigPage::apply(Plot *plot) {
  plot->setTitle(editTitle->text());
}

void PlotConfigPage::reset(Plot *plot) {
  editTitle->setText(plot->title().text());
}

//-------------------------------------------------------------------------
static QFrame* vLine(QWidget *parent) {
  QFrame *frame = new QFrame(parent);
  frame->setFrameShape(QFrame::VLine);
  frame->setFrameShadow(QFrame::Sunken);
  return frame;
}

static QFrame* hLine(QWidget *parent) {
  QFrame *frame = new QFrame(parent);
  frame->setFrameShape(QFrame::HLine);
  frame->setFrameShadow(QFrame::Sunken);
  return frame;
}

static QIcon penStyleIcon(Qt::PenStyle style) {
  QPixmap pixmap(16, 16);

  QPen pen;
  pen.setColor(Qt::black);
  pen.setStyle(style);
  pen.setWidth(1);

  QPainter painter;
  painter.begin(&pixmap);
  painter.setPen(QPen(Qt::NoPen));
  painter.fillRect(pixmap.rect(), QBrush(Qt::white));
  painter.setPen(pen);
  painter.drawLine(0, 16, 16, 0);
  painter.end();

  return QIcon(pixmap);
}

static QComboBox* comboBoxLineStyle(QWidget *parent) {
  QComboBox *combo = new QComboBox(parent);
  combo->addItem(penStyleIcon(Qt::NoPen), "none");
  combo->addItem(penStyleIcon(Qt::SolidLine), "solid");
  combo->addItem(penStyleIcon(Qt::DashLine), "dashed");
  combo->addItem(penStyleIcon(Qt::DotLine), "dotted");
  combo->addItem(penStyleIcon(Qt::DashDotLine), "dash-dot");
  combo->addItem(penStyleIcon(Qt::DashDotDotLine), "dash-dot-dot");
  return combo;
}

static QIcon symbolIcon(QwtSymbol::Style style, bool filled = false) {
  QPixmap pixmap(16, 16);

  QPainter painter;
  painter.begin(&pixmap);
  painter.setPen(QPen(Qt::NoPen));
  painter.fillRect(pixmap.rect(), QBrush(Qt::white));

  QwtSymbol symbol;
  symbol.setSize(10, 10);
  symbol.setBrush(filled ? QBrush(Qt::black) : QBrush(Qt::NoBrush));
  symbol.setPen(QPen(Qt::black));
  symbol.setStyle(style);
  symbol.draw(&painter, 7, 7);

  painter.end();

  return QIcon(pixmap);
}

static QComboBox* comboBoxSymbolStyle(QWidget *parent) {
  QComboBox *combo = new QComboBox(parent);
  combo->addItem(symbolIcon(QwtSymbol::NoSymbol), "none", QwtSymbol::NoSymbol);
  combo->insertSeparator(1);
  combo->addItem(symbolIcon(QwtSymbol::Ellipse), "circle", QwtSymbol::Ellipse);
  combo->addItem(symbolIcon(QwtSymbol::Rect), "rectangle", QwtSymbol::Rect);
  combo->addItem(symbolIcon(QwtSymbol::Diamond), "diamond", QwtSymbol::Diamond);
  combo->addItem(symbolIcon(QwtSymbol::DTriangle), "triangle (down)", QwtSymbol::DTriangle);
  combo->addItem(symbolIcon(QwtSymbol::UTriangle), "triangle (up)", QwtSymbol::UTriangle);
  combo->addItem(symbolIcon(QwtSymbol::LTriangle), "triangle (left)", QwtSymbol::LTriangle);
  combo->addItem(symbolIcon(QwtSymbol::RTriangle), "triangle (right)", QwtSymbol::RTriangle);
  combo->addItem(symbolIcon(QwtSymbol::Star2), "star (outline)", QwtSymbol::Star2);
  combo->addItem(symbolIcon(QwtSymbol::Hexagon), "hexagon", QwtSymbol::Hexagon);
  combo->insertSeparator(12);
  combo->addItem(symbolIcon(QwtSymbol::Cross), "cross", QwtSymbol::Cross);
  combo->addItem(symbolIcon(QwtSymbol::XCross), "cross (diagonal)", QwtSymbol::XCross);
  combo->addItem(symbolIcon(QwtSymbol::HLine), "line (horizontal)", QwtSymbol::HLine);
  combo->addItem(symbolIcon(QwtSymbol::VLine), "line (vertical)", QwtSymbol::VLine);
  combo->addItem(symbolIcon(QwtSymbol::Star1), "star", QwtSymbol::Star1);
  combo->insertSeparator(18);
  combo->addItem(symbolIcon(QwtSymbol::Ellipse, true), "circle", QwtSymbol::Ellipse + 100);
  combo->addItem(symbolIcon(QwtSymbol::Rect, true), "rectangle", QwtSymbol::Rect + 100);
  combo->addItem(symbolIcon(QwtSymbol::Diamond, true), "diamond", QwtSymbol::Diamond + 100);
  combo->addItem(symbolIcon(QwtSymbol::DTriangle, true), "triangle (down)", QwtSymbol::DTriangle + 100);
  combo->addItem(symbolIcon(QwtSymbol::UTriangle, true), "triangle (up)", QwtSymbol::UTriangle + 100);
  combo->addItem(symbolIcon(QwtSymbol::LTriangle, true), "triangle (left)", QwtSymbol::LTriangle + 100);
  combo->addItem(symbolIcon(QwtSymbol::RTriangle, true), "triangle (right)", QwtSymbol::RTriangle + 100);
  combo->addItem(symbolIcon(QwtSymbol::Star2, true), "star (outline)", QwtSymbol::Star2 + 100);
  combo->addItem(symbolIcon(QwtSymbol::Hexagon, true), "hexagon", QwtSymbol::Hexagon + 100);
  return combo;
}



class CurveConfigWidget : public QWidget {
public:
  CurveConfigWidget(QWidget *parent = 0L);

  void apply(PlotCurve *curve);
  void reset(PlotCurve *curve);

  QGroupBox *group;
  QLineEdit *editLegendLabel;
  QComboBox *comboLineStyle;
  QSpinBox *spinLineWidth;
  ColorButton *buttonLineColor;
  QComboBox *comboSymbolStyle;
  QSpinBox *spinSymbolSize;
  ColorButton *buttonSymbolColor;
  QComboBox *comboErrorbarStyle;
  QSpinBox *spinErrorbarWidth;
  ColorButton *buttonErrorbarColor;
  QDoubleSpinBox *spinScaleX;
  QDoubleSpinBox *spinScaleY;
};

CurveConfigWidget::CurveConfigWidget(QWidget *parent)
 : QWidget(parent),
   group(new QGroupBox(this)),
   editLegendLabel(new QLineEdit(this)),
   comboLineStyle(comboBoxLineStyle(this)),
   spinLineWidth(new QSpinBox(this)),
   buttonLineColor(new ColorButton(this)),
   comboSymbolStyle(comboBoxSymbolStyle(this)),
   spinSymbolSize(new QSpinBox(this)),
   buttonSymbolColor(new ColorButton(this)),
   comboErrorbarStyle(comboBoxLineStyle(this)),
   spinErrorbarWidth(new QSpinBox(this)),
   buttonErrorbarColor(new ColorButton(this)),
   spinScaleX(new QDoubleSpinBox(this)),
   spinScaleY(new QDoubleSpinBox(this)) {

  spinLineWidth->setSuffix("pt");
  spinSymbolSize->setSuffix("pt");
  spinErrorbarWidth->setSuffix("pt");

  spinScaleX->setDecimals(1);
  spinScaleX->setRange(0.1, 10.0);
  spinScaleY->setDecimals(4);
  spinScaleY->setRange(0.0001, 100000.0);

  buttonLineColor->setMinimumWidth(60);

  QGridLayout *groupLayout = new QGridLayout;
  groupLayout->addWidget(new QLabel("Legend Label"), 0, 0);
  groupLayout->addWidget(editLegendLabel, 0, 1, 1, 3);
  groupLayout->addWidget(new QLabel("Line Style"), 1, 0);
  groupLayout->addWidget(comboLineStyle, 1, 1);
  groupLayout->addWidget(spinLineWidth, 1, 2);
  groupLayout->addWidget(buttonLineColor, 1, 3);
  groupLayout->addWidget(new QLabel("Symbol Style"), 2, 0);
  groupLayout->addWidget(comboSymbolStyle, 2, 1);
  groupLayout->addWidget(spinSymbolSize, 2, 2);
  groupLayout->addWidget(buttonSymbolColor, 2, 3);
  groupLayout->addWidget(new QLabel("Error Bar Style"), 3, 0);
  groupLayout->addWidget(comboErrorbarStyle, 3, 1);
  groupLayout->addWidget(spinErrorbarWidth, 3, 2);
  groupLayout->addWidget(buttonErrorbarColor, 3, 3);
  groupLayout->addWidget(hLine(group), 4, 0, 1, 4);
  groupLayout->addWidget(new QLabel("Scale X"), 5, 0);
  groupLayout->addWidget(spinScaleX, 5, 1);
  groupLayout->addWidget(new QLabel("Scale Y"), 5, 2);
  groupLayout->addWidget(spinScaleY, 5, 3);

  group->setCheckable(true);
  group->setLayout(groupLayout);
}

void CurveConfigWidget::apply(PlotCurve *curve) {
  curve->setVisible(group->isChecked());
  curve->setTitle(editLegendLabel->text());

  curve->setScalingFactorX(spinScaleX->value());
  curve->setScalingFactorY(spinScaleY->value());
}

void CurveConfigWidget::reset(PlotCurve *curve) {
  group->setTitle(curve->fileName());
  group->setChecked(curve->isVisible());
  editLegendLabel->setText(curve->title());

  spinScaleX->setValue(curve->scalingFactorX());
  spinScaleY->setValue(curve->scalingFactorY());
}

//-------------------------------------------------------------------------
class CurveConfigPage : public ConfigPage {
public:
  CurveConfigPage(Plot *plot, QWidget *parent = 0L);
  void apply(Plot *plot);
  void reset(Plot *plot);

  QMap<PlotCurve*, CurveConfigWidget*> curveConfig;
};

CurveConfigPage::CurveConfigPage(Plot *plot, QWidget *parent)
 : ConfigPage(parent) {

  QScrollArea *scrollArea = new QScrollArea(this);
  scrollArea->setWidgetResizable(true);

  QWidget *w = new QWidget;
  QVBoxLayout *layout = new QVBoxLayout;

  foreach (PlotCurve *curve, plot->curves()) {
    curveConfig[curve] = new CurveConfigWidget(w);
    layout->addWidget(curveConfig[curve]->group);
  }
  layout->addStretch();

  w->setLayout(layout);
  scrollArea->setWidget(w);

  layout = new QVBoxLayout;
  layout->addWidget(scrollArea);
  setLayout(layout);
}

void CurveConfigPage::apply(Plot *plot) {
  QMap<PlotCurve*, CurveConfigWidget*>::iterator it;
  for (it = curveConfig.begin(); it != curveConfig.end(); ++it)
    it.value()->apply(it.key());
}

void CurveConfigPage::reset(Plot *) {
  QMap<PlotCurve*, CurveConfigWidget*>::iterator it;
  for (it = curveConfig.begin(); it != curveConfig.end(); ++it)
    it.value()->reset(it.key());
}



//-------------------------------------------------------------------------
class PlotConfigDialog::PlotConfigDialogPrivate {
public:
  PlotConfigDialogPrivate(Plot *plot);

  void setupUi(PlotConfigDialog *dlg);
  void setupIcons(PlotConfigDialog *dlg);

  void apply();
  void reset();

  Plot *plot;
  QListWidget *listView;
  QStackedWidget *stackedPages;
  QDialogButtonBox *buttonBox;
  QList<ConfigPage*> configPages;
};

PlotConfigDialog::PlotConfigDialogPrivate::PlotConfigDialogPrivate(Plot *p)
 : plot(p) {
}

void PlotConfigDialog::PlotConfigDialogPrivate::setupUi(PlotConfigDialog *dlg) {
  listView = new QListWidget(dlg);
  listView->setViewMode(QListView::IconMode);
  listView->setIconSize(QSize(96, 84));
  listView->setMovement(QListView::Static);
  listView->setMaximumWidth(128);
  listView->setSpacing(12);
  listView->setCurrentRow(0);
  setupIcons(dlg);

  configPages.append(new PlotConfigPage(plot, dlg));
  configPages.append(new CurveConfigPage(plot, dlg));

  stackedPages = new QStackedWidget(dlg);
  foreach(ConfigPage *page, configPages)
    stackedPages->addWidget(page);

  QHBoxLayout *horizontalLayout = new QHBoxLayout;
  horizontalLayout->addWidget(listView);
  horizontalLayout->addWidget(stackedPages, 1);

  buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                    | QDialogButtonBox::Apply
                                    | QDialogButtonBox::Reset
                                    | QDialogButtonBox::RestoreDefaults
                                    | QDialogButtonBox::Cancel,
                                   Qt::Horizontal, dlg);
  // FIXME: defaults not implemented yet
  buttonBox->button(QDialogButtonBox::RestoreDefaults)->setEnabled(false);

  connect(buttonBox, SIGNAL(clicked(QAbstractButton*)),
          dlg, SLOT(buttonClicked(QAbstractButton*)));

  QVBoxLayout *verticalLayout = new QVBoxLayout;
  verticalLayout->addLayout(horizontalLayout);
  verticalLayout->addWidget(buttonBox);

  dlg->setLayout(verticalLayout);
  dlg->setWindowTitle("FIXME: set title");
}

void PlotConfigDialog::PlotConfigDialogPrivate::setupIcons(PlotConfigDialog *dlg) {
  QListWidgetItem *icon;

  icon = new QListWidgetItem(listView);
//   icon->setIcon(QIcon(":/images/config.png"));
  icon->setText(tr("Plot Setup"));
  icon->setTextAlignment(Qt::AlignHCenter);
  icon->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

  icon = new QListWidgetItem(listView);
//   icon->setIcon(QIcon(":/images/config.png"));
  icon->setText(tr("Curves Setup"));
  icon->setTextAlignment(Qt::AlignHCenter);
  icon->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

  connect(listView, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
          dlg, SLOT(changePage(QListWidgetItem*, QListWidgetItem*)));
}

void PlotConfigDialog::PlotConfigDialogPrivate::apply() {
  plot->blockReplot(true);

  foreach(ConfigPage *page, configPages)
    page->apply(plot);

  plot->blockReplot(false);
}

void PlotConfigDialog::PlotConfigDialogPrivate::reset() {
  foreach(ConfigPage *page, configPages)
    page->reset(plot);
}


//-------------------------------------------------------------------------
PlotConfigDialog::PlotConfigDialog(Plot *plot, QWidget *parent)
 : QDialog(parent), p(new PlotConfigDialogPrivate(plot)) {

  p->setupUi(this);
  p->reset();
}

PlotConfigDialog::~PlotConfigDialog() {
  delete p;
}

void PlotConfigDialog::changePage(QListWidgetItem *cur, QListWidgetItem *prev) {
  p->stackedPages->setCurrentIndex(p->listView->row(cur ? cur : prev));
}

void PlotConfigDialog::buttonClicked(QAbstractButton *button) {
  switch (p->buttonBox->standardButton(button)) {
    case QDialogButtonBox::Ok:
      p->apply();
      accept();
      break;

    case QDialogButtonBox::Apply:
      p->apply();
      break;

    case QDialogButtonBox::Reset:
      p->reset();
      break;

    case QDialogButtonBox::RestoreDefaults:
//       p->defaults();
      break;

    case QDialogButtonBox::Cancel:
    default:
      reject();
      break;
  }
}

} // end of namespace Saxsview
