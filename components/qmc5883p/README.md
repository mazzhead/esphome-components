The `qmc5883p` allows you to use your QMC5883P triple-axis magnetometers
([datasheet](https://www.lcsc.com/datasheet/C2847467.pdf)) with
ESPHome. This sensor is very simular to the HMC5883L and QMC5883L sensor and is often found
as a knock off replacement. The QMC5883P sensor configuration differs. The I²C Bus is required to be set up in
your configuration for this sensor to work.

```yaml
# Example configuration entry
sensor:
  - platform: qmc5883p
    address: 0x2C
    field_strength_x:
      name: "QMC5883P Field Strength X"
    field_strength_y:
      name: "QMC5883P Field Strength Y"
    field_strength_z:
      name: "QMC5883P Field Strength Z"
    heading:
      name: "QMC5883P Heading"
    range: 2G
    oversampling: 8x
    noiselevel: 1x
    update_interval: 60s
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

- **range** (*Optional*): The range parameter for the sensor.
- **oversampling** (*Optional*): The oversampling parameter for the sensor.
- **update_interval** (*Optional*, [Time](#config-time)): The interval to check the sensor. Defaults to `60s`.
- **id** (*Optional*, ID): Manually specify the ID used for code generation.

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

## Noise level  Options

By default, the QMC5883P uses another filter for better noise performance, this depthh is 8x as default.
You can, however, configure this amount. Possible values:

- `8x` (default)
- `4x`
- `2x`
- `1x`
