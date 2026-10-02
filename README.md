# hardware_trng_74ls74
Hardware True Random Number Generator (TRNG) using BJT avalanche noise, a 74LS74 flip-flop and an Arduino Uno with Von Nuemann Debiasing

# Physical Hardware True Random Number Generator (TRNG)

A discrete hardware-based True Random Number Generator leveraging reverse-biased BJT avalanche breakdown noise, synchronous discrete TTL latching via a 74LS74 dual D-type flip-flop, and real-time Von Neumann de-biasing on an ATmega328P microcontroller (Arduino Uno).

---

## 1. Project Motivation & Theory

* **Vulnerability of Standard PRNGs:** Standard software pseudo-random number generators (such as C/C++ `random()` or `rand()`) are deterministic algorithms where future bit sequences become predictable if internal seed states are known.
* **True Physical Entropy:** Physical TRNGs sample non-deterministic semiconductor phenomena specifically quantum barrier tunneling, thermal agitation, and avalanche impact ionization that cannot be mathematically modeled or predicted.
* **Avalanche Breakdown Mechanism:** Operating an NPN BJT base-emitter junction in reverse breakdown initiates impact ionization, generating wideband, microscopic electrical current spikes (shot noise).
* **Signal Conditioning Need:** Avalanche noise fluctuates at microvolt levels; a dedicated single-stage common-emitter amplifier with high collector resistance is required to translate these microscopic fluctuations into measurable analog voltage swings.
* **Metastability Mitigation via 74LS74:** Sampling raw analog noise directly on a microcontroller GPIO or ADC risks digital latching failures or metastability if input transitions coincide with CPU sampling windows; an external edge-triggered 74LS74 flip-flop enforces clean, deterministic latching states.
* **Unified 5V Logic Design:** Migrating from mixed-voltage platforms (3.3V microcontrollers) to a native 5V Arduino Uno architecture removes voltage divider attenuation, logic-threshold mismatches, and power-rail cross-talk, keeping TTL logic thresholds compliant.
* **Mathematical Whitening:** Hardware components inherently possess temperature drift, bias, and DC offsets; Von Neumann de-biasing strips systemic skew algorithmically to produce a balanced, uniform distribution ($P(0) = P(1) = 0.5$).

---

## 2. System Architecture

* **Noise Generation Stage:** First NPN BJT (Q1) configured as an avalanche noise diode via a reverse-biased base-emitter junction.
* **AC Coupling:** A $1\,\mu\text{F}$ capacitor isolates the DC operating point while passing high-frequency noise transients to the amplifier stage.
* **High-Gain Amplifier Stage:** Second NPN BJT (Q2) arranged in common-emitter configuration to swing noise voltages across the active linear region.
* **Discrete Latching Stage:** 74LS74 dual D-type positive-edge-triggered flip-flop synchronizing raw entropy to digital levels.
* **Microcontroller Processing Stage:** Arduino Uno executing hardware clock pulsing, bit sampling, Von Neumann whitening, and Shannon entropy batch calculation.

---

## 3. Hardware Interfacings & Pin Connections

* **System Power Bus (+5V):** Arduino Uno `5V` pin feeds the breadboard positive distribution rail to power the analog stages and the 74LS74 TTL IC.
* **System Ground (GND):** Arduino Uno `GND` pin ties directly to the breadboard negative rail, establishing a shared reference voltage across all analog and digital grounds.
* **Noise Source High-Side Bias (Q1 Emitter with 10kΩ Resistor):** Connected to the $+5\text{V}$ rail via a **10 kΩ current-limiting resistor** to safely trigger continuous reverse avalanche breakdown without burning out the base-emitter junction.
* **Noise Source Low-Side Return (Q1 Base/Collector):** Tied directly to the breadboard GND rail to complete the avalanche loop and ground unused transistor terminals.
* **DC Blocking & AC Coupling (1µF Capacitor):** A **1 µF electrolytic/ceramic capacitor** is placed in series between the Q1 noise node and Q2 base to block the quiescent DC voltage while feeding pure AC microvolt noise transients into the amplifier.
* **Amplifier Base Bias (Q2 Base with 10kΩ Resistor):** Connected from the $+5\text{V}$ rail to the base of Q2 via a **10 kΩ resistor** to lightly bias the transistor into its active linear amplification region.
* **Amplifier Collector Load (Q2 Collector with 1kΩ Resistor):** Connected between the collector of Q2 and the $+5\text{V}$ rail via a **1 kΩ pull-up load resistor** to produce a high voltage gain swing across the analog linear-to-saturation boundary.
* **Noise Sampling Node (Q2 Collector to Arduino `A0`):** Connected directly from the collector junction of Q2 to Arduino analog pin `A0` to read amplified noise signals into the ADC.
* **74LS74 Power Supply (Pin 14 - $V_{CC}$):** Connected directly to the $+5\text{V}$ rail, fulfilling strict TTL logic power requirements.
* **74LS74 Ground Reference (Pin 7 - $GND$):** Tied to the common ground rail for proper return path and switching reference.
* **74LS74 Override Tie-Offs (Pins 1, 4, 10, 13):** Active-LOW clear ($\overline{\text{CLR}}$) and preset ($\overline{\text{PR}}$) terminals for both flip-flops are tied directly to $+5\text{V}$ to prevent unintentional hardware resets.
* **74LS74 Data Input (Pin 2 - $1D$):** Wired to Arduino digital pin `2` to supply data levels derived from entropy samples.
* **74LS74 Clock Trigger (Pin 3 - $1CLK$):** Driven by Arduino digital pin `3` to deliver precision positive-edge latching pulses.
* **74LS74 Output Readout (Pin 5 - $1Q$):** Routed to Arduino digital pin `4` to read the latched digital bit back into the microcontroller firmware.
* **Unused IC Terminals (Pins 6, 8, 9, 11, 12):** Left unconnected or tied off safely to avoid output short-circuits or excess thermal load.

---

## 4. Firmware Implementation & Entropy Extraction

* **Active Edge Latching:** The firmware sets data on pin `2`, applies a propagation delay, pulses clock pin `3` HIGH, samples the latched state on pin `4`, and resets the clock to LOW.
* **Von Neumann Conditioning Logic:** 
  * Extracts two consecutive non-overlapping raw bits ($b_1, b_2$) from the physical stage.
  * Pattern `01` yields an unbiased bit `0`.
  * Pattern `10` yields an unbiased bit `1`.
  * Patterns `00` and `11` are rejected to eliminate inherent analog circuit bias.
* **Shannon Entropy Verification:** 
  * Evaluates bit batches ($N = 500$) using the formula:
    $$H(X) = - \sum_{i \in \{0, 1\}} P(x_i) \log_2 P(x_i)$$
  * Theoretical maximum for an ideal binary source is $H(X) = 1.0000\text{ bits/symbol}$.
* **Cryptographic Seed Aggregation:** Packages 32 consecutive debiased bits into 32-bit hexadecimal words (`uint32_t`) suitable for cryptographic seeding.
* **Source Code:** Full implementation is available in the firmware source file in this repository.

---

## 5. Empirical Validation & Output Metrics

* **Shannon Entropy Score:** Consistently measures between `0.9994` and `1.0000` (ideal uniform binary entropy is `1.0000`).
* **Bit Uniformity:** Generates approximately a 50.0% / 50.0% distribution of logic 1s and 0s per batch.
* **Non-Deterministic Seeding:** Produces unique 32-bit hexadecimal random outputs with each batch run.

```text
[Entropy Batch] 1s: 256 (51.2%) | 0s: 244 | Shannon Entropy: 0.9996 / 1.0000
  -> 32-bit Random Hex Seed: 0x4C956851
----------------------------------------------
[Entropy Batch] 1s: 250 (50.0%) | 0s: 250 | Shannon Entropy: 1.0000 / 1.0000
  -> 32-bit Random Hex Seed: 0xA1EC9A2D
----------------------------------------------
[Entropy Batch] 1s: 254 (50.8%) | 0s: 246 | Shannon Entropy: 0.9998 / 1.0000
  -> 32-bit Random Hex Seed: 0x2C4C416C
----------------------------------------------
[Entropy Batch] 1s: 251 (50.2%) | 0s: 249 | Shannon Entropy: 1.0000 / 1.0000
  -> 32-bit Random Hex Seed: 0xE7425B66
