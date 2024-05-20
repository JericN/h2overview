
#define pressureSensorPin     A0
#define pressureOffset        0.483

#define flowSensorPin         D2
#define flowCalibrationFactor 4.5 // TODO: Calibrate this value

#define solenoidRelayPin      D3
#define SOLENOID_OPEN         0x1
#define SOLENOID_CLOSED       0x0

// Global variables for the flow sensor
volatile long pulse;
float volume = 0;
float lastTime = 0.0;


// =============================================================================
// ============================== PHYSICAL LAYER ===============================
// =============================================================================

// Read the pressure sensor and convert it to psi
float read_pressure() {
  float pressureVoltage = analogRead(pressureSensorPin) * 5.0 / 1023.0;
  float pressure = (pressureVoltage - pressureOffset) * (100.0 / (4.5 - baselinePressureVoltage));
  return pressure;
}

// Read the water volume in L
float read_water_volume() { return volume; }

// Read the water flow rate in L/s
float read_waterflow_rate() {
  if (millis() - lastTime > 1000) {
    pulse = 0;
    lastTime = millis();
  }
  return 2.663 * pulse;;
}

// Read the solenoid state. 0x1 is open and 0x0 is closed
uint_8t get_solenoid_state() { return digitalRead(solenoidRelayPin); }

// Set the solenoid state. 0x1 is open and 0x0 is closed
void set_solenoid_state(uint_8t STATE) { digitalWrite(solenoidRelayPin, STATE); }


// =============================================================================
// ============================= DEVICE FUNCTIONS ==============================
// =============================================================================

// Once the solenoid button is pressed, the solenoid state will be toggled
void switch_solendoid_state() {
  if (get_solenoid_state() == SOLENOID_OPEN) { set_solenoid_state(SOLENOID_CLOSED); }
  else { set_solenoid_state(SOLENOID_OPEN); }
}

// Update the volume of the water every loop
void update_volume() { volume = volume + read_waterflow_rate(); }

// Update pulse in Hz
void pulse_counter() { pulse++; }


// =============================================================================
// ============================== SETUP AND LOOP ===============================
// =============================================================================

void setup() {
  // Initialize the serial communication
  Serial.begin(9600);

  // Initialize the pins
  pinMode(flowSensorPin, INPUT);
  pinMode(solenoidRelayPin, OUTPUT);

  // FIXME: Start the solenoid with solenoid state in the database
  digitalWrite(solenoidRelayPin, LOW);

  // Attach the interrupt to the flow sensor
  attachInterrupt(digitalPinToInterrupt(flowSensorPin), pulse_counter, RISING);

}

void loop() {
  Serial.print("Volume: ");
  Serial.print(read_water_volume());
  Serial.println(" mL");

  Serial.print("Flow Rate: ");
  Serial.print(read_waterflow_rate());
  Serial.println(" mL/s");

  Serial.print("Pressure: ");
  Serial.print(read_pressure());
  Serial.println(" psi");

  delay(5000);
}

