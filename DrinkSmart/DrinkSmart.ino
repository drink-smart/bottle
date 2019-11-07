#include <NanoESP.h>
#include <SoftwareSerial.h>

const byte GYRO_THRESHOLD = 40;
const byte PIN_GYRO_X = A0;
const byte PIN_GYRO_Y = A1;
const byte PIN_GYRO_Z = A2;
const byte PIN_FLOW_SENSOR = 2;
const byte IRQ_FLOW_SENSOR = 0;

unsigned int gyroBaseX;
unsigned int gyroBaseY;
unsigned int gyroBaseZ;
unsigned long gyroPrevMillis = 0;

unsigned long flowPrevMillis = 0;
volatile byte flowPulseCount = 0;
float flowRate = 0.0;
unsigned int flowMilliLitres = 0;
unsigned long totalMilliLitres = 0;

#define SSID ""
#define PASSWORD ""

NanoESP nanoesp = NanoESP();

void setup() {
  Serial.begin(19200);

  nanoesp.init();

  // Station Mode, Connect to WiFi
  if (!nanoesp.configWifi(STATION, SSID, PASSWORD)) {
    Serial.println(F("Error: WLAN not Connected\n"));
  }
  else {
    Serial.println(F("WLAN Connected\n"));
    digitalWrite(LED_BUILTIN, HIGH);
  }

  // Print IP in Terminal
  Serial.println(nanoesp.getIp());

  /*
    pinMode(PIN_FLOW_SENSOR, INPUT);
    //digitalWrite(PIN_FLOW_SENSOR, HIGH);

    calibrateGyro();

    attachInterrupt(IRQ_FLOW_SENSOR, flowPulseCounter, RISING);
  */
}

void loop() {
  unsigned long currentMillis = millis();
  if ((currentMillis - flowPrevMillis) > 1000) {
    detachInterrupt(IRQ_FLOW_SENSOR);

    // 5Q is the pulse frequency of our sensor (YF-S201C) with Q equals the flow rate in l/min
    flowRate = ((1000.0 / (currentMillis - flowPrevMillis)) * flowPulseCount) / 5.0;
    flowMilliLitres = (flowRate / 60) * 1000.0;
    totalMilliLitres += flowMilliLitres;

    Serial.println(totalMilliLitres);

    // get going with the input again
    flowPulseCount = 0;
    flowPrevMillis = millis();
    attachInterrupt(IRQ_FLOW_SENSOR, flowPulseCounter, RISING);
  }

  // the bottle might have been moved to a new position at a different angle than before,
  // thus, we need to reset the gyro baseline every now and then
  if ((currentMillis - gyroPrevMillis) > 10000) {
    gyroPrevMillis = currentMillis;
    calibrateGyro();
  }
  isTiltedUp();

  delay(900);
}

boolean isTiltedUp() {
  int diffX = analogRead(PIN_GYRO_X) - gyroBaseX;
  int diffY = analogRead(PIN_GYRO_Y) - gyroBaseY;
  int diffZ = analogRead(PIN_GYRO_Z) - gyroBaseZ;


  Serial.println(diffX);
  Serial.println(diffY);
  Serial.println(diffZ);


  boolean result = (abs(diffX) > GYRO_THRESHOLD)
                   || (abs(diffY) > GYRO_THRESHOLD)
                   || (abs(diffZ) > GYRO_THRESHOLD) ;

  Serial.print("isTiltedUp() RETURN ");
  Serial.println(result);
  return result;
}

void calibrateGyro() {
  gyroBaseX = analogRead(PIN_GYRO_X);
  gyroBaseY = analogRead(PIN_GYRO_Y);
  gyroBaseZ = analogRead(PIN_GYRO_Z);
}

/**
   Increment the pulse counter that indicates flowing juice.
*/
void flowPulseCounter()
{
  flowPulseCount++;
}
