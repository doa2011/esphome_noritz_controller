import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, uart
from esphome.const import (
    CONF_ID,
    CONF_TEMPERATURE,
    DEVICE_CLASS_TEMPERATURE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
)

DEPENDENCIES = ["uart"]
AUTO_LOAD = ["sensor"]

CONF_POWER = "power"
CONF_UART_ID = "uart_id"

noritz_ns = cg.esphome_ns.namespace("noritz")

NoritzComponent = noritz_ns.class_(
    "NoritzComponent",
    cg.Component,
    uart.UARTDevice,
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(NoritzComponent),
            cv.Required(CONF_UART_ID): cv.use_id(uart.UARTComponent),
            cv.Required(CONF_POWER): sensor.sensor_schema(
                accuracy_decimals=0,
            ),
            cv.Required(CONF_TEMPERATURE): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)

async def to_code(config):
    parent = await cg.get_variable(config[CONF_UART_ID])

    var = cg.new_Pvariable(config[CONF_ID], parent)
    await cg.register_component(var, config)

    power_sensor = await sensor.new_sensor(config[CONF_POWER])
    temperature_sensor = await sensor.new_sensor(
        config[CONF_TEMPERATURE]
    )

    cg.add(var.set_power_sensor(power_sensor))
    cg.add(var.set_temperature_sensor(temperature_sensor))
