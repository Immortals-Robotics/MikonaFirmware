# MikonaFirmware

Mikona firmware running on its local PIC processor.

## I2C Protocol

Mikona exposes a register-based I2C slave interface. All transactions begin with a register address byte, followed by data bytes to write or read.

**Device ID:** `0x54`

---

### Register Map

| Address | Name     | Access     | Description                             |
|---------|----------|------------|-----------------------------------------|
| `0x01`  | `DEV_ID` | Read       | Device identification byte (`0x54`)     |
| `0x02`  | `STATUS` | Read/Write | Status and control flags                |
| `0x03`  | `V_OUT`  | Read       | Capacitor voltage (scaled, see below)   |
| `0x04`  | `KICK_A` | Write      | Fire solenoid A (2 bytes)               |
| `0x05`  | `KICK_B` | Write      | Fire solenoid B (2 bytes)               |
| `0x06`  | `FAULT`  | Read/Write | Fault cause bitfield                    |

---

### STATUS Register (`0x02`)

```text
Bit:  7  6  5  4  |  3   |  2  |    1    |   0
      X  X  X  X  |FAULT |DONE |DISCHARGE|CHARGE
```

| Bit | Name        | R/W | Description                                                                  |
|-----|-------------|-----|------------------------------------------------------------------------------|
| 0   | `CHARGE`    | R/W | Set `1` to start charging capacitors. Clear to stop.                         |
| 1   | `DISCHARGE` | R/W | Set `1` to discharge capacitors.                                             |
| 2   | `DONE`      | R   | `1` when capacitor voltage is within 10V of the learned peak.                |
| 3   | `FAULT`     | R   | Summary fault flag. Read-only — clear by writing `0x00` to `FAULT` register. |

**Write mask:** bits 0 (`CHARGE`) and 1 (`DISCHARGE`) only. Bits 2 (`DONE`) and 3 (`FAULT`) are read-only.

#### Charging behavior

The firmware maintains capacitor charge automatically:

1. Master sets `CHARGE=1` to begin the first charge cycle.
2. Once the IC signals done, the firmware records the peak voltage.
3. If voltage drops more than 10V below the learned peak, a new charge cycle is triggered automatically — no action required from the master.
4. `DONE=1` means the capacitors are charged to near-peak voltage; `DONE=0` means a charge cycle is in progress.
5. Set `CHARGE=0` to stop charging entirely.

The learned peak voltage is the median of the last 5 charge-done readings, providing noise rejection.

#### Invalid command combinations

The following combinations are rejected and set `FAULT`:

- `CHARGE=1` and `DISCHARGE=1` simultaneously.
- Sending a `KICK_A` or `KICK_B` command while `DISCHARGE=1`.

---

### V_OUT Register (`0x03`)

Read-only. Capacitor voltage scaled as:

```text
V = register_value * (4.096 * 101 / 1023) volts
```

The register saturates at `255`. Updated continuously via ADC interrupt.

---

### KICK_A / KICK_B Registers (`0x04`, `0x05`)

Write-only, 2 bytes. Writing a non-zero 16-bit value fires the corresponding solenoid for the specified pulse duration. The two bytes must be written in a single I2C transaction.

Kick A and Kick B are mutually exclusive at the hardware level — issuing one disables the other's output pin.

---

### FAULT Register (`0x06`)

```text
Bit:  7  6  5  |       4        |       3        |        2        |       1        |      0
      X  X  X  |KICK_B_NO_DROP  |KICK_A_NO_DROP  |DISCHARGE_STUCK  |CHARGE_TIMEOUT  |INVALID_CMD
```

| Bit | Name               | Description                                                                                        |
|-----|--------------------|----------------------------------------------------------------------------------------------------|
| 0   | `INVALID_CMD`      | Invalid command: charge+discharge simultaneously, or kick while discharging.                       |
| 1   | `CHARGE_TIMEOUT`   | Charge cycle did not complete within 5s. Possible causes: disconnected capacitor, failed IC.       |
| 2   | `DISCHARGE_STUCK`  | Voltage did not drop by 5V per second during discharge. Possible causes: stuck FET, open circuit.  |
| 3   | `KICK_A_NO_DROP`   | Voltage did not drop by 10V within 50ms of kick A. Possible causes: stuck FET, open circuit.       |
| 4   | `KICK_B_NO_DROP`   | Voltage did not drop by 10V within 50ms of kick B. Possible causes: stuck FET, open circuit.       |

Multiple bits may be set simultaneously. The `FAULT` bit in the `STATUS` register is a summary flag set whenever any fault bit is set.

**To clear:** write `0x00` to the `FAULT` register. This clears all fault bits, the `STATUS.FAULT` flag, and restores the green LED.

---

### LED Indicator

| Color | Meaning                                            |
|-------|----------------------------------------------------|
| Green | Normal operation                                   |
| Red   | Fault detected — read `FAULT` register for cause   |
