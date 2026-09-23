from skidl import Part

from library.interfaces import I2CBus, PowerDomain

R_0603 = "Resistor_SMD:R_0603_1608Metric"
HDR_1X04 = "Connector_PinHeader_2.54mm:PinHeader_1x04_P2.54mm_Vertical"


def add_i2c_bus(
    pwr: PowerDomain,
    i2c: I2CBus,
    *,
    add_pullups: bool = True,
    add_header: bool = True,
    pullup_value: str = "4.7k",
):
    """
    Adds the shared I2C bus infrastructure: pull-ups and optional breakout header.
    """
    v_logic = pwr.vcc
    gnd = pwr.gnd

    if add_pullups:
        r_scl = Part("Device", "R", value=pullup_value, ref="R_I2C_SCL", footprint=R_0603)
        r_sda = Part("Device", "R", value=pullup_value, ref="R_I2C_SDA", footprint=R_0603)
        v_logic += r_scl[1]
        i2c.scl += r_scl[2]
        v_logic += r_sda[1]
        i2c.sda += r_sda[2]

    if add_header:
        j_i2c = Part("Connector_Generic", "Conn_01x04", ref="J_I2C", footprint=HDR_1X04)
        gnd += j_i2c[1]
        v_logic += j_i2c[2]
        i2c.sda += j_i2c[3]
        i2c.scl += j_i2c[4]
