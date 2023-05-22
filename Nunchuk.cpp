/**
 * Copyright (c) 2022, Mattheo Krümmel
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

 /**
  * @section LICENSE
  *
  * This library is free software: you can redistribute it and/or modify
  * it under the terms of the GNU Lesser General Public License as published by
  * the Free Software Foundation, version 3 or (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful, but
  * WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  * Lesser General Public License for more details.
  *
  * You should have received a copy of the GNU Lesser General Public License
  * along with this program. If not, see <http://www.gnu.org/licenses/>.
  */

  /**
   * @file   Nunchuk.cpp
   * 
   * @brief  Klassenimplementierung für Wii Nunchuk und Kommunikation über I2C.
   *         Verwendet die Klasse Wire zur Kommunikation mit dem Nunchuk.
   * 
   * @author Mattheo Krümmel
   * 
   * @date   22-07-2020
   */

#include "Nunchuk.h"

#include <Wire.h>

namespace communication
{
void serialwrite(const char* mode = nullptr, const char* annotation = nullptr)
{
  if (!mode || !annotation)
    return;

  Serial.print("(");
  Serial.print(mode);
  Serial.print(") ");
  Serial.println(annotation);
}

void serialverbose(const char* annotation = nullptr)
{
  if constexpr (debugmode > 1)
  {
  if (!annotation)
    return;

  serialwrite("verbose", annotation);
  }
}

void serialinfo(const char* annotation = nullptr)
{
  if constexpr (debugmode > 0)
  {
  if (!annotation)
    return;

  serialwrite("info", annotation);
  }
}

void serialerror(const char* annotation = nullptr, const ExitCode code = 0x00)
{
  if constexpr (debugmode >= 0)
  {
  if (!annotation)
    return;

  static const char buffer[270] = {0x00};
  static_assert(sizeof(annotation) <= 255, "Annotation zu lang. Maximal 255 Zeichen zulässig.");

  sprintf(buffer, "%s (Code: %X)", annotation, static_cast<uint16_t>(code));
  serialwrite("error", buffer);
  }
}

    Nunchuk::Nunchuk(const unsigned long cycletime = 30, const uint8_t addr = Control::ADDR_NUNCHUK, const ClockMode mode = ClockMode::I2C_CLOCK_FAST_400_kHz)
        : m_addr { addr },
        m_pinLevelshifter { 0xFF },
        m_raw { 0x00 },
        m_isConnected{ false },
        m_lastError{ ExitCodes::NO_ERROR },
        m_cycleTime { cycletime },
        m_lastFetch { millis(); }
    {
      Wire.begin();
      Wire.setClock(static_cast<uint32_t>(mode));
    }

    explicit Nunchuk::Nunchuk(const uint8_t lvlshft, const unsigned long cycletime = 30, const uint8_t addr = Control::ADDR_NUNCHUK, const ClockMode mode = ClockMode::I2C_CLOCK_FAST_400_kHz)
        : m_addr { addr },
        m_pinLevelshifter { lvlshft },
        m_raw { 0x00 },
        m_isConnected{ false },
        m_lastError{ ExitCodes::NO_ERROR },
        m_cycleTime { cycletime },
        m_lastFetch { millis(); }
    {
      Wire.begin();
      Wire.setClock(static_cast<uint32_t>(mode));

      pinMode(m_pinLevelshifter, OUTPUT);
      digitalWrite(m_pinLevelshifter, LOW);
    }

    Nunchuk::~Nunchuk()
    {
      Wire.end();
    }

    const uint8_t Nunchuk::getAddress() const
    {
        return m_addr;
    }

    const bool Nunchuk::isConnected() const
    {
        return m_isConnected;
    }

    const ExitCode getLastError() const
    {
      return m_lastError;
    }

    ExitCode Nunchuk::begin()
    {
        serialverbose("Nunchuk-Initialisierung gestartet.");

        // Initialisierungssequen
        enable();

        Wire.beginTransmission(m_addr);
        // erstes Initialisierungsregister
        Wire.write(static_cast<uint8_t>(0xF0));
        // auf ersten Initialisierungswert setzen
        Wire.write(static_cast<uint8_t>(0x55));
        Wire.endTransmission(true);

        delay(1);

        Wire.beginTransmission(m_addr);
        // zweites Initialisierungsregister
        Wire.write(static_cast<uint8_t>(0xFB));
        // auf zweiten Initialisierungswert setzen
        Wire.write(static_cast<uint8_t>(0x00));

        disable();

        delay(1);

        switch (Wire.endTransmission(true))
        {
        case WireReturnCode::SUCCESS:
            m_isConnected = true;
            serialinfo("Nunchuk-Initalisierung erfolgreich.");
            return ExitCode::NO_ERROR;

        case WireReturnCode::DATA_TOO_LONG:
          serialerror("Übertragungsfehler: Zu viele Daten für Übertragungspuffer.", ExitCode::BAD_VALUE);
          m_lastError = ExitCode::BAD_VALUE;
          [[fallthrough]];

        case WireReturnCode::NACK_ON_ADDR:
          serialerror("Übertragungsfehler: NACK erhalten bei Übertragung der Adresse.", ExitCode::BAD_VALUE);
          m_lastError = ExitCode::BAD_VALUE;
          [[fallthrough]];
          
        case WireReturnCode::NACK_ON_DATA:
          serialerror("Übertragungsfehler: NACK erhalten bei Übertragung der Daten.", ExitCode::BAD_VALUE);
          m_lastError = ExitCode::BAD_VALUE;
          [[fallthrough]];

        case WireReturnCode::OTHER:
          serialerror("Übertragungsfehler: Allgemeiner Fehler.", ExitCode::ERROR_OCCURED);
          m_lastError = ExitCode::ERROR_OCCURED;
          [[fallthrough]];

        case WireReturnCode::TIMEOUT:
          serialerror("Übertragungsfehler: Nunchuk braucht zu lange zum Antworten.", ExitCode::TIMEOUT);
          m_lastError = ExitCode::TIMEOUT;
          [[fallthrough]];

        default:
          m_isConnected = false;
          break;
        }

        return m_lastError;
    }

    ExitCode Nunchuk::read()
    {
      // erst lesen, wenn die Zykluszeit vorbei ist
      if ((millis() - m_lastFetch) < m_cycletime)
        return ExitCode::NO_ERROR;

      m_lastFetch = millis();

      // Falls das Gerät nicht verbunden/initialisiert ist zweimal versuchen, sonst mit Fehler
      // zurückkehren
      for (int i = 1; i < 3; i++)
      {
        if (m_isConnected)
        {
          serialinfo("Nunchuk bereit zur Kommunikation");            
          break;
        }
        else
        {
          begin();
        }
      }

      if (!m_isConnected)
      {
          serialerror("Verbindungsaufbau nach 3 Versuchen fehlgeschlagen.", ExitCode::NOT_CONNECTED);
          return m_lastError = ExitCode::NOT_CONNECTED;
      }
      
      // Rohdaten vom Gerät anfordern
      enable();

      Wire.beginTransmission(m_addr);
      Wire.write(Control::REG_RAW_DATA);
      Wire.endTransmission(true);

      delayMicroseconds(1);

      if (Wire.requestFrom(m_addr, Control::LEN_RAW_DATA) != Control::LEN_RAW_DATA)
      {
          disable();

          // falls Fehler bei der Kommunikation, das Gerät als getrennt markieren und mit
          // Fehler zurückkehren
          m_isConnected = false;
          serialerror("Übertragung fehlgeschlagen.", ExitCode::NOT_CONNECTED);
          return m_lastError = ExitCode::NOT_CONNECTED;
      }
      disable();

      if constexpr (debugmode > 1)
      {
        auto msg = String("Anzahl der verfügbaren Bytes: ");
        msg += String(Wire.available());
        serialverbose(msg.c_str());
      }

      // empfangene Daten auslesen
      for (uint8_t i = 0; (i < Control::LEN_RAW_DATA) && Wire.available(); i++)
      {
          m_raw[i] = Wire.read();
      }

      // ggf. Rohdaten ausgeben
      if constexpr (debugmode > 0)
      {
        serialinfo("Rohdaten:");
          for (uint8_t i = 0; i < Control::LEN_RAW_DATA; i++)
          {
            Serial.print(m_raw[i], HEX);
            Serial.print(" ");
          }
          Serial.println();
      }

      return ExitCode::NO_ERROR;
    }

    const bool Nunchuk::decodeButtonZ() const
    {
        return !static_cast<bool>((m_raw[5] & Bitmasks::BUTTON_Z_STATE) >> 0);
    }

    const bool Nunchuk::decodeButtonC() const
    {
        return !static_cast<bool>((m_raw[5] & Bitmasks::BUTTON_C_STATE) >> 1);
    }

    const int16_t Nunchuk::decodeAccelerationX() const
    {
        return static_cast<int16_t>((m_raw[2] << 2) | (static_cast<uint16_t>((m_raw[5] >> 2) & Bitmasks::ACC_X_BIT_0_1))
            - Acceleration::X_NULL);
    }

    const int16_t Nunchuk::decodeAccelerationY() const
    {
        return static_cast<int16_t>((m_raw[3] << 2) | (static_cast<uint16_t>((m_raw[5] >> 4) & Bitmasks::ACC_Y_BIT_0_1))
            - Acceleration::Y_NULL);
    }

    const int16_t Nunchuk::decodeAccelerationZ() const
    {
        return static_cast<int16_t>((m_raw[4] << 2) | (static_cast<uint16_t>((m_raw[5]>> 6) & Bitmasks::ACC_Z_BIT_0_1) )
            - Acceleration::Z_NULL);
    }

    const int8_t Nunchuk::decodeJoystickX() const
    {
        return m_raw[0] - Joystick::X_NULL;
    }

    const int8_t Nunchuk::decodeJoystickY() const
    {
        return m_raw[1] - Joystick::Y_NULL;
    }

    void Nunchuk::print()
    {
      if (!m_isConnected)
      {
        serialerror("Es liegen keine neuen Sensorendaten vor.", ExitCodes::NO_DATA_AVAILABLE);
        m_lastError = ExitCodes::NO_DATA_AVAILABLE;
        return;
      }
      
      Serial.print("\nDaten (dezimale Werte)\n\n");
      Serial.print("Joystick:\t\t\tX = ");
      Serial.print(decodeJoystickX(), DEC);
      Serial.print("\tY = ");
      Serial.print(decodeJoystickY(), DEC);
      Serial.println();
      Serial.print("Beschleunigung:\tX = ");
      Serial.print(decodeAccelerationX(), DEC);
      Serial.print("\tY = ");
      Serial.print(decodeAccelerationY(), DEC);
      Serial.print("\tZ = ");
      Serial.print(decodeAccelerationZ(), DEC);
      Serial.println();
      Serial.print("Buttons:\n\tC = ");
      Serial.print(decodeButtonC() ? "gedrückt" : "nicht gedrückt");
      Serial.println();
      Serial.print("\tZ = ");
      Serial.print(decodeButtonZ() ? "gedrückt" : "nicht gedrückt");
      Serial.println();
    }

    void Nunchuk::enable() const
    {
      if (m_pinLevelshifter == 0xFF)
        return;

      serialverbose("Pegelwandler aktiviert.");
      digitalWrite(m_pinLevelshifter, HIGH);
    }

    void Nunchuk::disable() const
    {
      if (m_pinLevelshifter == 0xFF)
        return;
        
      serialverbose("Pegelwandler deaktiviert.")
      digitalWrite(m_pinLevelshifter, LOW);
    }
}