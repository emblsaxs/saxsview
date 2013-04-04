

#ifndef SVIMAGEMASKTHRESHOLDSDIALOG_H
#define SVIMAGEMASKTHRESHOLDSDIALOG_H

#include <QDialog>

class SVImageMaskThresholdsDialog : public QDialog {
  Q_OBJECT

public:
  SVImageMaskThresholdsDialog(QWidget *parent = 0L);

  void setRange(double min, double max);

  void selectedThreshold(double *min, double *max) const;

signals:
  /** Emitted whenever a threshold changes. */
  void currentThresholdChanged(double lower, double upper);

  /** Emitted once the dialog was accepted. */
  void thresholdSelected(double lower, double upper);

private slots:
  void thresholdChanged();
  void accept();

private:
  class Private;
  Private *p;
};

#endif // !SVIMAGEMASKTHRESHOLDSDIALOG_H
