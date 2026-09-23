# Profile: Sensors (I2C)

## Components

### BME280 (Environmental)
- **Function**: Temperature, Humidity, Pressure sensor.
- **Interface**: I2C (default address 0x76 when SDO=GND).
- **Power**: 1.71V - 3.6V.
- **Layout Constraints**:
    - Place away from heat sources (MCU, LDO, Battery Charger).
    - Provide a thermal isolation "cut" or gap in the copper pour if extreme precision is needed.
    - Keep decoupling capacitor (100nF) as close to VDD pin as possible.

### MPU-6050 (IMU)
- **Function**: 3-axis Accelerometer + 3-axis Gyroscope.
- **Interface**: I2C (default address 0x68 when AD0=GND).
- **Power**: 2.375V - 3.46V (Commonly 3.3V).
- **Layout Constraints**:
    - Place in the center of the PCB for best balance.
    - Align axes with the PCB edges.
    - REGOUT requires a 2.2uF ceramic capacitor to GND for stability.

## I2C Bus Standards
- **Pull-ups**: Required (typically 4.7k or 10k for 400kHz Fast Mode).
- **Crosstalk**: Avoid running SDA/SCL parallel to high-speed switching lines (e.g., LED PWM, Boost output).
