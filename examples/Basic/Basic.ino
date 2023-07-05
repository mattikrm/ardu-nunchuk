#include <Wire.h>
#include <Nunchuk.h>

using namespace communication;
constexpr const uint8_t PIN_LVLSHFT_NUNCHUK{11};
Nunchuk dev{100, 50, ClockMode::I2C_CLOCK_FAST_400_kHz};


void setup()
{
  Serial.begin(115200);
  Serial.println("Serieller Monitor initialisiert");

  // Nunchuk initialisieren
  dev.begin();
}

void loop()
{
  Serial.println();

  // Messwerte auslesen
  dev.read();

  // Messwerte auslesen
  dev.print();
}