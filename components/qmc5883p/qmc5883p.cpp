#include "qmc5883p.h"
#include "esphome/core/application.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"
#include <cmath>

namespace esphome {
namespace qmc5883p {

static const char *const TAG = "qmc5883p";
static const uint8_t QMC5883P_ADDRESS = 0x2C;

static const uint8_t QMC5883P_REGISTER_DATA_X_LSB = 0x01;
static const uint8_t QMC5883P_REGISTER_DATA_X_MSB = 0x02;
static const uint8_t QMC5883P_REGISTER_DATA_Y_LSB = 0x03;
static const uint8_t QMC5883P_REGISTER_DATA_Y_MSB = 0x04;
static const uint8_t QMC5883P_REGISTER_DATA_Z_LSB = 0x05;
static const uint8_t QMC5883P_REGISTER_DATA_Z_MSB = 0x06;
static const uint8_t QMC5883P_REGISTER_STATUS = 0x09;
static const uint8_t QMC5883P_REGISTER_CONTROL_1 = 0x0A;
static const uint8_t QMC5883P_REGISTER_CONTROL_2 = 0x0B;
static const uint8_t QMC5883P_REGISTER_PERIOD = 0x0C;

void QMC5883PComponent::setup() {
  ESP_LOGD(TAG,"Initializing...");
  // From setup examples in datasheet register 29H is listed.....
  if (!this->write_byte(0x29,0x06)) {
    this->error_code_ = COMMUNICATION_FAILED;
    this->mark_failed();
    return;
  }
  /* removed, just for initial testing
  if (!this->write_byte(0x0B,0x08)) {
    this->error_code_ = COMMUNICATION_FAILED;
    this->mark_failed();
    return;
  }
  if (!this->write_byte(0x0A,0xCD)) {
    this->error_code_ = COMMUNICATION_FAILED;
    this->mark_failed();
    return;
  } */

  // First set second register for Range, no soft reset/self test
  // Set/reset mode initial both, later make option
  uint8_t control_2 = 0;
  control_2 |= 0b0 << 7;  // SOFT_RST (Soft Reset) -> 0b00=disabled, 0b01=enabled
  control_2 |= 0b0 << 6;  // SELF_TEST -> 0b00=disabled, 0b01=enabled
  control_2 |= this->range_ << 2;  // RNG (Range 2,8,12,30G)
  control_2 |= 0b00 << 0;  // SET/RESET MODE -> 0b00=both on, 0b01=Set only, none
  if (!this->write_byte(QMC5883P_REGISTER_CONTROL_2, control_2)) {
    this->error_code_ = COMMUNICATION_FAILED;
    this->mark_failed();
    return;
  }
  ESP_LOGD(TAG,"Control Register 2: %x",control_2);

  // Register to set mode, datarate, oversampling and doensampling (noiselevel)
  uint8_t control_1 = 0;
  control_1 |= this->mode_ << 0;  // MODE (Mode) -> 0b00=standby, 0b01=continuous
  control_1 |= this->datarate_ << 2;
  control_1 |= this->oversampling_ << 4;
  control_1 |= this->noiselevel_ << 6;
  if (!this->write_byte(QMC5883P_REGISTER_CONTROL_1, control_1)) {
    this->error_code_ = COMMUNICATION_FAILED;
    this->mark_failed();
    return;
  }
  ESP_LOGD(TAG,"Control Register 1: %x",control_1);

  if (this->get_update_interval() < App.get_loop_interval()) {
    high_freq_.start();
  }
}
void QMC5883PComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "QMC5883P:");
  LOG_I2C_DEVICE(this);
  if (this->error_code_ == COMMUNICATION_FAILED) {
    ESP_LOGE(TAG, ESP_LOG_MSG_COMM_FAIL);
  }
  LOG_UPDATE_INTERVAL(this);

  LOG_SENSOR("  ", "X Axis", this->x_sensor_);
  LOG_SENSOR("  ", "Y Axis", this->y_sensor_);
  LOG_SENSOR("  ", "Z Axis", this->z_sensor_);
  LOG_SENSOR("  ", "Heading", this->heading_sensor_);
}

float QMC5883PComponent::get_setup_priority() const { return setup_priority::DATA; }

void QMC5883PComponent::update() {
  i2c::ErrorCode err;
  uint8_t status = false;

  // Status byte gets cleared when data is read, so we have to read this first.
  // Just read whole register, no check on DRDY/OVFL yet
  if (ESPHOME_LOG_LEVEL >= ESPHOME_LOG_LEVEL_DEBUG) {
    err = this->read_register(QMC5883P_REGISTER_STATUS, &status, 1);
    if (err != i2c::ERROR_OK) {
      this->status_set_warning(str_sprintf("status read failed (%d)", err).c_str());
      return;
    }
//  ESP_LOGD(TAG, "Read Status Register for Data Ready %d", status);
  }

  uint16_t raw[3] = {0};
  // Z must always be requested for QMC5883L, otherwise the data registers will remain
  // locked against updates. QMC5883P does not need this acc. to datasheet but keep.
  // Skipping the Y axis if X and Z are needed actually requires an additional byte of comms.
  // Starting partway through the axes does save you traffic.
  uint8_t start, dest;
  if (this->heading_sensor_ != nullptr || this->x_sensor_ != nullptr) {
    start = QMC5883P_REGISTER_DATA_X_LSB;
    dest = 0;
  } else if (this->y_sensor_ != nullptr) {
    start = QMC5883P_REGISTER_DATA_Y_LSB;
    dest = 1;
  } else {
    start = QMC5883P_REGISTER_DATA_Z_LSB;
    dest = 2;
  }
  err = this->read_bytes_16_le_(start, &raw[dest], 3 - dest);
  if (err != i2c::ERROR_OK) {
    this->status_set_warning(str_sprintf("mag read failed (%d)", err).c_str());
    return;
  }
//ESP_LOGD(TAG,"Raw data: X %d Y %d Z %d", raw[0],raw[1],raw[2]);

  // Measurement is 2s complement. So for 2Gauss 2000/32767miligauss per bit
  float mg_per_bit;
  switch (this->range_) {
    case QMC5883P_RANGE_2_G:
      mg_per_bit = 2000.0f/32767;
      break;
    case QMC5883P_RANGE_8_G:
      mg_per_bit = 8000.0f/32767;
      break;
    case QMC5883P_RANGE_12_G:
      mg_per_bit = 12000.0f/32767;
      break;
    case QMC5883P_RANGE_30_G:
      mg_per_bit = 30000.0f/32767;
      break;
    default:
      mg_per_bit = NAN;
  }

  // Gauss to Tesla is /10.000, so mg is 10 µT
  const float x = int16_t(raw[0]) * mg_per_bit * 0.1f;
  const float y = int16_t(raw[1]) * mg_per_bit * 0.1f;
  const float z = int16_t(raw[2]) * mg_per_bit * 0.1f;

  float heading = atan2f(0.0f - x, y) * 180.0f / M_PI;

  ESP_LOGD(TAG, "Got x=%0.02fµT y=%0.02fµT z=%0.02fµT heading=%0.01f° status=%u", x, y, z, heading,
          status);

  if (this->x_sensor_ != nullptr)
    this->x_sensor_->publish_state(x);
  if (this->y_sensor_ != nullptr)
    this->y_sensor_->publish_state(y);
  if (this->z_sensor_ != nullptr)
    this->z_sensor_->publish_state(z);
  if (this->heading_sensor_ != nullptr)
    this->heading_sensor_->publish_state(heading);
}

i2c::ErrorCode QMC5883PComponent::read_bytes_16_le_(uint8_t a_register, uint16_t *data, uint8_t len) {
  i2c::ErrorCode err = this->read_register(a_register, reinterpret_cast<uint8_t *>(data), len * 2);
  if (err != i2c::ERROR_OK)
    return err;
  for (size_t i = 0; i < len; i++)
    data[i] = convert_little_endian(data[i]);
  return err;
}

}  // namespace qmc5883p
}  // namespace esphome
