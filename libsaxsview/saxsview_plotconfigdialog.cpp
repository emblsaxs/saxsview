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
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QStackedWidget>
#include <QStringList>
#include <QWidget>

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


class CurveConfigWidget : public QWidget {
public:
  CurveConfigWidget(QWidget *parent = 0L);

  void apply(PlotCurve *curve);
  void reset(PlotCurve *curve);

  QCheckBox *checkVisible;
  QLineEdit *editLegendLabel;
  QComboBox *comboLineStyle;
  QSpinBox *spinLineWidth;
  QPushButton *buttonLineColor;
  QComboBox *comboSymbolStyle;
  QSpinBox *spinSymbolSize;
  QPushButton *buttonSymbolColor;
  QCheckBox *checkSymbolFilled;
  QComboBox *comboErrorbarStyle;
  QSpinBox *spinErrorbarWidth;
  QPushButton *buttonErrorbarColor;
  QDoubleSpinBox *spinScalingFactor;
  QDoubleSpinBox *spinOffset;
};

CurveConfigWidget::CurveConfigWidget(QWidget *parent)
 : QWidget(parent),
   checkVisible(new QCheckBox(this)),
   editLegendLabel(new QLineEdit(this)),
   comboLineStyle(new QComboBox(this)),
   spinLineWidth(new QSpinBox(this)),
   buttonLineColor(new QPushButton(this)),
   comboSymbolStyle(new QComboBox(this)),
   spinSymbolSize(new QSpinBox(this)),
   buttonSymbolColor(new QPushButton(this)),
   checkSymbolFilled(new QCheckBox(this)),
   comboErrorbarStyle(new QComboBox(this)),
   spinErrorbarWidth(new QSpinBox(this)),
   buttonErrorbarColor(new QPushButton(this)),
   spinScalingFactor(new QDoubleSpinBox(this)),
   spinOffset(new QDoubleSpinBox(this)) {

  QHBoxLayout *layout = new QHBoxLayout;
  layout->setSpacing(0);
  layout->setContentsMargins(0,0,0,0);

  layout->addWidget(checkVisible);
  layout->addWidget(editLegendLabel);
  layout->addWidget(vLine(this));
  layout->addWidget(comboLineStyle);
  layout->addWidget(spinLineWidth);
  layout->addWidget(buttonLineColor);
  layout->addWidget(vLine(this));
  layout->addWidget(comboSymbolStyle);
  layout->addWidget(spinSymbolSize);
  layout->addWidget(checkSymbolFilled);
  layout->addWidget(buttonSymbolColor);
  layout->addWidget(vLine(this));
  layout->addWidget(comboErrorbarStyle);
  layout->addWidget(spinErrorbarWidth);
  layout->addWidget(buttonErrorbarColor);
  layout->addWidget(vLine(this));
  layout->addWidget(spinScalingFactor);
  layout->addWidget(spinOffset);
  setLayout(layout);
}

void CurveConfigWidget::apply(PlotCurve *curve) {
  curve->setVisible(checkVisible->isChecked());
  curve->setTitle(editLegendLabel->text());
}

void CurveConfigWidget::reset(PlotCurve *curve) {
  checkVisible->setChecked(curve->isVisible());
  editLegendLabel->setText(curve->title());
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
  scrollArea->setWidgetResizable(false);

  QWidget *w = new QWidget;
  QVBoxLayout *layout = new QVBoxLayout;
  foreach (PlotCurve *curve, plot->curves()) {
    curveConfig[curve] = new CurveConfigWidget(w);
    layout->addWidget(curveConfig[curve]);
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

  plot->updateLayout();
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
