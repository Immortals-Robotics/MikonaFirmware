# MikonaFirmware audit — July 2026

Scope: all hand-written sources (`main.c`, `mikona.c/h`, `protocol.c/h`), the MCC-generated
drivers as configured, the README/protocol doc, and the full commit history (28 commits,
2023 baseline + the pre-competition burst of March–April 2026).

## Verdict

The code is in better shape than "aggressively hacked" suggests — it's small (~500 lines of
application code, 56% flash / 21% RAM used), the supervision ideas (charge maintenance,
fault detection) are sound, and repo hygiene is fine. But the architecture is genuinely
patched-together: all logic lives in interrupt handlers, the charge state machine is spread
across three interrupt contexts, and there is **one serious latent hazard** (sleep freezing
every timer, including mid-kick-pulse) plus several smaller bugs and significant
documentation drift.

**Recommendation: rewrite the application layer** (mikona.c + protocol.c) around a
main-loop/event architecture, keeping the external I2C register protocol (with small
additive changes), the PWM one-shot kick mechanism, and the MCC drivers. That's a weekend-
sized rewrite, not a ground-up one. Details in §5.

---

## 1. What the firmware does today

PIC16F15224 @ 1 MHz HFINTOSC, acting as an I2C slave (7-bit address `0x2B`, hardcoded in
`i2c1_slave.c:50`) on a kicker board built around an LT3750 capacitor charger and two
solenoid FETs.

- **Timebase**: TMR0 overflows every 10 ms (`0xF63C` reload @ FOSC/4) → increments
  `g_time_ms`, kicks the WDT, and hardware-triggers an ADC conversion (`ADACT = TMR0`).
- **Voltage**: ADC reads the cap divider against FVR 4.096 V; `v_out` holds calibrated
  volts, 1 V/LSB, saturating at 255.
- **Charging**: LT3750 does one cycle per CHARGE assertion; firmware relearns the peak
  voltage (median of last 5 cycle peaks) and re-triggers cycles when voltage sags >10 V
  below the learned peak. DONE pin edge arrives via IOC.
- **Kicking**: TMR2 in one-shot mode + PWM3/PWM4 duty = hardware-timed pulse, 16 µs
  resolution, ~16 ms max. Solenoid select is done by tri-stating the other channel's pin.
- **Supervision**: charge timeout (5 s), discharge-stuck (<5 V drop/s), kick-no-drop
  (<10 V drop in 50 ms), invalid-command; all latched into a FAULT register + LED.
- **Main loop**: `while (1) SLEEP();` — everything happens in ISRs.

---

## 2. Critical findings

### 2.1 `SLEEP()` freezes every timebase — including mid-kick (HIGH)

`main.c:18-21` sleeps whenever no ISR is running. The PIC16F15224 has **no Idle/Doze mode**
(no `CPUDOZE` register on this part) — `SLEEP` is full sleep and stops FOSC. Everything
clocked from FOSC/4 halts:

- **TMR2/PWM freeze mid-kick pulse.** A kick is started from the I2C ISR
  (`mikona.c:131-181`); the ISR returns, main executes `SLEEP()`, and the PWM output pin is
  frozen **high**, energizing the solenoid until the next wake event (next I2C byte, DONE
  edge, or WDT wake after ~1 s). The pulse duration you program is only honored to the
  extent that I2C polling traffic happens to keep the CPU awake. This landed in `2c755a8`
  (Mar 2026); the kick timing work (`2313459`, `58c9738`) predates/ignores it. If you saw
  inconsistent kick strength at competition, this is a prime suspect.
- **TMR0 halts** → `g_time_ms` advances only during cumulative awake time → the 5 s charge
  timeout, 1 s discharge window, and 50 ms kick check all stretch by an unknown,
  traffic-dependent factor.
- **ADC conversions stop** (triggered by TMR0) → `v_out` goes stale → recharge maintenance
  reacts late.

The firmware appears to work because the master polls constantly, and because at 1 MHz the
I2C ISR is so slow relative to the bus (see §3.5) that the CPU is awake a large fraction of
the time during traffic. That's an accident, not a design.

**Fix**: don't sleep. A board that pumps hundreds of volts into capacitors gains nothing
from saving ~500 µA of CPU. If you ever want sleep, it must be conditional (never while a
kick/charge/discharge/supervision window is active) and the tick source must move to
LFINTOSC — but "never sleep" is the right call here.

### 2.2 The watchdog is ineffective as designed (MEDIUM)

`f55e583` enabled WDT (~1 s period) and clears it in the TMR0 ISR (`mikona.c:223-227`). Two
problems:

- On this core, executing `SLEEP` **clears the WDT**. Since main does nothing but sleep,
  the WDT is reset on every wake→sleep cycle regardless of application health.
- Clearing the watchdog *inside an ISR* proves only that the timer ISR runs. In an
  architecture where all work happens in ISRs, that's circular; in the recommended
  main-loop architecture, `CLRWDT()` belongs in the main loop, gated on "all subsystems
  recently made progress".

Also: after a WDT reset (or brown-out), all registers clear — CHARGE silently drops and the
master has no way to distinguish "board rebooted" from "nothing happened". See §5.3.

### 2.3 I2C write overrun corrupts adjacent registers (MEDIUM severity, LOW likelihood)

`protocol.c:53-86`: for KICK_A/KICK_B, `array_idx = counter - 1` indexes `u8[2]` with no
bounds check. A master write with 3+ data bytes walks past the union:
`kick_a.u8[2]` lands on `kick_b`, and `kick_b.u8[2]` lands on **`fault`** (struct order in
`protocol.h:29-36`). A misbehaving or buggy master can silently corrupt register state.
One-line bounds check; the rewrite should bound every register write.

### 2.4 Discharge supervision false-faults on a successful discharge (MEDIUM)

`mikona.c:250-266`: DISCHARGE_STUCK fires when voltage drops <5 V per 1 s window. A
*successful* discharge ends with the caps near 0 V — at which point the next window
necessarily sees <5 V of drop and raises a spurious fault, turning the LED red and
disabling discharge. The check needs a floor ("v_out below ~10 V ⇒ discharge complete,
auto-clear DISCHARGE, no fault") — which would also give the master a useful "discharge
finished" signal it currently lacks.

Related wart: STATUS write handling is level-triggered per byte (`protocol.c:29-52`).
Every STATUS write with DISCHARGE=1 calls `on_discharge_requested(true)` again, resetting
the stuck-detector window (`mikona.c:346-354`). A master that re-writes STATUS more often
than once per second can *never* get a DISCHARGE_STUCK fault. Side effects should fire on
bit *edges*, not levels.

### 2.5 Voltage calibration rests on an unexplained ×1.25 fudge (MEDIUM)

`mikona.c:91-98` computes `v = adc * 404 / 1250`, i.e. the nominal `4.096 × 101 / 1023`
divided by an empirical 1.25 (`d239c8a`, TODO still in code). FVR is genuinely configured
at 4.096 V (`fvr.c: FVRCON = 0x83`), so the discrepancy is on the board: the real divider
ratio is ~81:1, not 101:1 — or the ADC is loading a too-high-impedance divider. This
matters because **every threshold in the firmware (50 V failsafe floor, 10 V recharge
band, 10 V kick drop, 5 V/s discharge) is denominated in these calibrated volts**. Worth an
hour with a multimeter against the schematic; then encode the real ratio as named constants
in one place and delete the fudge.

### 2.6 The README contradicts the code (MEDIUM, cheap to fix)

- **V_OUT formula is wrong.** README says `V = register_value × (4.096 × 101 / 1023)`.
  In reality the register already *is* volts (1 V/LSB, calibrated). A master following the
  README multiplies volts by 0.40 and gets nonsense. The formula describes the pre-`d239c8a`
  raw-ADC scheme.
- `mikona.h:21` comment on `get_v_out` ("0-255 = 0-4.096V") is similarly stale/wrong.
- **"Device ID: 0x54"** reads like the I2C address; the actual bus address is `0x2B`
  (write byte `0x56`), buried in generated code. Document both explicitly.
- **KICK byte order is undocumented** (it's little-endian: LSB first).
- **Kick quantization/limits undocumented**: 16 µs steps (rounded up), max 15 984 µs,
  values above are silently clamped.
- **Auto-clearing of CHARGE/DISCHARGE bits on fault** (timeout, stuck) is observable by the
  master but undocumented.

---

## 3. Architectural assessment

### 3.1 Everything runs in interrupt context

`9773d35` moved actuator control into the I2C ISR callbacks; since then, GPIO writes, the
charge state machine, fault logic, a 32-bit multiply/divide (`get_v_out`), and an insertion
sort (`update_max_voltage`) all execute inside ISRs, while main sleeps. It *happens* to be
race-free because this core never nests interrupts — every line of logic is serialized in
one ISR context. But that invariant is undocumented, load-bearing, and evaporates the
moment anyone adds code to the main loop (16-bit `g_time_ms` reads would tear, RMW on
`status` would race). The design also maximizes ISR latency: at 1 MHz the I2C ISR path
costs more than an I2C byte time, so the slave clock-stretches heavily and every subsystem
adds jitter to every other.

### 3.2 One state machine, three owners

`g_charge_state` is written by the ADC ISR (`adc_interrupt_handler`), the IOC ISR
(`done_ioc_handler`), and I2C callbacks (`on_charge_requested`, `setKickA/B`). The
CS_KICK_INHIBIT resume path lives inside the *kick fault check*; the recharge trigger and
the failsafe floor live in the ADC handler; the peak learning in the IOC handler. Each of
these was added by a separate commit (`79662c6` → `dbeae70` → `df4ab86`) and it shows: to
answer "what happens if DONE fires while a kick is pending?" you have to mentally interleave
three files' worth of handlers. This is the strongest argument for the rewrite: the *rules*
are fine, their *distribution* is the problem.

### 3.3 Module coupling is circular

`protocol.c` (transport) calls application functions (`setKickA`, `on_charge_requested`,
`set_fault`) directly from the I2C ISR; `mikona.c` (application) reaches back into
`g_registers` (owned by protocol.h) to set STATUS/fault bits. Neither module can be
understood or tested alone. There's no HAL boundary either — application code calls MCC
pin macros and `PWM3_LoadDutyValue` directly, including the subtle RC2/RC3 tri-state
mutual-exclusion trick (`mikona.c:138-142`), which deserves a named function and a comment
about the gate-pulldown assumption.

### 3.4 Magic numbers coupled to MCC configuration

`KICK_PWM_PERIOD_COUNTS = 1000`, `KICK_PWM_STEP_US = 16` (`mikona.c:15-18`) are only true
while T2PR=249, TMR2 prescale=1:16, and FOSC=1 MHz — all of which live in generated code /
the `.mc3` file. Change any of them in MCC and kick durations silently break. Same for the
10 ms tick assumption. Derive these from `_XTAL_FREQ` + the register values with macros and
`_Static_assert`, or at least cross-check them at compile time.

### 3.5 The 1 MHz clock choice starves the CPU

250 k instructions/s means the MCC I2C ISR costs several hundred µs per byte — longer than
a 100 kHz I2C byte — so the bus runs at whatever rate clock-stretching allows, and the
32-bit division in the 100 Hz ADC path is a meaningful fraction of the budget. The chip
runs at 32 MHz for free. The only thing tied to 1 MHz is the kick PWM step (16 µs); at
32 MHz with TMR2 prescale 1:128 you get 4 µs steps with ~4 ms max pulse (check your longest
real kick), or you can keep a slower TMR2 clock and raise FOSC anyway — TMR2's clock is
selectable independently. Decide based on your longest kick, but don't keep 1 MHz by inertia.

### 3.6 Generated-code management is fuzzy

`mcc_generated_files/` has been touched by 11 commits. Most changes trace back to `.mc3`
regeneration (good — the interrupt priority reorder in `595913f` is captured in the MCC
config), but nothing marks which files are pristine. Decide a policy: either *never*
hand-edit generated files (regeneration-safe) or vendor them ("this is now our code") and
delete the `.mc3`. Given how thin these drivers are (see `pwm3.c` — 20 real lines), fully
owning ~6 small files is very defensible and removes the MCC version treadmill
(`98cdd3b` was exactly that churn).

### 3.7 Smaller inventory

- Naming is inconsistent: `set_led_color` / `on_charge_requested` vs `setKickA` /
  `setCharge`; `LedColorRed` vs `charge_state_t`/`CS_*`.
- `setKickA`/`setKickB` are copy-paste twins (`mikona.c:131-181`) — parameterize.
- Dead code: `g_internal.read_address` (`protocol.c:12,130`) is written, never read;
  `array_idx` in `i2c_write_callback` unused; `main.c:23-25` unreachable;
  `is_done()` needn't be public.
- `KICK_MIN_START_V` defined mid-file after variables (`mikona.c:49`).
- `= {}` initializers are a GNU/C23-ism; XC8 accepts it, but `{0}` is the portable idiom.
- Multi-byte STATUS writes re-execute charge/discharge side effects once per byte.
- Multi-byte reads repeat the same register value (no auto-increment) — fine, but
  undocumented.

---

## 4. What's good and worth keeping

- **The register-based I2C protocol shape.** Register map, write masks, latched fault
  register with summary bit — this is the right interface. Keep it (with additions, §5.3).
- **The PWM one-shot kick.** Hardware-timed pulse end, immune to ISR latency — genuinely
  the right mechanism on this part. It only needs the sleep fix and a compile-time link to
  the clock config.
- **The charge-maintenance concept.** Median-of-5 peak learning + sag-retrigger +
  <50 V failsafe relearn is a sensible answer to the LT3750's one-shot behavior, and the
  wrapping `(uint16_t)(now - start)` timeout idiom is correct everywhere.
- **The supervision faults.** Charge-timeout / discharge-stuck / kick-no-drop with a
  low-voltage guard (`9f76a26`) is a good safety net; only the discharge floor (§2.4) is
  missing.
- **Repo hygiene**: build artifacts properly ignored, `.mc3` tracked, README exists and is
  mostly good — it just drifted.
- **Headroom**: 44% flash and 79% RAM free — no constraint on restructuring.

---

## 5. Recommended rewrite

Rewrite `mikona.c` + `protocol.c` (~500 lines) around a superloop; keep MCC drivers (or
vendor them, §3.6) and the external protocol. Suggested shape:

```
src/
  main.c        superloop: drain event queue, run state machines, CLRWDT
  board.{c,h}   pin/PWM/ADC access, voltage calibration in ONE place,
                kick-channel mux (the TRIS trick, named and commented)
  charger.{c,h} LT3750 state machine — one transition function:
                charger_step(event, now_ms)
  kicker.{c,h}  pulse start + no-drop supervision + charge-inhibit handshake
  faults.{c,h}  fault latch + LED
  regs.{c,h}    I2C register bank: ISR only latches reads/writes,
                completed writes become events for the main loop
```

Design rules that fix the failure classes found above:

1. **ISRs record, main loop acts.** I2C read callbacks may serve current register values
   (reads are snapshots), but *side effects* of writes (kick, charge, discharge, fault
   clear) become queued events handled in main. This kills the ISR-latency problem, makes
   atomicity explicit, and makes the state machines testable.
2. **No `SLEEP()`.** Main loop spins on the event queue; `CLRWDT()` lives there.
3. **One transition function per state machine**, with events named
   (`EV_TICK_10MS, EV_DONE_FALL, EV_CMD_CHARGE_ON, EV_KICK_FIRED, ...`). The current rules
   transplant 1:1; the difference is you can read them top to bottom.
4. **Critical sections, not luck**: tiny `ATOMIC()` helper (save GIE, disable, restore) for
   the few multi-byte shared reads (`g_time_ms`, 16-bit kick latch).
5. **Compile-time config coherence**: derive tick period, PWM step, and max pulse from
   `_XTAL_FREQ` + timer settings; `_Static_assert` the assumptions.
6. **Bounds-check every register write**; ignore excess bytes.
7. Pick one naming convention (`snake_case` to match the majority) and apply it.

### 5.3 Protocol v2 additions (you own both sides — all cheap)

- **Reset-cause visibility**: a STATUS or FAULT bit set on power-up/WDT/BOR reset, cleared
  like other faults. Without it the master can't tell the board rebooted and silently
  stopped charging (§2.2).
- **VERSION register** (firmware version / git short-hash byte) next to DEV_ID.
- **Expose the learned peak voltage** as a read-only register — free debuggability.
- Consider one **KICK command register** (`[channel][duration_lo][duration_hi]`) instead of
  two registers — halves the duplicated code path.
- **Discharge-complete semantics** from §2.4: auto-clear DISCHARGE + DONE-style flag
  instead of a spurious fault.
- Rewrite the README register doc from the new code (fix §2.6 items: real I2C address,
  byte order, 16 µs quantization, clamping, auto-clear behaviors).

### 5.4 Suggested order of work

1. Measure the real divider ratio; replace the ×1.25 fudge with named constants (§2.5).
2. Decide clock: stay at 1 MHz or move to 32 MHz (§3.5) — this pins the kick-timer math.
3. Vendor or regenerate MCC drivers once, freeze the policy (§3.6).
4. Rewrite per §5.1/5.2; bring rules over one state machine at a time
   (charger → kicker → faults → protocol shim).
5. Bench-verify with a scope: kick pulse width vs commanded (idle bus *and* busy bus —
   this specifically validates the §2.1 fix), charge/recharge cycle, each fault injected
   (disconnect cap sense, hold DONE, kick with no solenoid), WDT recovery, README v2
   walk-through from a clean master implementation.

## 6. Quick wins if you patch instead of rewriting

In descending value-per-line:

1. Delete `SLEEP()` (make the loop `while(1) CLRWDT();` and remove the ISR `CLRWDT`) — §2.1/2.2.
2. Bounds-check `array_idx < 2` in KICK writes — §2.3.
3. Add the discharge-complete floor — §2.4.
4. Fix the README V_OUT formula, address, byte order — §2.6.
5. Edge-trigger STATUS side effects — §2.4.
6. Fix the stale `get_v_out` comment in `mikona.h`.

---

## Appendix: commit-history observations

The history itself tells the "patched until it worked" story and largely maps to the
findings above:

- `2c755a8` introduced sleep after `9773d35` moved all work into ISRs — the two together
  created §2.1; the kick-accuracy work predates them.
- `d3a8354` ("Fix g_time_ms") and `f55e583` (WDT) are patches on the timing system that
  §2.1 undermines in sleep.
- `5923366` → `a0ea1b2` → `afff90e` → `9f76a26` is four rounds of patching the fault
  feature in one day (Apr 9), including fixing STATUS.FAULT being clobbered by the write
  mask — symptomatic of side-effectful register writes with no layering (§3.3).
- `79662c6` → `dbeae70` → `df4ab86` is the charge state machine accreting across three
  commits/contexts (§3.2).
- `d239c8a` is the ×1.25 fudge (§2.5).
- Cleanliness signals worth keeping up: `418ca64` removed committed logs, `1d5c678` did a
  proper toolchain migration, and commit messages are consistently good.
