/*
  SIH26058 - 3-SENSOR ADAPTIVE DEMO
  HC-SR04 MORE SENSITIVE / STABLE VERSION

  Inputs:
    A0 -> Temperature potentiometer
    A1 -> Depth potentiometer
    D7 -> HC-SR04 TRIG
    D8 -> HC-SR04 ECHO

  HC-SR04 demo range:
    approximately 2.0 cm to 5.0 cm

  Center frequencies:
    C1 ->  50 kHz
    C2 -> 100 kHz
    C3 -> 200 kHz
    C4 -> 350 kHz
    C5 -> 500 kHz

  Bandwidth:
    CONSTANT = 50 kHz

  Sensitivity tuning:
    - 3-sample average instead of 5
    - hysteresis reduced from 0.15 cm to 0.05 cm
    - mode changes still use adjacent-state hysteresis
    - small sensor noise should be filtered, but deliberate hand
      movement should change the mode more easily

  Serial packet:
    DATA,T_C,Depth_m,Range_cm,fc_kHz,BW_kHz,Mode
*/

const byte TEMP_PIN  = A0;
const byte DEPTH_PIN = A1;

const byte TRIG_PIN = 7;
const byte ECHO_PIN = 8;


// ============================================================
// POTENTIOMETER MAPPINGS
// ============================================================
const float T_MIN = -2.0;
const float T_MAX = 30.0;

const float DEPTH_MIN_M = 0.0;
const float DEPTH_MAX_M = 8000.0;


// ============================================================
// HC-SR04 DEMO RANGE
// ============================================================
const float RANGE_MIN_CM = 2.0;
const float RANGE_MAX_CM = 5.0;


// ============================================================
// SONAR
// ============================================================
const float BW_KHZ = 50.0;


// ============================================================
// FREQUENCY MODES
// ============================================================
const float MODE_FC[5] = {
  50.0,
  100.0,
  200.0,
  350.0,
  500.0
};

const char* MODE_NAME[5] = {
  "C1",
  "C2",
  "C3",
  "C4",
  "C5"
};


// ============================================================
// MODE BOUNDARIES
//
// Nominal boundaries:
// C1/C2 = 2.375 cm
// C2/C3 = 3.125 cm
// C3/C4 = 3.875 cm
// C4/C5 = 4.625 cm
//
// Smaller hysteresis => more sensitive.
// ============================================================
const float BOUNDARY_12 = 2.375;
const float BOUNDARY_23 = 3.125;
const float BOUNDARY_34 = 3.875;
const float BOUNDARY_45 = 4.625;

// Reduced from 0.15 cm to 0.05 cm
const float HYSTERESIS_CM = 0.05;


// ============================================================
// STATE
// ============================================================
int currentMode = 2;       // Start at C3 = 200 kHz
float lastValidRange = 3.50;


// ============================================================
// TEMPERATURE
// ============================================================
float readTemperature()
{
  int adc = analogRead(TEMP_PIN);

  return T_MIN +
         (T_MAX - T_MIN) *
         ((float)adc / 1023.0);
}


// ============================================================
// DEPTH
// ============================================================
float readDepth()
{
  int adc = analogRead(DEPTH_PIN);

  return DEPTH_MIN_M +
         (DEPTH_MAX_M - DEPTH_MIN_M) *
         ((float)adc / 1023.0);
}


// ============================================================
// ONE HC-SR04 MEASUREMENT
// ============================================================
float readDistanceCmOnce()
{
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(5);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration =
      pulseIn(ECHO_PIN, HIGH, 30000UL);

  if (duration == 0)
    return -1.0;

  float distance_cm = duration / 58.0;

  if (distance_cm < 1.0 || distance_cm > 400.0)
    return -1.0;

  return distance_cm;
}


// ============================================================
// 3-SAMPLE AVERAGE
//
// This is deliberately less smoothing than the previous 5-sample
// version so the system reacts faster to intentional movement.
// ============================================================
float readDistanceCm()
{
  const byte NUM_SAMPLES = 3;

  float sum = 0.0;
  byte valid = 0;

  for (byte i = 0; i < NUM_SAMPLES; i++)
  {
    float d = readDistanceCmOnce();

    if (d > 0.0)
    {
      sum += d;
      valid++;
    }

    delay(15);
  }

  if (valid == 0)
    return -1.0;

  return sum / valid;
}


// ============================================================
// HYSTERESIS-BASED MODE SELECTION
// ============================================================
int updateModeWithHysteresis(float range_cm)
{
  // C1
  if (currentMode == 0)
  {
    if (range_cm > BOUNDARY_12 + HYSTERESIS_CM)
      currentMode = 1;
  }

  // C2
  else if (currentMode == 1)
  {
    if (range_cm < BOUNDARY_12 - HYSTERESIS_CM)
      currentMode = 0;
    else if (range_cm > BOUNDARY_23 + HYSTERESIS_CM)
      currentMode = 2;
  }

  // C3
  else if (currentMode == 2)
  {
    if (range_cm < BOUNDARY_23 - HYSTERESIS_CM)
      currentMode = 1;
    else if (range_cm > BOUNDARY_34 + HYSTERESIS_CM)
      currentMode = 3;
  }

  // C4
  else if (currentMode == 3)
  {
    if (range_cm < BOUNDARY_34 - HYSTERESIS_CM)
      currentMode = 2;
    else if (range_cm > BOUNDARY_45 + HYSTERESIS_CM)
      currentMode = 4;
  }

  // C5
  else if (currentMode == 4)
  {
    if (range_cm < BOUNDARY_45 - HYSTERESIS_CM)
      currentMode = 3;
  }

  return currentMode;
}


// ============================================================
// SETUP
// ============================================================
void setup()
{
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  digitalWrite(TRIG_PIN, LOW);

  Serial.begin(115200);

  delay(1000);

  Serial.println("# SIH26058 3-sensor adaptive demo");
  Serial.println("# A0 = Temperature POT");
  Serial.println("# A1 = Depth POT");
  Serial.println("# D7 = HC-SR04 TRIG");
  Serial.println("# D8 = HC-SR04 ECHO");
  Serial.println("# HC-SR04 range = 2-5 cm demo region");
  Serial.println("# 3-sample averaging + 0.05 cm hysteresis");
  Serial.println("# BW = CONSTANT 50 kHz");
  Serial.println("# Format: DATA,T_C,Depth_m,Range_cm,fc_kHz,BW_kHz,Mode");
}


// ============================================================
// MAIN LOOP
// ============================================================
void loop()
{
  float temperature = readTemperature();
  float depth_m = readDepth();

  // Smoothed range measurement
  float newRange = readDistanceCm();

  if (newRange > 0.0)
  {
    lastValidRange = newRange;
  }

  float range_cm = lastValidRange;

  // Update mode with reduced hysteresis
  currentMode = updateModeWithHysteresis(range_cm);


  // ----------------------------------------------------------
  // Diagnostic line
  // ----------------------------------------------------------
  Serial.print("SENSOR,Temp=");
  Serial.print(temperature, 2);

  Serial.print(",Depth=");
  Serial.print(depth_m, 1);

  Serial.print(",Range=");
  Serial.print(range_cm, 2);

  Serial.print("cm,Mode=");
  Serial.print(MODE_NAME[currentMode]);

  Serial.print(",fc=");
  Serial.print(MODE_FC[currentMode], 1);

  Serial.println("kHz");


  // ----------------------------------------------------------
  // MATLAB DATA packet
  // ----------------------------------------------------------
  Serial.print("DATA,");
  Serial.print(temperature, 2);
  Serial.print(",");
  Serial.print(depth_m, 1);
  Serial.print(",");
  Serial.print(range_cm, 2);
  Serial.print(",");
  Serial.print(MODE_FC[currentMode], 1);
  Serial.print(",");
  Serial.print(BW_KHZ, 1);
  Serial.print(",");
  Serial.println(MODE_NAME[currentMode]);


  // Short cycle delay for responsiveness
  delay(50);
}
