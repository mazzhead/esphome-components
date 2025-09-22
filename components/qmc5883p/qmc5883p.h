#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace qmc5883p {

enum QMC5883PDatarate {
  QMC5883P_DATARATE_10_HZ = 0b00,
  QMC5883P_DATARATE_50_HZ = 0b01,
  QMC5883P_DATARATE_100_HZ = 0b10,
  QMC5883P_DATARATE_200_HZ = 0b11,
};

enum QMC5883PRange {
  QMC5883P_RANGE_30_G = 0b00,
  QMC5883P_RANGE_12_G = 0b01,
  QMC5883P_RANGE_8_G = 0b10,
  QMC5883P_RANGE_2_G = 0b11,
};

enum QMC5883PMode {
  QMC5883P_MODE_SUSPEND = 0b00,
  QMC5883P_MODE_NORMAL = 0b01,
  QMC5883P_MODE_SINGLE = 0b10,
  QMC5883P_MODE_CONTINUOUS = 0b11,
};

enum QMC5883POversampling {
  QMC5883P_SAMPLING_8 = 0b00,
  QMC5883P_SAMPLING_4 = 0b01,
  QMC5883P_SAMPLING_2 = 0b10,
  QMC5883P_SAMPLING_1 = 0b11,
};

enum QMC5883PNoiselevel {
  QMC5883P_DSAMPLING_1 = 0b00,
  QMC5883P_DSAMPLING_2 = 0b01,
  QMC5883P_DSAMPLING_4 = 0b10,
  QMC5883P_DSAMPLING_8 = 0b11,
};

class QMC5883PComponent : public PollingComponent, public i2c::I2CDevice {
 public:
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override;
  void update() override;
  void loop() override;
  void read_measurement();
  void set_mode();

  void set_datarate(QMC5883PDatarate datarate) { datarate_ = datarate; }
  void set_range(QMC5883PRange range) { range_ = range; }
  void set_mode(QMC5883PMode mode) { mode_ = mode; }
  void set_oversampling(QMC5883POversampling oversampling) { oversampling_ = oversampling; }
  void set_noiselevel(QMC5883PNoiselevel noiselevel) { noiselevel_ = noiselevel; }
  void set_x_sensor(sensor::Sensor *x_sensor) { x_sensor_ = x_sensor; }
  void set_y_sensor(sensor::Sensor *y_sensor) { y_sensor_ = y_sensor; }
  void set_z_sensor(sensor::Sensor *z_sensor) { z_sensor_ = z_sensor; }
  void set_heading_sensor(sensor::Sensor *heading_sensor) { heading_sensor_ = heading_sensor; }

 protected:
  QMC5883PDatarate datarate_{QMC5883P_DATARATE_200_HZ};
  QMC5883PRange range_{QMC5883P_RANGE_30_G};
  QMC5883PMode mode_{QMC5883P_MODE_SINGLE};
  QMC5883POversampling oversampling_{QMC5883P_SAMPLING_8};
  QMC5883PNoiselevel noiselevel_{QMC5883P_DSAMPLING_8};
  sensor::Sensor *x_sensor_{nullptr};
  sensor::Sensor *y_sensor_{nullptr};
  sensor::Sensor *z_sensor_{nullptr};
  sensor::Sensor *heading_sensor_{nullptr};
  enum ErrorCode {
    NONE = 0,
    COMMUNICATION_FAILED,
  } error_code_;
  i2c::ErrorCode read_bytes_16_le_(uint8_t a_register, uint16_t *data, uint8_t len = 1);
  HighFrequencyLoopRequester high_freq_;
};

}  // namespace qmc5883p
}  // namespace esphome
