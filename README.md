# hardware_trng_74ls74
Hardware True Random Number Generator (TRNG) using BJT avalanche noise, a 74LS74 flip-flop and an Arduino Uno with Von Nuemann Debiasing to produce uniform 32-bit hexadecimal random outputs.

## 1. Project Overview & Motivation

* **Limitations of Standard PRNGs:** Software pseudo-random functions (like `random()` in C/C++) are deterministic mathematical algorithms. If internal seeds are exposed, output sequences become completely predictable.
* **Physical Semiconductor Noise:** Physical TRNGs harvest unpredictable microscopic charge fluctuations (avalanche and thermal shot noise) occurring across semiconductor junctions.
* **Avalanche Breakdown Entropy:** Operating the base-emitter junction of an NPN transistor (Q1) in reverse breakdown produces wideband microvolt-level current fluctuations that are physically non-deterministic.
* **Amplification Stage:** The raw noise amplitude is extremely small (microvolts); a second transistor (Q2) configured as a common-emitter amplifier magnifies this signal into measurable voltage swings.
* **Metastability Mitigation via 74LS74:** Microcontroller sampling can experience latching errors or metastability if transitions align with clock sample edges. An external positive-edge-triggered 74LS74 D-type flip-flop establishes clean digital transition boundaries.
* **Unified 5V Logic Design:** Built on a single +5V power rail from the Arduino Uno, ensuring full compatibility with 74LS TTL logic thresholds without needing level shifters or voltage dividers.
* **Von Neumann De-biasing:** Hardware components naturally have DC offsets and temperature-dependent biases. Comparing consecutive bit pairs (01 -> 0, 10 -> 1, rejecting 00 and 11) mathematically guarantees a balanced 50/50 bit distribution without complex formulas.

---

## 2. Hardware Architecture & Pin Interfacings

* **System Power Bus (+5V):** Arduino Uno `5V` pin powers the analog noise stages and the 74LS74 IC.
* **System Ground (GND):** Arduino Uno `GND` pin provides a unified reference ground across the breadboard.
* **Noise Generation (Q1 Emitter with 10 kΩ Resistor):** Connected to +5V through a **10 kΩ resistor** to drive the base-emitter junction into safe, continuous reverse breakdown.
* **Noise Low-Side Return (Q1 Base & Collector):** Connected directly to the common ground rail.
* **AC Coupling Capacitor (1 µF):** Placed in series between Q1 noise output and Q2 base to block quiescent DC voltage while passing high-frequency AC noise spikes.
* **Amplifier Base Bias (Q2 Base with 10 kΩ Resistor):** Connected between +5V and Q2 base via a **10 kΩ resistor** to keep the amplifier in its active amplification region.
* **Amplifier Collector Load (Q2 Collector with 1 kΩ Resistor):** Connected between +5V and Q2 collector via a **1 kΩ resistor** to produce substantial voltage gain.
* **Noise Readout (Q2 Collector to Arduino `A0`):** Connected directly from the collector junction of Q2 to Arduino analog pin `A0`.
* **74LS74 Power (Pin 14 - VCC):** Tied to +5V rail.
* **74LS74 Ground (Pin 7 - GND):** Tied to the ground rail.
* **74LS74 Control Tie-offs (Pins 1, 4, 10, 13):** Active-LOW clear (`/CLR`) and preset (`/PR`) pins are tied high to +5V to prevent unintended reset states.
* **74LS74 Data In (Pin 2 - 1D):** Connected to Arduino digital pin `2`.
* **74LS74 Clock In (Pin 3 - 1CLK):** Connected to Arduino digital pin `3`.
* **74LS74 Data Out (Pin 5 - 1Q):** Connected to Arduino digital pin `4` for reading latched output bits.

---

## 3. Components Used

* 1x Arduino Uno (ATmega328P)
* 1x 74LS74 Flip-Flop IC
* 2x BC547B NPN Transistors
* 2x 10 kΩ Resistors
* 1x 1 kΩ Resistor 
* 1x 1 µF Capacitor
* Breadboard and jumper wires

---

## 4. Extraction & De-biasing Workflow

* **Step 1 (Physical Sampling):** The Arduino samples analog noise on pin `A0` and extracts the least significant bit (LSB).
* **Step 2 (Hardware Latch):** The LSB is applied to flip-flop pin `1D`, clocked via pin `1CLK`, and verified on pin `1Q`.
* **Step 3 (Von Neumann Whitening):**
  * Two consecutive latched bits ($b_1, b_2$) are evaluated.
  * `01` transition produces a valid bit `0`.
  * `10` transition produces a valid bit `1`.
  * Matching pairs `00` and `11` are rejected.
* **Step 4 (32-Bit Aggregation):** 32 conditioned bits are shifted into an unsigned 32-bit integer (`uint32_t`) to form a complete hexadecimal word.

---

## 5. Sample Output

```text
==============================================
       Hardware TRNG (32-Bit Hex Output)      
==============================================

Random Hex Seed: 0x4C956851  |  Dec: 1284859985
Random Hex Seed: 0xA1EC9A2D  |  Dec: 2716637741
Random Hex Seed: 0x2C4C416C  |  Dec: 743194988
Random Hex Seed: 0xE7425B66  |  Dec: 3879885670
Random Hex Seed: 0x98FB123C  |  Dec: 2566582844
