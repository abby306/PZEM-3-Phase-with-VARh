#include <PZEM004Tv30.h>
#include <Preferences.h>

PZEM004Tv30 pzem(Serial2,16,17);  // Adjust based on your wiring

Preferences preferences;
double voltage, current, power, pf, energy, apparentPower, reactivePower,wh, varh = 0;
unsigned long lastUpdateTimeVAR = 0;
unsigned long lastUpdateTimeW = 0;
unsigned long disp_time = 0;


void saveVARh(){
preferences.begin("varh",false);
preferences.putDouble("value",varh);
preferences.end();
}

void getVARh(){
preferences.begin("varh",true);
varh = preferences.getDouble("value",0.0);
preferences.end();
}

void setup() {
    Serial.begin(115200);
    pinMode(13,OUTPUT);
    digitalWrite(13,HIGH);
}

void loop() {
    voltage = pzem.voltage();
    current = pzem.current();
    power = pzem.power();
    pf = pzem.pf();
    energy = pzem.energy();

    // Calculate Apparent Power (VA)
    apparentPower = voltage * current;

    // Calculate Reactive Power (VAR)
   // reactivePower = sqrt(pow(apparentPower, 2) - pow(power, 2));

  // Calculate Reactive Power (VAR) using Real Power and Power Factor
    if (pf != 0) {  // Prevent division by zero
        reactivePower = power * tan(acos(pf));
    } else {
        reactivePower = 0;  // Set to 0 if pf is zero to avoid undefined behavior
    }


    // Accumulate Reactive Energy (VARh)
    unsigned long currentTime = millis();
    if (lastUpdateTimeVAR > 0) {
        float timeInterval = (currentTime - lastUpdateTimeVAR) / 3600000.0; // Convert ms to hours
        varh += reactivePower * timeInterval;
    } 
    lastUpdateTimeVAR = currentTime;

    currentTime = millis();
    if (lastUpdateTimeW > 0) {
        float timeInterval = (currentTime - lastUpdateTimeW) / 3600000.0; // Convert ms to hours
        wh += power * timeInterval;
    } 
    lastUpdateTimeW = currentTime;

    if (millis()-disp_time>=5000){

    // Serial.println(apparentPower);
    // Serial.println(pow(apparentPower,2));
    // Serial.println(pow(power,2));

    // Serial.println(pow(apparentPower, 2) - pow(power, 2));

    // Print Results
Serial.print("Voltage: "); Serial.print(voltage, 10); Serial.println("V");
Serial.print("Current: "); Serial.print(current, 10); Serial.println("A");
Serial.print("Power: "); Serial.print(power, 10); Serial.println("W");
Serial.print("Power Factor: "); Serial.print(pf, 10); Serial.println();
Serial.print("Pzem Energy: "); Serial.print(energy, 10); Serial.println(" kWh");
Serial.print("Reactive Power: "); Serial.print(reactivePower, 10); Serial.println(" VAR");
Serial.print("Reactive Energy: "); Serial.print(varh, 10); Serial.println(" VARh");
Serial.print("My Energy: "); Serial.print(wh, 10); Serial.println(" Wh");

    disp_time = millis();
    }
    delay(50);
}
