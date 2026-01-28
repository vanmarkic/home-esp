"""ESPHome Example Bridge Component (RF433)."""
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

CODEOWNERS = ["@dragan"]
DEPENDENCIES = []
AUTO_LOAD = ["binary_sensor"]

# Namespace
home_esp_ns = cg.esphome_ns.namespace("home_esp")
ExampleBridgeComponent = home_esp_ns.class_(
    "ExampleBridgeComponent", cg.Component
)

# Configuration keys
CONF_PULSE_LENGTH = "pulse_length"
CONF_TOLERANCE = "tolerance"
CONF_MOTION_CODE = "motion_code"

# Configuration schema
CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(ExampleBridgeComponent),
        cv.Optional(CONF_PULSE_LENGTH, default=350): cv.int_range(min=100, max=1000),
        cv.Optional(CONF_TOLERANCE, default=25): cv.int_range(min=5, max=50),
        cv.Optional(CONF_MOTION_CODE, default=0): cv.uint32_t,
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    """Generate C++ code for the component."""
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    cg.add(var.set_pulse_length(config[CONF_PULSE_LENGTH]))
    cg.add(var.set_tolerance(config[CONF_TOLERANCE]))
    cg.add(var.set_motion_code(config[CONF_MOTION_CODE]))
