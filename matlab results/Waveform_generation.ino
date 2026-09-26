/*
  SIH26058 - Arduino -> PC Adaptive Frequency Stream

  Hardware:
    Pot 1 wiper -> A0 : Temperature
    Pot 2 wiper -> A1 : Depth
    Pot outer terminals -> 5V and GND

  Fixed for current prototype:
    Salinity   = 35 ppt
    Turbidity  = 100 NTU

  This sketch DOES NOT generate the 100/200 kHz acoustic waveform.
  It reads the two potentiometers, applies the same 5-row MATLAB LUT
  used in the working prototype, and streams the selected center
  frequency to the PC.

  The PC Python program then synthesizes and displays the LFM waveform.
*/

const int TEMP_PIN  = A0;
const int DEPTH_PIN = A1;

const float T_MIN = -2.0;
const float T_MAX = 30.0;
const float D_MIN = 0.0;
const float D_MAX = 8000.0;

const float FIXED_SALINITY  = 35.0;
const float FIXED_TURBIDITY = 100.0;

// Global constant bandwidth: 50 kHz
const float BW_KHZ = 50.0;

// Same 5-row MATLAB prototype LUT that already gave 100/200 kHz changes.
struct LutRow {
  float T;
  float depth;
  const char* mode;
  float fc_kHz;
};

LutRow lut[] = {
  { 1.2, 7680.0, "C1",  50.0 },
  { 7.6, 5600.0, "C2", 100.0 },
  {14.0, 3040.0, "C3", 200.0 },
  {20.4,  960.0, "C3", 200.0 },
  {26.8,   80.0, "C2", 100.0 }
};

const int LUT_N = sizeof(lut) / sizeof(lut[0]);

float adcToTemperature(int adc) {
  return T_MIN + (T_MAX - T_MIN) * ((float)adc / 1023.0);
}

float adcToDepth(int adc) {
  return D_MIN + (D_MAX - D_MIN) * ((float)adc / 1023.0);
}

int nearestLut(float T, float depth) {
  // Normalize dimensions so temperature and depth contribute comparably.
  float bestDist = 1.0e30;
  int bestIdx = 0;

  for (int i = 0; i < LUT_N; i++) {
    float dT = (T - lut[i].T) / (T_MAX - T_MIN);
    float dD = (depth - lut[i].depth) / (D_MAX - D_MIN);

    float dist2 = dT*dT + dD*dD;

    if (dist2 < bestDist) {
      bestDist = dist2;
      bestIdx = i;
    }
  }

  return bestIdx;
}

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("# SIH26058 adaptive frequency stream");
  Serial.println("# Format: DATA,T,Depth,fc_kHz,BW_kHz,Mode");
}

void loop() {
  int adcT = analogRead(TEMP_PIN);
  int adcD = analogRead(DEPTH_PIN);

  float T = adcToTemperature(adcT);
  float depth = adcToDepth(adcD);

  int idx = nearestLut(T, depth);

  Serial.print("DATA,");
  Serial.print(T, 2);
  Serial.print(",");
  Serial.print(depth, 1);
  Serial.print(",");
  Serial.print(lut[idx].fc_kHz, 1);
  Serial.print(",");
  Serial.print(BW_KHZ, 1);
  Serial.print(",");
  Serial.println(lut[idx].mode);

  delay(100);
}
