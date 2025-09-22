The `qmc5883p` allows you to use your QMC5883P triple-axis magnetometers
([datasheet](https://www.lcsc.com/datasheet/C2847467.pdf)) with
ESPHome. This sensor is very simular to the HMC5883L and QMC5883L sensor and is often found
as a replacement. The QMC5883P sensor configuration differs from the others. The I²C Bus is required
to be set up in your configuration for this sensor to work.

```yaml
# Example configuration entry
sensor:
  - platform: qmc5883p
    field_strength_x:
      name: "QMC5883P Field Strength X"
    field_strength_y:
      name: "QMC5883P Field Strength Y"
    field_strength_z:
      name: "QMC5883P Field Strength Z"
    heading:
      name: "QMC5883P Heading"
```

## Configuration variables

- **address** (*Optional*, int): Manually specify the I²c address of the sensor. Defaults to `0x2C`.
- **field_strength_x** (*Optional*): The field strength in microtesla along the X-Axis. All options from
  Sensor.
- **field_strength_y** (*Optional*): The field strength in microtesla along the Y-Axis. All options from
  Sensor.
- **field_strength_z** (*Optional*): The field strength in microtesla along the Z-Axis. All options from
  Sensor.
- **heading** (*Optional*): The heading of the sensor in degrees. All options from
  Sensor.
- **mode** (*Optional*): The mode parameter for the sensor.
- **range** (*Optional*): The range parameter for the sensor.
- **oversampling** (*Optional*): The oversampling parameter for the sensor.
- **noise_level** (*Optional*): The downsampling on noiselevel for the sensor
- **update_interval** (*Optional*, [Time](#config-time)): The interval to check the sensor. Defaults to `60s`.
- **id** (*Optional*, ID): Manually specify the ID used for code generation.

## Mode Options

By default, the QMC5883 sensor will run in Single Mode. Possible mode values:

- `0` : Suspend Mode, no measurements and lowest power.
- `1` : Normal Mode, measurement interval is determined by Output rate. Higher power consumption
- `2` : Single Mode (default), measurement is requested once every update_interval. Lower power consumption.
- `3` : Continuous Mode, continuous measurements, as fast as possible. Highest power consumption.

## Output data rate

Although this is adjustable on the sensor, this rate is depending on the update_interval when not running in Continuous Mode.
Thus it is calculated based on the available rates of 10, 50, 100 and 200Hz.
In Continuous Mode it is set to the maximum of 200Hz as the ESP will probably be the limiting factor.
With ESP8266MOD maximum achieved around 25 readings per second. ESP32S3 about 50 readings per second.

For Single Mode it is useless as only a single reading is done. The frequency is then determined by the 
update_interval, e.g. 50ms will result in  data rate of about 20Hz if the microcontroller is capable of
handling this rate. 

## Range Options

By default, the QMC5883P sensor measurement range is 30 Gauss. You can, however,
configure this amount. Possible values:

- `30G` (default)
- `12G`
- `8G`
- `2G`

## Oversampling Ratio Options

By default, the QMC5883P sensor uses these to control bandwidth of an internal digital filter. Larger values
lead to smaller filter bandwidth, less in-band noise and higher power consumption. Default is 8x. You can, however,
configure this amount. Possible sampling values:

- `8x` (default)
- `4x`
- `2x`
- `1x`

## Noiselevel  Options

By default, the QMC5883P uses another filter for better noise performance, this depth is 8x as default.
You can, however, configure this amount. Possible values:

- `8x` (default)
- `4x`
- `2x`
- `1x`

## Self-test

The sensor has the possibility for self-test, not (yet) implemented.
