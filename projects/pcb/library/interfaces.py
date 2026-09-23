from skidl import Net

class PowerDomain:
    """Represents a power domain with a voltage net and a ground net."""
    def __init__(self, name_prefix="", vcc_name=None, gnd_name="GND", gnd_net=None):
        self.vcc = Net(vcc_name if vcc_name else f"{name_prefix}VCC")
        self.gnd = gnd_net if gnd_net is not None else Net(gnd_name)  # GND is typically shared

class I2CBus:
    """Represents an I2C bus with SDA and SCL nets."""
    def __init__(self, name="I2C"):
        self.sda = Net(f"{name}_SDA")
        self.scl = Net(f"{name}_SCL")

class UARTBus:
    """Represents a UART bus with TX and RX nets."""
    def __init__(self, name="UART"):
        self.tx = Net(f"{name}_TX")
        self.rx = Net(f"{name}_RX")

class USBBus:
    """Represents a USB D+/D- bus."""
    def __init__(self, name="USB"):
        self.dp = Net(f"{name}_DP")
        self.dn = Net(f"{name}_DN")

class SPIBus:
    """Represents an SPI bus."""
    def __init__(self, name="SPI"):
        self.sck = Net(f"{name}_SCK")
        self.mosi = Net(f"{name}_MOSI")
        self.miso = Net(f"{name}_MISO")
        self.cs = Net(f"{name}_CS")

class StrappingPins:
    """Represents core strapping/control pins for MCU."""
    def __init__(self):
        self.en = Net("EN_BTN")
        self.boot = Net("BOOT_BTN")
