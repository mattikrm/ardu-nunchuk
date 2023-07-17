#include <Wire.h>
#include <Nunchuk.h>

using namespace communication;
constexpr const uint8_t PIN_LVLSHFT_NUNCHUK{11};
Nunchuk dev{PIN_LVLSHFT_NUNCHUK, 100, 50, ClockMode::I2C_CLOCK_FAST_400_kHz};


void setup()
{
  Serial.begin(115200);
  delay(3000);
  Serial.println("Serieller Monitor initialisiert");

  // Nunchuk initialisieren
  dev.begin();
}

void loop()
{
  // Messwerte auslesen
  if(dev.read() != State::NO_DATA_AVAILABLE)
  {
    Serial.println();
    // Messwerte auslesen
    dev.print();
  }

  delayMicroseconds(1000);
}