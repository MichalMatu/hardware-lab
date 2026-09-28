import pcbnew

BOARD_LEFT_MM = 0.0
BOARD_TOP_MM = 0.0
BOARD_RIGHT_MM = 27.94
BOARD_BOTTOM_MM = 48.26
ROW_SPACING_MM = 25.40
BOARD_CENTER_X_MM = (BOARD_LEFT_MM + BOARD_RIGHT_MM) / 2.0
LEFT_ROW_X_MM = BOARD_CENTER_X_MM - (ROW_SPACING_MM / 2.0)
RIGHT_ROW_X_MM = BOARD_CENTER_X_MM + (ROW_SPACING_MM / 2.0)
TOP_PIN_Y_MM = 1.27
PIN_PITCH_MM = 2.54
PIN_COUNT = 19
EDGE_WIDTH_MM = 0.1
EXACT_SIZE_EDGE_CLEARANCE_MM = 0.4

PLACEMENTS = {
    # ── Host sockets (F.Cu, fixed by spec) ───────────────────────────────
    "J_HOST_L": (LEFT_ROW_X_MM, TOP_PIN_Y_MM, 0),
    "J_HOST_R": (RIGHT_ROW_X_MM, TOP_PIN_Y_MM, 0),

    # ── PMIC core (B.Cu) ────────────────────────────────────────────────
    # Pinout after rot=180: LEFT side y=15.2..18.8 = pins 21..30 (FB1, LX1,
    # VIN1, VIN2, LX2, FB2, VBackup, VRTC, PWROK, PWRON); BOTTOM side
    # x=11.7..16.3 = pins 31..40 (TS, GPIO1, BAT, VSYS, SW, VMID, VBUS, IRQ,
    # SDA, SCL); RIGHT side bottom-to-top = pins 1..10 (CHGLED, VREF, GND,
    # FB3, LX3, VIN3, VIN4, LX4, FB4, CPULDOS); TOP side right-to-left =
    # pins 11..20 (DC4SW, BLDO1, BLDOIN, BLDO2, ALDO4, ALDO3, ALDOIN,
    # ALDO1, ALDO2, DC1SW).
    "U_PMIC": (13.97, 16.5, 180),

    # LEFT of PMIC – DCDC1 inductor + cap (LX1 pin 22), VIN1/VIN2, VRTC
    "L_DCDC1":     (8.5, 16.5, -90),
    "C_DCDC1":     (8.5, 21.0, 0),
    "C_PMIC_VIN1": (5.5, 15.5, 90),
    "C_PMIC_VIN2": (3.8, 15.5, 90),
    "C_PMIC_VRTC": (5.2, 18.5, 90),

    # RIGHT of PMIC – VREF (pin 2), VIN3 (pin 6), VIN4 (pin 7)
    "C_PMIC_VREF": (19.0, 17.5, 0),
    "C_PMIC_VIN3": (19.0, 15.5, 0),
    "C_PMIC_VIN4": (19.0, 13.5, 0),

    # ABOVE PMIC – ALDOIN (pin 17), BLDOIN (pin 13)
    "C_PMIC_ALDOIN": (13.0, 12.0, 0),
    "C_PMIC_BLDOIN": (16.0, 12.0, 0),

    # BELOW PMIC – SW inductor (pins 35/36) and VSYS/VBUS/VMID/BAT caps
    "L_PMIC_SW":    (13.5, 21.5, 180),
    "C_PMIC_BAT":   (8.0, 25.5, 90),
    "C_PMIC_SYSA":  (10.5, 25.5, 90),
    "C_PMIC_SYSB":  (13.5, 25.5, 90),
    "C_PMIC_VMID":  (15.5, 25.5, 90),
    "C_PMIC_VBUSA": (17.5, 25.5, 90),
    "C_PMIC_VBUSB": (19.5, 25.5, 90),

    # ── I2C pull-ups (B.Cu, y=29.0) ─────────────────────────────────────
    "R_I2C_SCL":  (5.0, 29.0, 0),
    "R_I2C_SDA":  (8.0, 29.0, 0),
    "R_PMIC_IRQ": (11.0, 29.0, 0),

    # ── HOST 3V3 load switch (B.Cu, y=30.5) ───────────────────────────────
    "U_HOST_3V3_SW":  (15.0, 30.5, 0),
    "R_HOST_3V3_OFF": (18.5, 30.5, 90),
    "R_HOST_3V3_ON":  (20.5, 30.5, 90),

    # ── Charge-LED indicator (B.Cu, left column x=5.1 / 4.8) ────────────
    "R_CHG_G":   (4.8, 31.5, 0),
    "R_CHG_PD":  (4.8, 33.0, 0),
    "Q_CHG":     (5.1, 35.5, 180),
    "D_CHG":     (4.8, 38.0, 0),
    "R_LED_CHG": (4.8, 40.0, 0),

    # ── PWRON button cluster (B.Cu) ─────────────────────────────────────
    "R_PWRON":  (15.2, 33.5, 180),
    "C_PWRON":  (15.2, 35.0, 180),
    "SW_PWRON": (10.5, 37.0, 90),

    # ── Battery connector (B.Cu, bottom) ────────────────────────────────
    "J_BAT": (14.5, 43.5, 0),

    # ── Test pads (B.Cu, edges) ─────────────────────────────────────────
    "TP_GND":   (4.8, 42.3, 0),
    "TP_TS":    (4.8, 44.7, 0),
    "TP_VSYS":  (23.0, 38.0, 0),
    "TP_VRTC":  (23.0, 40.5, 0),
    "TP_VREF":  (23.0, 43.0, 0),
    "TP_VMID":  (23.0, 45.5, 0),
    "TP_PWRON": (20.5, 45.5, 0),
}

BOTTOM_SIDE_REFS = {
    "U_HOST_3V3_SW",
    "R_HOST_3V3_OFF",
    "R_HOST_3V3_ON",
    "R_I2C_SCL",
    "R_I2C_SDA",
    "R_PMIC_IRQ",
    "R_LED_CHG",
    "D_CHG",
    "Q_CHG",
    "R_CHG_G",
    "R_CHG_PD",
    "SW_PWRON",
    "R_PWRON",
    "C_PWRON",
    "U_PMIC",
    "C_PMIC_VBUSA",
    "C_PMIC_VBUSB",
    "C_PMIC_VMID",
    "L_PMIC_SW",
    "C_PMIC_SYSA",
    "C_PMIC_SYSB",
    "C_PMIC_BAT",
    "C_PMIC_VREF",
    "C_PMIC_VRTC",
    "C_PMIC_VIN3",
    "L_DCDC1",
    "C_DCDC1",
    "C_PMIC_VIN4",
    "C_PMIC_ALDOIN",
    "C_PMIC_BLDOIN",
    "C_PMIC_VIN1",
    "C_PMIC_VIN2",
}

SILKLESS_REFS = {
    "J_HOST_L",
    "J_HOST_R",
    "U_PMIC",
}

FOUR_LAYER_COPPER_LAYERS = (
    pcbnew.F_Cu,
    pcbnew.In1_Cu,
    pcbnew.In2_Cu,
    pcbnew.B_Cu,
)

LAYER_STRATEGY = (
    ("F.Cu", "components and critical hat routing"),
    ("In1.Cu", "5V and 3V3 distribution"),
    ("In2.Cu", "solid GND reference plane (adjacent to B.Cu where PMIC and inductors sit)"),
    ("B.Cu", "U_PMIC, inductors, decouplers and bottom-side signals"),
)

LEFT_PIN_LABELS = [
    ("3V3", "3V3"),
    ("EN", "EN"),
    ("VP", "GPIO36 / VP"),
    ("VN", "GPIO39 / VN"),
    ("34", "GPIO34"),
    ("35", "GPIO35"),
    ("32", "GPIO32"),
    ("33", "GPIO33"),
    ("25", "GPIO25"),
    ("26", "GPIO26"),
    ("27", "GPIO27"),
    ("14", "GPIO14"),
    ("12", "GPIO12"),
    ("GND", "GND"),
    ("13", "GPIO13"),
    ("D2", "FLASH D2 / GPIO9"),
    ("D3", "FLASH D3 / GPIO10"),
    ("CMD", "FLASH CMD / GPIO11"),
    ("5V", "5V"),
]

RIGHT_PIN_LABELS = [
    ("GND", "GND"),
    ("23", "GPIO23 / MOSI"),
    ("22", "GPIO22 / SCL"),
    ("TX", "U0TXD / GPIO1"),
    ("RX", "U0RXD / GPIO3"),
    ("21", "GPIO21 / SDA"),
    ("GND", "GND"),
    ("19", "GPIO19 / MISO"),
    ("18", "GPIO18 / SCK"),
    ("5", "GPIO5"),
    ("17", "GPIO17"),
    ("16", "GPIO16"),
    ("4", "GPIO4"),
    ("0", "GPIO0 / BOOT"),
    ("2", "GPIO2"),
    ("15", "GPIO15"),
    ("D1", "FLASH D1 / GPIO8"),
    ("D0", "FLASH D0 / GPIO7"),
    ("CLK", "FLASH CLK / GPIO6"),
]
