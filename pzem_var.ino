#include <PZEM004Tv30.h>
#include <Preferences.h>

// Create PZEM object for reading electrical parameters
PZEM004Tv30 pzem(Serial2, 16, 17);  // TX = GPIO16, RX = GPIO17

//#define threephase  // Uncomment for three-phase measurement

// Preferences library for storing persistent data (Reactive Energy in VARh)
Preferences preferences;

// Variables to store electrical parameters for three channels
// Channel 1
double voltage1, current1, power1, pf1, energy1, apparentPower1, reactivePower1, wh1, varh1 = 0;
unsigned long lastUpdateTimeVAR1 = 0;

// Channel 2
double voltage2, current2, power2, pf2, energy2, apparentPower2, reactivePower2, w2, varh2 = 0;
unsigned long lastUpdateTimeVAR2 = 0;

// Channel 3
double voltage3, current3, power3, pf3, energy3, apparentPower3, reactivePower3, wh3, varh3 = 0;
unsigned long lastUpdateTimeVAR3 = 0;

// Function to save reactive energy (VARh) into non-volatile storage
void saveVARh() {
    preferences.begin("varh", false);
    preferences.putDouble("value1", varh1);
    preferences.putDouble("value2", varh2);
    preferences.putDouble("value3", varh3);
    preferences.end();
}

// Function to retrieve stored reactive energy (VARh) from non-volatile storage
void getVARh() {
    preferences.begin("varh", true);
    varh1 = preferences.getDouble("value1", 0.0);
    varh2 = preferences.getDouble("value2", 0.0);
    varh3 = preferences.getDouble("value3", 0.0);
    preferences.end();
}

// Task for running loop1() on the second core of ESP32
TaskHandle_t task_loop1;
void esploop1(void* pvParameters) {
    setup1();
    for (;;) loop1();
}

//Core 0
void setup() { 
    Serial.begin(115200);
    Serial.print("Main Core: ");
    Serial.println(xPortGetCoreID());
    
    // Create a task to run loop1() on the second core
    xTaskCreatePinnedToCore(
        esploop1,               // Task function
        "loop1",                // Name of task
        10000,                  // Stack size
        NULL,                   // Parameter of the task
        1,                      // Priority of the task
        &task_loop1,            // Task handle
        !ARDUINO_RUNNING_CORE   // Pin task to the other core
    );
    
    // Initialize LED on GPIO 13
    pinMode(13, OUTPUT);
    digitalWrite(13, HIGH);
    
    // Retrieve stored reactive energy values
    getVARh();
}
//Core 0 loop only for displaying values at 5s interval. You can do other processes here 
void loop() {
    // Print measured electrical parameters for each channel
    Serial.print("Voltage1: "); Serial.print(voltage1, 10); Serial.println("V");
    Serial.print("Current1: "); Serial.print(current1, 10); Serial.println("A");
    Serial.print("Power1: "); Serial.print(power1, 10); Serial.println("W");
    Serial.print("Power Factor1: "); Serial.print(pf1, 10); Serial.println();
    Serial.print("Pzem Energy1: "); Serial.print(energy1, 10); Serial.println(" kWh");
    Serial.print("Reactive Power1: "); Serial.print(reactivePower1, 10); Serial.println(" VAR");
    Serial.print("Reactive Energy1: "); Serial.print(varh1, 10); Serial.println(" VARh");
    Serial.println("---------------------------------------------");
    
    Serial.print("Voltage2: "); Serial.print(voltage2, 10); Serial.println("V");
    Serial.print("Current2: "); Serial.print(current2, 10); Serial.println("A");
    Serial.print("Power2: "); Serial.print(power2, 10); Serial.println("W");
    Serial.print("Power Factor2: "); Serial.print(pf2, 10); Serial.println();
    Serial.print("Pzem Energy2: "); Serial.print(energy2, 10); Serial.println(" kWh");
    Serial.print("Reactive Power2: "); Serial.print(reactivePower2, 10); Serial.println(" VAR");
    Serial.print("Reactive Energy2: "); Serial.print(varh2, 10); Serial.println(" VARh");
    Serial.println("---------------------------------------------");
    
    Serial.print("Voltage3: "); Serial.print(voltage3, 10); Serial.println("V");
    Serial.print("Current3: "); Serial.print(current3, 10); Serial.println("A");
    Serial.print("Power3: "); Serial.print(power3, 10); Serial.println("W");
    Serial.print("Power Factor3: "); Serial.print(pf3, 10); Serial.println();
    Serial.print("Pzem Energy3: "); Serial.print(energy3, 10); Serial.println(" kWh");
    Serial.print("Reactive Power3: "); Serial.print(reactivePower3, 10); Serial.println(" VAR");
    Serial.print("Reactive Energy3: "); Serial.print(varh3, 10); Serial.println(" VARh");
    Serial.println("---------------------------------------------");
    
    // Save updated reactive energy values
    saveVARh();
    delay(1000);

    // do other processes 
}

//Core 1
void setup1() {
    Serial.begin(115200);
    Serial.print("Reading Core: ");
    Serial.println(xPortGetCoreID());
}

//Core 1 loop dedicated to non stop calculation and accumulation of VARh
void loop1() {
    #ifdef threephase
        Serial.println("Measure from 3 PZEM sensors here");
    #else
        // Read values from PZEM sensor
        voltage1 = pzem.voltage();
        current1 = pzem.current();
        power1 = pzem.power();
        pf1 = pzem.pf();
        energy1 = pzem.energy();
        
        // Assign same values to other channels
        voltage2 = voltage1;
        current2 = current1;
        power2 = power1;
        pf2 = pf1;
        energy2 = energy1;
        
        voltage3 = voltage1;
        current3 = current1;
        power3 = power1;
        pf3 = pf1;
        energy3 = energy1;
    #endif
    
    // Calculate Apparent and Reactive Power
    apparentPower1 = voltage1 * current1;
    apparentPower2 = voltage2 * current2;
    apparentPower3 = voltage3 * current3;
    
    if (pf1 != 0) reactivePower1 = power1 * tan(acos(pf1));
    if (pf2 != 0) reactivePower2 = power2 * tan(acos(pf2));
    if (pf3 != 0) reactivePower3 = power3 * tan(acos(pf3));
    
    // Calculate Reactive Energy accumulation over time
    unsigned long currentTime = micros();
    if (lastUpdateTimeVAR1 > 0) varh1 += reactivePower1 * ((currentTime - lastUpdateTimeVAR1) / 3600000000.0);
    lastUpdateTimeVAR1 = currentTime;
    
    delay(10);
}
