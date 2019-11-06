#include <TeensyDMX.h>

namespace teensydmx = ::qindesign::teensydmx;

// Create the DMX sender on Serial1

teensydmx::Sender dmxTx{Serial1};

//Don't set to lower than 8!!!
const int nrOfPixels = 8;

float dutyCycles[nrOfPixels];
bool pixelStates[nrOfPixels];

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(23, OUTPUT);

  Serial.begin(115200);
  for (int i = 0; i < nrOfPixels; i++) {
    dutyCycles[i] = 0;
    pixelStates[i] = true;
  }

  // switchToSerial();
  switchToDMX();

  delay(500);
  Serial.printf("SKETCH STARTED!!\n");
}

bool dmxModeActive = false;

void switchToSerial() {
  dmxModeActive = false;
  dmxTx.end();
  delay(500);
  Serial2.begin(9600, SERIAL_8N1);
  Serial2.transmitterEnable(2);
}

void switchToDMX() {
  Serial2.clear();
  Serial2.flush();
  Serial2.end();
  dmxModeActive = true;
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);

  delay(500);
  dmxTx.setPacketSize(75);
  dmxTx.begin();
  dmxTx.pause();
}

const float frequency = 30;
const int period = 1000 / frequency;
float duty = 0.5;

bool cycleActive = false;
elapsedMillis now;
elapsedMillis sinceLoop;
uint32_t mainLoopDuration;

elapsedMillis sincePrint;
uint32_t printInterval = 50;

elapsedMillis sinceDMXPacket;
uint32_t actualOnDuration = 123456789;
uint32_t actualOffDuration = 987654321;

elapsedMillis sinceCycleStart = 0;

// uint timeSlot = 5;

void loop() {

  // setAllPixels();
  // pixelStates[0] = false;

  // pixelStates[2] = false;
  // updateAllDMXChannels();
  // delay(300);
  // return;

  mainLoopDuration = sinceLoop;
  sinceLoop = 0;
  if (sincePrint >= printInterval) {
    sincePrint = 0;
    // timeSlot++;
    // if (timeSlot > 10)
    // {
    //   timeSlot = 4;
    // }
    // Serial.printf("timeSlot: %i \n", timeSlot);
    // Serial.printf("on: %u ms \t off: %u ms. MAINLOOP: %u ms\n", actualOnDuration, actualOffDuration, mainLoopDuration);

    Serial.printf("dutyCycles: %f,\t%f,\t%f\n", dutyCycles[0], dutyCycles[1], dutyCycles[2]);
  }

  for (int i = 0; i < 3; i++) {
    int potValue = analogRead(14 + i);
    dutyCycles[i] = potValue / 1023.f;
  }

  if (sinceCycleStart > period) {
    sinceCycleStart = 0;
    // Serial.printf("Staring new cycle\n\n");
    //START A NEW CYCLE
    setAllPixels();
    updateAllDMXChannels();
  }

  bool anyPixelChanged = false;
  for (int i = 0; i < nrOfPixels; i++) {
    if (pixelStates[i] && sinceCycleStart >= (int)(dutyCycles[i] * period)) {
      // Serial.printf("turning off pixel %i @ %u \n", i, (uint32_t)sinceCycleStart);
      //Turn off that pixel
      pixelStates[i] = false;
      anyPixelChanged = true;

      // Set the channel that holds that pixel
      int packetIndex = i / 8;
      uint8_t sendValues = 0;
      for (int j = 0; j < 8; j++) {
        int pixelIndex = packetIndex * sizeof(uint8_t) + j;
        sendValues |= (int)pixelStates[pixelIndex] << j;
      }
      dmxTx.set(packetIndex + 1, sendValues);

      // for (int i = 0; i < dmxTx.packetSize(); i++) {
      //   uint8_t sendValues = 0;
      //   for (int j = 0; j < 8; j++) {
      //     int pixelIndex = i * 8 + j;
      //     sendValues |= (int)pixelStates[pixelIndex] << j;
      //   }

      //   dmxTx.set(i + 1, sendValues);
      // }
      // dmxTx.resumeFor(1);
    }
  }

  if (anyPixelChanged) {
    // updateAllDMXChannels();
    dmxTx.resumeFor(1);
  }

  return;

  // int potValue = analogRead(15);
  // duty = potValue / 1023.f;

  uint onPeriod = period * duty;
  uint offPeriod = period * (1.0f - duty);

  // Serial.printf("duty: %f\n", duty);
  // analogWrite(23, potValue / 4);
  // digitalWrite(23, HIGH);
  // delay(onPeriod);

  // digitalWrite(23, LOW);
  // delay(offPeriod);
  // return;

  while (Serial.available()) {
    char byte = Serial.read();
    if (byte == 'q') {
      cycleActive = !cycleActive;
    } else if (byte == 'x') {
      Serial.printf("switching to DMX mode\n");
      switchToDMX();
    } else if (byte == 's') {
      Serial.printf("switching to SERIAL mode\n");
      switchToSerial();
    } else if (!dmxModeActive) {
      Serial2.write(byte);
    }
  }

  if (dmxModeActive) {

    digitalWrite(LED_BUILTIN, HIGH);
    for (int i = 1; i <= 10; i++) {
      dmxTx.set(i, 0xFF);
    }

    sinceDMXPacket = 0;
    dmxTx.resumeFor(1);
    while (sinceDMXPacket < onPeriod || dmxTx.isTransmitting())
    // while (dmxTx.isTransmitting())
    {
      // yield();
    }
    actualOnDuration = sinceDMXPacket;
    // delay(onPeriod);

    digitalWrite(LED_BUILTIN, LOW);
    for (int i = 1; i <= 10; i++) {
      dmxTx.set(i, 0x00);
    }
    sinceDMXPacket = 0;
    dmxTx.resumeFor(1);
    while (sinceDMXPacket < offPeriod || dmxTx.isTransmitting()) {
      // yield();
    }
    actualOffDuration = sinceDMXPacket;
    // delay(offPeriod);
  } else {
    while (Serial2.available()) {
      Serial.write(Serial2.read());
    }

    if (cycleActive) {
      Serial2.printf("HFFFFFFFF");
      Serial.printf("Sending: HFFFFFFFF\n");
      delay(500);
      Serial2.printf("H00000000");
      Serial.printf("Sending: H00000000\n");
      delay(500);
    }
  }
}

void setAllPixels() {
  for (int i = 0; i < nrOfPixels; i++) {
    pixelStates[i] = true;
  }
}

void unSetAllPixels() {
  for (int i = 0; i < nrOfPixels; i++) {
    pixelStates[i] = false;
  }
}

void updateAllDMXChannels() {
  // for (int i = 0; i < dmxTx.packetSize(); i++) {
  int i = 0;
  uint8_t sendValues = 0;
  for (int j = 0; j < 8; j++) {
    int index = i * 8 + j;
    sendValues |= ((int)pixelStates[index] << j);
    // Serial.printf("pixelStates[%i]: %i\n", index, pixelStates[i]);
  }
  // Serial.printf("packet: %u \n", sendValues);

  dmxTx.set(i + 1, sendValues);
  // }
  dmxTx.resumeFor(1);
}