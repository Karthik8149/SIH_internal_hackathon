# OJAS

## Optimization-guided Joule-aware Adaptive Sonar

**A Physics-Guided, Minimum-Energy Software-Defined Real-Time Sonar Transmitter Payload for Autonomous Underwater Vehicles (AUVs).**

OJAS is a software-defined sonar transmitter architecture that dynamically selects the operating frequency of an LFM (Linear Frequency Modulated) acoustic pulse according to the surrounding underwater environment.

Instead of transmitting every pulse at a fixed frequency, OJAS uses **temperature, salinity, depth and operating conditions** to estimate the local acoustic environment and automatically select a suitable frequency that satisfies the required detection/communication condition while avoiding unnecessary acoustic energy expenditure.

> **Core idea:**
> **Sense → Model → Predict → Correlate → Select → Transmit**

---

## Why OJAS?

Conventional sonar systems commonly operate using predefined frequency configurations selected according to the intended operating range, resolution and transducer characteristics.

A fixed-frequency approach is simple and reliable, but the underwater acoustic environment is not fixed.

Temperature, salinity and pressure/depth affect the speed of sound, while frequency-dependent absorption and propagation loss influence how efficiently acoustic energy reaches the receiver.

Consequently, a frequency that is suitable in one environment may not be the most energy-efficient operating point in another.

OJAS addresses this by introducing **real-time, physics-guided frequency adaptation** into the transmitter itself.

---

# Traditional Approach

A conventional fixed-frequency LFM sonar can be represented as:

```text
Environment
     │
     ▼
Fixed Sonar Configuration
     │
     ▼
Fixed Centre Frequency
     │
     ▼
LFM Pulse Generation
     │
     ▼
Acoustic Transmission
```

For example, the transmitter may continuously operate around a predetermined centre frequency such as:

```text
fc = 200 kHz
```

regardless of changes in the surrounding medium.

This provides predictable operation, but the transmitter does not explicitly use environmental information to modify its operating frequency.

OJAS instead introduces an adaptive decision layer:

```text
Temperature ─┐
Salinity ────┤
Depth ───────┤
              ▼
      Physics-Based Model
              │
              ▼
   Acoustic Environment Estimate
              │
              ▼
      Frequency Candidates
              │
              ▼
       Correlation Check
              │
              ▼
      Adaptive Selection
              │
              ▼
          LFM Pulse
```

---

# The Physics Behind OJAS

## Mackenzie Sound-Speed Equation

The first stage of the OJAS environmental model estimates the speed of sound in seawater.

The **Mackenzie nine-term equation**, published by Kenneth V. Mackenzie in 1981, relates sound speed to:

* Temperature
* Salinity
* Depth

The equation is:

$$
c(T,S,D)=1448.96+4.591T-5.304\times10^{-2}T^2
+2.374\times10^{-4}T^3
+1.340(S-35)
$$

$$
+1.630\times10^{-2}D
+1.675\times10^{-7}D^2
-1.025\times10^{-2}T(S-35)
-7.139\times10^{-13}TD^3
$$

where:

* \(c\) = speed of sound [m/s]
* \(T\) = temperature [°C]
* \(S\) = salinity [ppt/PSU]
* \(D\) = depth [m]

The equation was developed from oceanographic measurements and has historically been widely used because it provides a compact computational representation of the relationship between environmental variables and underwater sound speed.

Modern scientific and engineering software continues to implement Mackenzie's formulation. MathWorks, for example, explicitly cites Mackenzie (1981) as one of the reference equations used for underwater sound-speed calculation.

Published literature describes the Mackenzie formulation as a nine-term empirical equation applicable over typical oceanic ranges, with reported accuracy on the order of approximately 0.1 m/s depending on the stated environmental range and reference used.

### Why use Mackenzie?

OJAS does not use Mackenzie because it is the most accurate equation available.

It is used because it provides an excellent **embedded-systems trade-off between physical relevance, computational simplicity and low latency**.

A more computationally demanding model could potentially provide greater accuracy, but OJAS is specifically designed for a resource-constrained real-time transmitter.

Therefore:

> **Mackenzie acts as the low-complexity physics layer of OJAS, not as the final frequency-selection algorithm.**

---

# Frequency-Dependent Acoustic Loss

Sound speed alone is not sufficient for selecting the operating frequency.

OJAS also considers frequency-dependent acoustic absorption using the **Francois-Garrison absorption model**.

The general concept is:

$$
TL(f,R)
=
TL_{\text{spreading}}
+
TL_{\text{absorption}}(f,R)
$$

where:

* \(TL\) = transmission/path loss
* \(f\) = acoustic frequency
* \(R\) = propagation range

The important characteristic is that underwater absorption depends strongly on frequency.

Therefore, different candidate frequencies can experience different propagation losses under the same environmental conditions.

This gives OJAS a physics-based quantity with which candidate frequencies can be compared.

---

# OJAS Architecture

The complete concept is:

```text
 ┌──────────────────────────────┐
 │       AUV Environment        │
 │                              │
 │ Temperature                  │
 │ Salinity                     │
 │ Depth                        │
 │ Range / Operating Condition  │
 └──────────────┬───────────────┘
                │
                ▼
 ┌──────────────────────────────┐
 │     Environmental Sensors    │
 └──────────────┬───────────────┘
                │
                ▼
 ┌──────────────────────────────┐
 │   Mackenzie Sound-Speed      │
 │          Model               │
 └──────────────┬───────────────┘
                │
                ▼
 ┌──────────────────────────────┐
 │ Frequency-Dependent Acoustic │
 │      Loss Estimation         │
 │                              │
 │ Francois-Garrison +          │
 │ propagation model            │
 └──────────────┬───────────────┘
                │
                ▼
 ┌──────────────────────────────┐
 │ Candidate Frequency Library  │
 │                              │
 │ 50 | 75 | 100 | 125 |       │
 │ 150 | 175 | 200 kHz          │
 └──────────────┬───────────────┘
                │
                ▼
 ┌──────────────────────────────┐
 │      Correlation Layer       │
 │                              │
 │ Received response / expected │
 │ LFM response consistency     │
 └──────────────┬───────────────┘
                │
                ▼
 ┌──────────────────────────────┐
 │   Adaptive Decision Engine   │
 │                              │
 │ Select lowest-energy         │
 │ feasible operating point     │
 └──────────────┬───────────────┘
                │
                ▼
 ┌──────────────────────────────┐
 │       LFM Generator          │
 │                              │
 │ Adaptive Centre Frequency    │
 │ Constant Bandwidth           │
 │ Pulse Duration               │
 └──────────────┬───────────────┘
                │
                ▼
 ┌──────────────────────────────┐
 │      Sonar Transmitter       │
 └──────────────────────────────┘
```

---

# What Makes OJAS Different?

The fundamental difference is not simply "using LFM."

Both the conventional and OJAS systems can use the same LFM waveform structure.

The difference is **how the LFM centre frequency is selected**.

### Traditional system

```text
Centre frequency
       ↓
    FIXED
       ↓
   LFM pulse
```

### OJAS

```text
Environmental measurements
          ↓
     Physics model
          ↓
Propagation characteristics
          ↓
Candidate frequencies
          ↓
Correlation / feasibility
          ↓
Lowest suitable frequency
          ↓
Adaptive LFM pulse
```

The bandwidth and pulse duration can remain constant while the **centre frequency changes according to the operating environment**.

This allows the waveform structure to remain simple while making the transmitter decision adaptive.

---

# Physics-Guided Adaptation

OJAS is intentionally **physics-guided rather than purely data-driven**.

No large neural network is required to make the primary frequency-selection decision.

The decision engine is based on measurable physical relationships:

```text
T, S, D
  │
  ▼
Sound Speed
  │
  ▼
Acoustic Propagation
  │
  ▼
Frequency-dependent Loss
  │
  ▼
Candidate Evaluation
  │
  ▼
Frequency Selection
```

This provides several advantages for an embedded AUV payload:

* Deterministic behaviour
* Low computational complexity
* Low memory requirement
* Low latency
* Explainable decisions
* No requirement for a large training dataset
* Easier hardware verification
* Easier debugging
* Easier deployment on microcontrollers

The physics model therefore provides **interpretability and a direct connection between the measured environment and transmitter behaviour**.

---

# Correlation Layer

A key part of OJAS is the **correlation-based validation mechanism**.

The transmitter does not rely only on the environmental model.

After transmission and reception, the received acoustic signal can be compared with the expected LFM reference using correlation.

Conceptually:

$$
\rho =
\frac{
\sum (x-\bar{x})(y-\bar{y})
}{
\sqrt{
\sum(x-\bar{x})^2
\sum(y-\bar{y})^2
}}
$$

where:

* \(x\) = reference/transmitted LFM signal
* \(y\) = received signal
* \(\rho\) = correlation coefficient

A strong correlation indicates that the received signal retains the expected LFM structure.

This provides a **measurement-guided feedback mechanism** that complements the physics model.

Therefore, OJAS is not simply:

> "calculate an equation and change frequency."

It is intended to evolve into:

> **Physics prediction + acoustic measurement + correlation-based validation → adaptive transmission decision.**

This is particularly useful because underwater propagation is affected by real-world effects that simplified analytical models cannot completely capture.

---

# Minimum-Energy Frequency Selection

OJAS evaluates a set of available frequencies rather than continuously searching an unlimited frequency space.

For each candidate frequency:

1. Estimate acoustic loss.
2. Estimate expected received SNR.
3. Check whether the candidate satisfies the required operating condition.
4. Reject unsuitable candidates.
5. Select the lowest suitable operating frequency.

Conceptually:

```text
Candidate frequency
       │
       ▼
Estimate propagation loss
       │
       ▼
Estimate received SNR
       │
       ▼
SNR requirement satisfied?
      / \
    NO   YES
    │      │
 Reject    ▼
        Candidate
           │
           ▼
    Compare feasible
      candidates
           │
           ▼
   Minimum-energy point
```

This makes the optimization objective explicit:

> **Do not transmit more acoustic energy than the environment requires.**

---

# Why Lower Frequency?

Underwater acoustic absorption is generally frequency dependent, and higher frequencies can experience greater absorption over propagation distance.

Therefore, when the operating conditions permit it, moving to a lower suitable frequency can reduce propagation loss.

However, OJAS does **not** simply select the lowest possible frequency.

The lowest frequency may not always satisfy:

* Required SNR
* Bandwidth constraints
* Transducer constraints
* Receiver sensitivity
* Range requirements
* System-specific detection requirements

Therefore the decision is:

$$
\boxed{
\text{Lowest feasible frequency}
}
$$

rather than:

$$
\text{Lowest frequency available}
$$

This distinction is central to the OJAS architecture.

---

# Low-Power Design

OJAS is designed around the idea of **minimum required transmit energy**, rather than maximum available transmit power.

If two frequencies provide sufficient received performance, the system can prefer the operating point requiring less estimated transmit energy.

This can potentially reduce:

* Acoustic transmission energy
* Unnecessary transmitter activity
* AUV energy consumption
* Thermal load in the transmitter electronics
* Energy allocated to repeated sonar operations

The current simulation reports **estimated transmit energy required to achieve a target received SNR**.

It should not be interpreted as measured electrical power consumption.

Actual hardware-level power savings will additionally depend on:

* Transducer efficiency
* Power-amplifier efficiency
* Matching network
* Driver losses
* Frequency response of the acoustic transducer
* Receiver characteristics

These parameters will be characterized during hardware validation.

---

# Low Latency and Embedded Feasibility

One of the main design goals of OJAS is to avoid computationally expensive real-time optimization.

The embedded decision chain is intentionally lightweight:

```text
Sensor data
    ↓
Small physics calculation
    ↓
Lookup / candidate evaluation
    ↓
Correlation
    ↓
Frequency selection
    ↓
LFM generation
```

The current implementation targets approximately **1.5 KB of memory for the adaptive decision logic**, making the approach suitable for deployment on resource-constrained embedded hardware.

The architecture is therefore designed around:

| Property                  | OJAS Design                                        |
| ------------------------- | -------------------------------------------------- |
| Decision method           | Physics-guided                                     |
| Environmental inputs      | Temperature, salinity, depth, operating conditions |
| Waveform                  | LFM                                                |
| Centre frequency          | Adaptive                                           |
| Bandwidth                 | Constant                                           |
| Frequency selection       | Candidate-based                                    |
| Feedback                  | Correlation-based                                  |
| Memory footprint          | ~1.5 KB for decision logic                         |
| Latency                   | Low                                                |
| Large ML model required   | No                                                 |
| Training dataset required | No                                                 |
| Embedded deployment       | Designed for it                                    |

---

# Practical Feasibility

OJAS is intended as a **software-defined transmitter architecture**, meaning that the adaptive decision can be implemented without fundamentally changing the sonar concept.

A practical implementation can use:

```text
Temperature Sensor
        │
Salinity Sensor
        │
Depth Sensor
        │
        ▼
Embedded Controller
        │
        ├── Mackenzie Model
        ├── Acoustic Loss Model
        ├── Frequency LUT
        ├── Correlation
        └── Decision Engine
                │
                ▼
        LFM Waveform Generator
                │
                ▼
        DAC / Timer / DMA
                │
                ▼
        Power Amplifier
                │
                ▼
        Acoustic Transducer
```

The computational requirements are considerably smaller than those of a full data-driven optimization or deep-learning architecture.

This makes OJAS particularly suitable for AUV applications where:

* Energy is limited.
* Computing resources are constrained.
* Real-time operation is required.
* Deterministic behaviour is desirable.
* Hardware complexity must remain low.

---

# Simulation and Validation

The current OJAS simulation compares:

### Traditional system

**Fixed-frequency LFM**

* Centre frequency: 200 kHz
* Constant bandwidth
* Fixed pulse duration

### OJAS

**Adaptive LFM**

* Environment-dependent centre frequency
* Same bandwidth
* Same pulse duration
* Physics-guided frequency selection
* Correlation-based validation

The simulation evaluates parameters including:

* Centre frequency
* Predicted channel loss
* Received signal level
* Received SNR
* Received pulse energy
* Estimated transmit energy required for target SNR
* Pulse correlation

The comparison is performed under multiple simulated underwater environmental conditions.

---

# Current Simulation Architecture

```text
                 ┌───────────────────────┐
                 │ Environmental Scenario│
                 │ T / S / Depth / Range │
                 └───────────┬───────────┘
                             │
                             ▼
                 ┌───────────────────────┐
                 │ Mackenzie Sound Speed │
                 └───────────┬───────────┘
                             │
                             ▼
                 ┌───────────────────────┐
                 │ Francois-Garrison     │
                 │ Absorption Model      │
                 └───────────┬───────────┘
                             │
                             ▼
                 ┌───────────────────────┐
                 │ Candidate Frequency   │
                 │ Evaluation            │
                 └───────────┬───────────┘
                             │
                             ▼
                 ┌───────────────────────┐
                 │ Adaptive Frequency    │
                 │ Selection             │
                 └───────────┬───────────┘
                             │
                             ▼
                 ┌───────────────────────┐
                 │ LFM Waveform Generator│
                 └───────────┬───────────┘
                             │
                             ▼
                 ┌───────────────────────┐
                 │ Acoustic Channel      │
                 │ + Spreading           │
                 │ + Absorption          │
                 │ + Multipath           │
                 │ + Doppler             │
                 │ + Noise               │
                 └───────────┬───────────┘
                             │
                             ▼
                 ┌───────────────────────┐
                 │ Received Signal       │
                 └───────────┬───────────┘
                             │
                             ▼
                 ┌───────────────────────┐
                 │ Correlation / Metrics │
                 └───────────────────────┘
```

---

# Design Philosophy

OJAS follows three principles:

### 1. Physics before complexity

Use known physical relationships wherever they can provide a useful decision.

### 2. Adapt only when necessary

Do not continuously optimize every waveform parameter.

Instead, select from a controlled set of feasible operating points.

### 3. Measure and validate

The physics model provides a prediction, while correlation and received-signal measurements provide real-world feedback.

This creates a hybrid architecture:

$$
\boxed{
\text{Physics-Guided}
+
\text{Measurement-Guided}
=
\text{Adaptive Sonar}
}
$$

---

# Advantages

| Feature                             | Traditional Fixed-Frequency LFM | OJAS |
| ----------------------------------- | ------------------------------: | ---: |
| LFM waveform                        |                               ✓ |    ✓ |
| Fixed centre frequency              |                               ✓ |    — |
| Environment-aware                   |                         Limited |    ✓ |
| Physics-based prediction            |                               — |    ✓ |
| Adaptive centre frequency           |                               — |    ✓ |
| Frequency-dependent loss considered |                         Limited |    ✓ |
| Correlation feedback                |                        Optional |    ✓ |
| Minimum-energy selection            |                               — |    ✓ |
| Low-memory architecture             |                               ✓ |    ✓ |
| Large ML model required             |                              No |   No |
| Real-time embedded deployment       |                               ✓ |    ✓ |
| Explainable decision                |                               ✓ |    ✓ |
| Software-defined adaptation         |                         Limited |    ✓ |

---

# Limitations and Future Work

OJAS is currently a **physics-guided prototype and simulation framework**, not a complete production sonar.

Future work includes:

* Hardware implementation on MCU/FPGA
* Real transducer characterization
* Power-amplifier efficiency characterization
* Real hydrophone feedback
* Experimental channel measurements
* Real-time correlation implementation
* Adaptive threshold calibration
* Environmental sensor integration
* More accurate propagation modelling
* Bellhop/ray-tracing-based validation
* Shallow-water multipath validation
* Hardware-in-the-loop testing
* Sea trials with an AUV platform

The analytical channel model is intentionally simplified. Real underwater propagation can involve complex multipath, boundary interactions, spatially varying sound-speed profiles and other effects that cannot be completely represented by a simple transmission-loss model.

Therefore, the current results should be interpreted as **simulation-based feasibility evidence**, followed by experimental validation.

---

# Project Status

| Component                          | Status         |
| ---------------------------------- | -------------- |
| LFM waveform generation            | ✅ Implemented  |
| Fixed-frequency baseline           | ✅ Implemented  |
| Mackenzie sound-speed model        | ✅ Implemented  |
| Frequency-dependent absorption     | ✅ Implemented  |
| Candidate-frequency search         | ✅ Implemented  |
| Adaptive frequency selection       | ✅ Implemented  |
| Analytical underwater channel      | ✅ Implemented  |
| Multipath simulation               | ✅ Implemented  |
| Doppler simulation                 | ✅ Implemented  |
| Noise modelling                    | ✅ Implemented  |
| Correlation metric                 | ✅ Implemented  |
| Traditional vs adaptive comparison | ✅ Implemented  |
| MATLAB visualization               | ✅ Implemented  |
| Embedded implementation            | 🔄 Next stage  |
| Hardware validation                | 🔄 Future work |
| AUV/sea-trial validation           | 🔄 Future work |

---

# Repository Structure

```text
OJAS/
│
├── MATLAB/
│   ├── OJAS_final.m
│   ├── SIH26058_Final_Adaptive_LFM_Comparison.m
│   └── ...
│
├── Simulation/
│   ├── Channel/
│   ├── Waveform/
│   └── Results/
│
├── Hardware/
│   └── ...
│
├── Documentation/
│   └── ...
│
├── Results/
│   ├── Figures/
│   └── Tables/
│
└── README.md
```

---

# Key Contribution

The central contribution of OJAS is the integration of:

```text
Environmental Sensing
        +
Physics-Based Sound-Speed Estimation
        +
Frequency-Dependent Acoustic Modelling
        +
Correlation-Based Feedback
        +
Minimum-Energy Candidate Selection
        +
Software-Defined LFM Generation
```

into a compact real-time transmitter architecture.

Rather than replacing established sonar physics with a computationally heavy black-box model, OJAS uses a **lightweight physics-guided decision layer** to make the transmitter adaptive.

The objective is simple:

> **Transmit only as much acoustic energy as the current environment requires, while maintaining the required signal quality.**

---

# References

1. **Mackenzie, K. V. (1981).** *Nine-term equation for sound speed in the oceans.* Journal of the Acoustical Society of America, 70(3), 807–812. DOI: 10.1121/1.386920.

2. **MathWorks.** *underwaterSoundSpeed — Calculate speed of sound underwater based on temperature, salinity, and depth.* The documentation explicitly references Mackenzie (1981) among the established underwater sound-speed formulations.

3. **Francois, R. E., & Garrison, G. R.** Frequency-dependent absorption of sound in seawater and its use in underwater acoustic modelling.

4. **MathWorks.** *Linear Frequency Modulated Pulse Waveforms.* Documentation for LFM waveform generation and time-bandwidth characteristics.

5. **MathWorks.** *Underwater Acoustic Channels / Bellhop.* Higher-fidelity tools for underwater propagation, multipath and transmission-loss modelling.

---

# Disclaimer

OJAS is a research and engineering prototype.

The simulated energy-saving values are **model-based estimates**, not experimentally measured electrical power savings. Actual system performance depends on the acoustic transducer, power amplifier, matching network, receiver, environmental conditions and measured underwater propagation characteristics.

The candidate frequency range used in simulation represents the **OJAS research search space** and should not be interpreted as the operating specification of any particular commercial sonar.

---

## OJAS

**Optimization-guided Joule-aware Adaptive Sonar**

> **Sense the environment.
> Understand the physics.
> Correlate the reality.
> Adapt the transmission.
> Minimize the energy.**
