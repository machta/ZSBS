#include "spikedetsettingsdialog.h"

#include "error.h"

#include <QAbstractButton>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>

using namespace std;

SpikedetSettingsDialog::SpikedetSettingsDialog(DETECTOR_SETTINGS *settings,
                                               double *eventDuration,
                                               bool *originalSpikedet,
                                               QWidget *parent)
    : QDialog(parent), settings(settings), eventDuration(eventDuration),
      originalSpikedet(originalSpikedet) {
  auto box = new QVBoxLayout();

  auto grid = new QFormLayout();
  box->addLayout(grid);

  auto buttonBox =
      new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel |
                           QDialogButtonBox::Reset);
  connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

  connect(buttonBox, &QDialogButtonBox::clicked,
          [this, buttonBox, settings, eventDuration,
           originalSpikedet](QAbstractButton *button) {
            if (buttonBox->buttonRole(button) == QDialogButtonBox::ResetRole) {
              resetSettings(settings, eventDuration, originalSpikedet);
              setValues();
            }
          });

  box->addWidget(buttonBox);

  setLayout(box);

  QLabel *label;
  const double maxFrequency = 10000;
  const int maxDecimals = 3;

  label = new QLabel("Band low:");
  label->setToolTip("Lowpass filter frequency (--fl)");
  fl_box = new QSpinBox();
  fl_box->setMaximum(maxFrequency);
  connect(fl_box, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
          [settings](int val) { settings->m_band_low = val; });
  grid->addRow(label, fl_box);

  label = new QLabel("Band high:");
  label->setToolTip("Highpass filter frequency (--fh)");
  fh_box = new QSpinBox();
  fh_box->setMaximum(maxFrequency);
  connect(fh_box, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
          [settings](int val) { settings->m_band_high = val; });
  grid->addRow(label, fh_box);

  label = new QLabel("K1:");
  label->setToolTip("(--k1)");
  k1_box = new QDoubleSpinBox();
  k1_box->setDecimals(maxDecimals);
  k1_box->setMaximum(1000);
  connect(k1_box, static_cast<void (QDoubleSpinBox::*)(double)>(
                      &QDoubleSpinBox::valueChanged),
          [settings](double val) { settings->m_k1 = val; });
  grid->addRow(label, k1_box);

  label = new QLabel("K2:");
  label->setToolTip("if K2=K1, K2 is ignored (--k2)");
  k2_box = new QDoubleSpinBox();
  k2_box->setDecimals(maxDecimals);
  k2_box->setMaximum(1000);
  connect(k2_box, static_cast<void (QDoubleSpinBox::*)(double)>(
                      &QDoubleSpinBox::valueChanged),
          [settings](double val) { settings->m_k2 = val; });
  grid->addRow(label, k2_box);

  label = new QLabel("K3:");
  label->setToolTip("(--k3)");
  k3_box = new QDoubleSpinBox();
  k3_box->setDecimals(maxDecimals);
  k3_box->setMaximum(1000);
  connect(k3_box, static_cast<void (QDoubleSpinBox::*)(double)>(
                      &QDoubleSpinBox::valueChanged),
          [settings](double val) { settings->m_k3 = val; });
  grid->addRow(label, k3_box);

  label = new QLabel("Winsize:");
  label->setToolTip("(--w)");
  w_box = new QSpinBox();
  // spinBox->setMaximum();
  connect(w_box, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
          [settings](int val) { settings->m_winsize = val; });
  grid->addRow(label, w_box);

  label = new QLabel("Noverlap:");
  label->setToolTip("(--n)");
  n_box = new QDoubleSpinBox();
  n_box->setDecimals(maxDecimals);
  // n_box->setMaximum();
  connect(n_box, static_cast<void (QDoubleSpinBox::*)(double)>(
                     &QDoubleSpinBox::valueChanged),
          [settings](double val) { settings->m_noverlap = val; });
  grid->addRow(label, n_box);

  label = new QLabel("Buffering:");
  label->setToolTip("(--buf)");
  buf_box = new QSpinBox();
  buf_box->setMaximum(10000);
  connect(buf_box,
          static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
          [settings](int val) { settings->m_buffering = val; });
  grid->addRow(label, buf_box);

  label = new QLabel("Main hum. freq.:");
  label->setToolTip("(--h)");
  h_box = new QSpinBox();
  h_box->setMaximum(maxFrequency);
  connect(h_box, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
          [settings](int val) { settings->m_main_hum_freq = val; });
  grid->addRow(label, h_box);

  label = new QLabel("Discharge tol.:");
  label->setToolTip("(--dt)");
  dt_box = new QDoubleSpinBox();
  dt_box->setDecimals(maxDecimals);
  // dt_box->setMaximum();
  connect(dt_box, static_cast<void (QDoubleSpinBox::*)(double)>(
                      &QDoubleSpinBox::valueChanged),
          [settings](double val) { settings->m_discharge_tol = val; });
  grid->addRow(label, dt_box);

  label = new QLabel("Polyspike union time:");
  label->setToolTip("(--pt)");
  pt_box = new QDoubleSpinBox();
  pt_box->setDecimals(maxDecimals);
  // ps_box->setMaximum();
  connect(pt_box, static_cast<void (QDoubleSpinBox::*)(double)>(
                      &QDoubleSpinBox::valueChanged),
          [settings](double val) { settings->m_polyspike_union_time = val; });
  grid->addRow(label, pt_box);

  label = new QLabel("Decimation:");
  label->setToolTip("(--dec)");
  dec_box = new QSpinBox();
  dec_box->setMaximum(maxFrequency);
  connect(dec_box,
          static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
          [settings](int val) { settings->m_decimation = val; });
  grid->addRow(label, dec_box);

  label = new QLabel("Spike event duration:");
  label->setToolTip("in seconds (--sed)");
  sed_box = new QDoubleSpinBox();
  sed_box->setDecimals(maxDecimals);
  // sed_box->setMaximum();
  connect(sed_box, static_cast<void (QDoubleSpinBox::*)(double)>(
                       &QDoubleSpinBox::valueChanged),
          [eventDuration](double val) { *eventDuration = val; });
  grid->addRow(label, sed_box);

  label = new QLabel("Original Spikedet:");
  label->setToolTip("Use orginal Spikedet implementation instead of the "
                    "optimized version (--osd)");
  odm_box = new QCheckBox();
  connect(odm_box, &QCheckBox::clicked,
          [originalSpikedet](bool val) { *originalSpikedet = val; });
  grid->addRow(label, odm_box);

  setValues();
}

void SpikedetSettingsDialog::resetSettings(DETECTOR_SETTINGS *settings,
                                           double *eventDuration,
                                           bool *originalSpikedet) {
  settings->m_band_low = PROGRAM_OPTIONS["fl"].as<int>();
  settings->m_band_high = PROGRAM_OPTIONS["fh"].as<int>();
  settings->m_k1 = PROGRAM_OPTIONS["k1"].as<double>();
  settings->m_k2 = PROGRAM_OPTIONS["k2"].as<double>();
  settings->m_k3 = PROGRAM_OPTIONS["k3"].as<double>();
  settings->m_winsize = PROGRAM_OPTIONS["w"].as<int>();
  settings->m_noverlap = PROGRAM_OPTIONS["n"].as<double>();
  settings->m_buffering = PROGRAM_OPTIONS["buf"].as<int>();
  settings->m_main_hum_freq = PROGRAM_OPTIONS["h"].as<int>();
  settings->m_discharge_tol = PROGRAM_OPTIONS["dt"].as<double>();
  settings->m_polyspike_union_time = PROGRAM_OPTIONS["pt"].as<double>();
  settings->m_decimation = PROGRAM_OPTIONS["dec"].as<int>();

  if (eventDuration)
    *eventDuration = PROGRAM_OPTIONS["sed"].as<double>();

  if (originalSpikedet)
    *originalSpikedet = PROGRAM_OPTIONS["osd"].as<bool>();
}

void SpikedetSettingsDialog::setValues() {
  fl_box->setValue(settings->m_band_low);
  fh_box->setValue(settings->m_band_high);
  k1_box->setValue(settings->m_k1);
  k2_box->setValue(settings->m_k2);
  k3_box->setValue(settings->m_k3);
  w_box->setValue(settings->m_winsize);
  n_box->setValue(settings->m_noverlap);
  buf_box->setValue(settings->m_buffering);
  h_box->setValue(settings->m_main_hum_freq);
  dt_box->setValue(settings->m_discharge_tol);
  pt_box->setValue(settings->m_polyspike_union_time);
  dec_box->setValue(settings->m_decimation);
  sed_box->setValue(*eventDuration);
  odm_box->setChecked(*originalSpikedet);
}
