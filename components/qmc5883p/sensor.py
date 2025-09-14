import esphome.codegen as cg
from esphome.components import i2c, sensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_ADDRESS,
    CONF_FIELD_STRENGTH_X,
    CONF_FIELD_STRENGTH_Y,
    CONF_FIELD_STRENGTH_Z,
    CONF_HEADING,
    CONF_ID,
    CONF_OVERSAMPLING,
    CONF_NOISE_LEVEL,
    CONF_RANGE,
    CONF_MODE,
    CONF_TEMPERATURE,
    CONF_UPDATE_INTERVAL,
    ICON_MAGNET,
    ICON_SCREEN_ROTATION,
    STATE_CLASS_MEASUREMENT,
    UNIT_DEGREES,
    UNIT_MICROTESLA,
)

DEPENDENCIES = ["i2c"]

qmc5883p_ns = cg.esphome_ns.namespace("qmc5883p")

QMC5883PComponent = qmc5883p_ns.class_(
    "QMC5883PComponent", cg.PollingComponent, i2c.I2CDevice
)

QMC5883PDatarate = qmc5883p_ns.enum("QMC5883PDatarate")
QMC5883PDatarates = {
    10: QMC5883PDatarate.QMC5883P_DATARATE_10_HZ,
    50: QMC5883PDatarate.QMC5883P_DATARATE_50_HZ,
    100: QMC5883PDatarate.QMC5883P_DATARATE_100_HZ,
    200: QMC5883PDatarate.QMC5883P_DATARATE_200_HZ,
}

QMC5883PRange = qmc5883p_ns.enum("QMC5883PRange")
QMC5883P_RANGES = {
    30: QMC5883PRange.QMC5883P_RANGE_30_G,
    12: QMC5883PRange.QMC5883P_RANGE_12_G,
    8: QMC5883PRange.QMC5883P_RANGE_8_G,
    2: QMC5883PRange.QMC5883P_RANGE_2_G,
}

QMC5883PMode = qmc5883p_ns.enum("QMC5883PMode")
QMC5883P_MODES = {
    0: QMC5883PMode.QMC5883P_MODE_SUSPEND,
    1: QMC5883PMode.QMC5883P_MODE_NORMAL,
    2: QMC5883PMode.QMC5883P_MODE_SINGLE,
    3: QMC5883PMode.QMC5883P_MODE_CONTINUOUS,
}

QMC5883POversampling = qmc5883p_ns.enum("QMC5883POversampling")
QMC5883POversamplings = {
    8: QMC5883POversampling.QMC5883P_SAMPLING_8,
    4: QMC5883POversampling.QMC5883P_SAMPLING_4,
    2: QMC5883POversampling.QMC5883P_SAMPLING_2,
    1: QMC5883POversampling.QMC5883P_SAMPLING_1,
}

QMC5883PNoiselevel = qmc5883p_ns.enum("QMC5883PNoiselevel")
QMC5883PNoiselevels = {
    8: QMC5883PNoiselevel.QMC5883P_DSAMPLING_8,
    4: QMC5883PNoiselevel.QMC5883P_DSAMPLING_4,
    2: QMC5883PNoiselevel.QMC5883P_DSAMPLING_2,
    1: QMC5883PNoiselevel.QMC5883P_DSAMPLING_1,
}


def validate_enum(enum_values, units=None, int=True):
    _units = []
    if units is not None:
        _units = units if isinstance(units, list) else [units]
        _units = [str(x) for x in _units]
    enum_bound = cv.enum(enum_values, int=int)

    def validate_enum_bound(value):
        value = cv.string(value)
        for unit in _units:
            if value.endswith(unit):
                value = value[: -len(unit)]
                break
        return enum_bound(value)

    return validate_enum_bound


field_strength_schema = sensor.sensor_schema(
    unit_of_measurement=UNIT_MICROTESLA,
    icon=ICON_MAGNET,
    accuracy_decimals=1,
    state_class=STATE_CLASS_MEASUREMENT,
)
heading_schema = sensor.sensor_schema(
    unit_of_measurement=UNIT_DEGREES,
    icon=ICON_SCREEN_ROTATION,
    accuracy_decimals=1,
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(QMC5883PComponent),
            cv.Optional(CONF_ADDRESS): cv.i2c_address,
            cv.Optional(CONF_RANGE, default="30G"): validate_enum(
                QMC5883P_RANGES, units=["Gauss", "G"]
            ),
            cv.Optional(CONF_MODE, default=1): validate_enum(
                QMC5883P_MODES
            ),
            cv.Optional(CONF_OVERSAMPLING, default="8x"): validate_enum(
                QMC5883POversamplings, units="x"
            ),
            cv.Optional(CONF_NOISE_LEVEL, default="8x"): validate_enum(
                QMC5883PNoiselevels, units="x"
            ),
            cv.Optional(CONF_FIELD_STRENGTH_X): field_strength_schema,
            cv.Optional(CONF_FIELD_STRENGTH_Y): field_strength_schema,
            cv.Optional(CONF_FIELD_STRENGTH_Z): field_strength_schema,
            cv.Optional(CONF_HEADING): heading_schema,
        }
    )
    .extend(cv.polling_component_schema("60s"))
    .extend(i2c.i2c_device_schema(0x2C))
)

# Output datarate is automatically matched to  update interval
def auto_data_rate(config):
    interval_sec = config[CONF_UPDATE_INTERVAL].total_milliseconds / 1000
    interval_hz = 1.0 / interval_sec
    for datarate in sorted(QMC5883PDatarates.keys()):
        if float(datarate) >= interval_hz:
            return QMC5883PDatarates[datarate]
    return QMC5883PDatarates[200]

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    cg.add(var.set_oversampling(config[CONF_OVERSAMPLING]))
    cg.add(var.set_noiselevel(config[CONF_NOISE_LEVEL]))
    cg.add(var.set_datarate(auto_data_rate(config)))
    cg.add(var.set_range(config[CONF_RANGE]))
    cg.add(var.set_mode(config[CONF_MODE]))
    if CONF_FIELD_STRENGTH_X in config:
        sens = await sensor.new_sensor(config[CONF_FIELD_STRENGTH_X])
        cg.add(var.set_x_sensor(sens))
    if CONF_FIELD_STRENGTH_Y in config:
        sens = await sensor.new_sensor(config[CONF_FIELD_STRENGTH_Y])
        cg.add(var.set_y_sensor(sens))
    if CONF_FIELD_STRENGTH_Z in config:
        sens = await sensor.new_sensor(config[CONF_FIELD_STRENGTH_Z])
        cg.add(var.set_z_sensor(sens))
    if CONF_HEADING in config:
        sens = await sensor.new_sensor(config[CONF_HEADING])
        cg.add(var.set_heading_sensor(sens))
