
#include "svimagemaskthresholdsdialog.h"

#include <QtGui>


class SVImageMaskThresholdsDialog::Private {
public:
  void setupUi(SVImageMaskThresholdsDialog*);


  QCheckBox *checkAboveThreshold, *checkBelowThreshold;
  QSpinBox *spinLowerThreshold, *spinUpperThreshold;
};

void SVImageMaskThresholdsDialog::Private::setupUi(SVImageMaskThresholdsDialog *dlg) {
  spinUpperThreshold = new QSpinBox(dlg);
  connect(spinUpperThreshold, SIGNAL(valueChanged(int)),
          dlg, SLOT(thresholdChanged()));

  checkAboveThreshold = new QCheckBox("above this threshold", dlg);
  checkAboveThreshold->setChecked(true);
  connect(checkAboveThreshold, SIGNAL(toggled(bool)),
          spinUpperThreshold, SLOT(setEnabled(bool)));

  spinLowerThreshold = new QSpinBox(dlg);
  connect(spinLowerThreshold, SIGNAL(valueChanged(int)),
          dlg, SLOT(thresholdChanged()));

  checkBelowThreshold = new QCheckBox("below this threshold", dlg);
  checkBelowThreshold->setChecked(true);
  connect(checkBelowThreshold, SIGNAL(toggled(bool)),
          spinLowerThreshold, SLOT(setEnabled(bool)));

  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  connect(buttonBox, SIGNAL(accepted()),
          dlg, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()),
          dlg, SLOT(reject()));

  QGridLayout *layout = new QGridLayout();
  layout->addWidget(new QLabel("Mask pixels ..."), 0, 0, 1, 2);
  layout->addWidget(checkAboveThreshold, 1, 0, 1, 1);
  layout->addWidget(spinUpperThreshold, 1, 1, 1, 1);
  layout->addWidget(checkBelowThreshold, 2, 0, 1, 1);
  layout->addWidget(spinLowerThreshold, 2, 1, 1, 1);
  layout->addWidget(buttonBox, 3, 0, 1, 2);

  dlg->setLayout(layout);
  dlg->setWindowTitle("Set Mask By Threshold");
}


SVImageMaskThresholdsDialog::SVImageMaskThresholdsDialog(QWidget *parent)
 : QDialog(parent), p(new Private) {

  p->setupUi(this);
}

void SVImageMaskThresholdsDialog::setRange(double min, double max) {
  p->spinUpperThreshold->setRange(min, max);
  p->spinUpperThreshold->setValue(max);

  p->spinLowerThreshold->setRange(min, max);
  p->spinLowerThreshold->setValue(min);
}

void SVImageMaskThresholdsDialog::selectedThreshold(double *min, double *max) const {
  if (min) {
    if (p->checkBelowThreshold->isChecked())
      *min = p->spinLowerThreshold->value();
    else
      *min = p->spinLowerThreshold->minimum();
  }

  if (max) {
    if (p->checkAboveThreshold->isChecked())
      *max = p->spinUpperThreshold->value();
    else
      *max = p->spinUpperThreshold->maximum();
  }
}

void SVImageMaskThresholdsDialog::thresholdChanged() {
  emit currentThresholdChanged(p->spinLowerThreshold->value(),
                               p->spinUpperThreshold->value());
}

void SVImageMaskThresholdsDialog::accept() {
  emit thresholdSelected(p->spinLowerThreshold->value(),
                         p->spinUpperThreshold->value());
  QDialog::accept();
}
