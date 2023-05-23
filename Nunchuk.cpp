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

#include "Button.h"

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

void serialverbose(const char* annotation)
{
  if constexpr (debugmode > 1)
  {
  if (!annotation)
    return;

  serialwrite("verbose", annotation);
  }
}

void serialinfo(const char* annotation)
{
  if constexpr (debugmode > 0)
  {
  if (!annotation)
    return;

  serialwrite("info", annotation);
  }
}

void serialerror(const char* annotation, const State code)
{
  if constexpr (debugmode >= 0)
  {
  if (!annotation)
    return;

  static char buffer[270] = {0x00};
  static_assert(sizeof(annotation) <= 255, "Annotation zu lang. Maximal 255 Zeichen zulässig.");

  sprintf(buffer, "%s (Code: %X)", annotation, static_cast<uint16_t>(code));
  serialwrite("error", buffer);
  }
}

    Nunchuk::Nunchuk(const unsigned long cycletime, const ClockMode mode)
        : m_pinLevelshifter { 0xFF },
        m_raw { 0x00 },
        m_state{ State::BEGIN },
        m_cycletime { cycletime },
        m_lastFetch { millis() },
        m_buttonC {this},
        m_buttonZ {this}
    {
      Wire.begin();
      Wire.setClock(static_cast<uint32_t>(mode));
    }

    Nunchuk::Nunchuk(const uint8_t lvlshft, const unsigned long cycletime, const ClockMode mode)
        :  m_pinLevelshifter { lvlshft },
        m_raw { 0x00 },
        m_state{ State::BEGIN },
        m_cycletime { cycletime },
        m_lastFetch { millis() },
        m_buttonC {this},
        m_buttonZ {this}
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

    const bool Nunchuk::isConnected() const
    {
        return m_state == State::CONNECTED;
    }

    const State Nunchuk::getState() const
    {
      return m_state;
    }

    State Nunchuk::begin()
    {
      m_state = State::BEGIN;
      
      serialverbose("Nunchuk-Initialisierung gestartet.");

      // Initialisierungssequenz
      enable();

      Wire.beginTransmission(Control::ADDR_NUNCHUK);
      // erstes Initialisierungsregister
      Wire.write(static_cast<uint8_t>(0xF0));
      // auf ersten Initialisierungswert setzen
      Wire.write(static_cast<uint8_t>(0x55));
      Wire.endTransmission(true);

      delay(1);

      Wire.beginTransmission(Control::ADDR_NUNCHUK);
      // zweites Initialisierungsregister
      Wire.write(static_cast<uint8_t>(0xFB));
      // auf zweiten Initialisierungswert setzen
      Wire.write(static_cast<uint8_t>(0x00));

      switch (Wire.endTransmission(true))
      {
      case WireReturnCode::SUCCESS:
        serialinfo("Nunchuk-Initalisierung erfolgreich.");
        m_state = State::CONNECTED;
        break;

      case WireReturnCode::DATA_TOO_LONG:
        serialerror("Übertragungsfehler: Zu viele Daten für Übertragungspuffer.", State::BAD_VALUE);
        m_state = State::BAD_VALUE;
        [[fallthrough]];

      case WireReturnCode::NACK_ON_ADDR:
        serialerror("Übertragungsfehler: NACK erhalten bei Übertragung der Adresse.", State::BAD_VALUE);
        m_state = State::BAD_VALUE;
        [[fallthrough]];
        
      case WireReturnCode::NACK_ON_DATA:
        serialerror("Übertragungsfehler: NACK erhalten bei Übertragung der Daten.", State::BAD_VALUE);
        m_state = State::BAD_VALUE;
        [[fallthrough]];

      case WireReturnCode::OTHER:
        serialerror("Übertragungsfehler: Allgemeiner Fehler.", State::ERROR_OCCURED);
        m_state = State::ERROR_OCCURED;
        [[fallthrough]];

      case WireReturnCode::TIMEOUT:
        serialerror("Übertragungsfehler: Nunchuk braucht zu lange zum Antworten.", State::TIMEOUT);
        m_state = State::TIMEOUT;
        [[fallthrough]];

      default:
        m_state = State::NOT_CONNECTED;
        break;
      }
      disable();
      return m_state;
    }

    State Nunchuk::read()
    {
      switch (m_state)
      {
      case State::CONNECTED:
        // erst lesen, wenn die Zykluszeit vorbei ist
        if ((millis() - m_lastFetch) >= m_cycletime)
        {
          m_lastFetch = millis();
        }

        // Rohdaten vom Gerät anfordern
        enable();

        Wire.beginTransmission(Control::ADDR_NUNCHUK);
        Wire.write(Control::REG_RAW_DATA);
        Wire.endTransmission(true);

        delayMicroseconds(1);

        if (Wire.requestFrom(Control::ADDR_NUNCHUK, Control::LEN_RAW_DATA) != Control::LEN_RAW_DATA)
        {
  
            // falls Fehler bei der Kommunikation, das Gerät als getrennt markieren und mit
            // Fehler zurückkehren
            m_state = State::NOT_CONNECTED;
            serialerror("Übertragung fehlgeschlagen.", m_state);
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
        break;

      case State::NOT_CONNECTED:
        // Falls das Gerät nicht verbunden/initialisiert ist zweimal versuchen, sonst mit Fehler
        // zurückkehren
        for (int i = 1; i <= 3; i++)
        {
          begin();
          
          if (m_state == State::CONNECTED)
          {
            serialinfo("Nunchuk bereit zur Kommunikation");
            break;
          }
          else
          {
            serialerror("Verbindungsaufbau nach 3 Versuchen fehlgeschlagen.", m_state);
          }
        }
      
      default:
        m_state = State::ERROR_OCCURED;
        break;
      }

      return m_state;
    }

    const bool Nunchuk::decodeButtonZ() const
    {
        return !static_cast<bool>((m_raw[5] & Bitmask::BUTTON_Z_STATE) >> 0);
    }

    const bool Nunchuk::decodeButtonC() const
    {
        return !static_cast<bool>((m_raw[5] & Bitmask::BUTTON_C_STATE) >> 1);
    }

    const int16_t Nunchuk::decodeAccelerationX() const
    {
        return static_cast<int16_t>((m_raw[2] << 2) | (static_cast<uint16_t>((m_raw[5] >> 2) & Bitmask::ACC_X_BIT_0_1))
            - Acceleration::X_NULL);
    }

    const int16_t Nunchuk::decodeAccelerationY() const
    {
        return static_cast<int16_t>((m_raw[3] << 2) | (static_cast<uint16_t>((m_raw[5] >> 4) & Bitmask::ACC_Y_BIT_0_1))
            - Acceleration::Y_NULL);
    }

    const int16_t Nunchuk::decodeAccelerationZ() const
    {
        return static_cast<int16_t>((m_raw[4] << 2) | (static_cast<uint16_t>((m_raw[5]>> 6) & Bitmask::ACC_Z_BIT_0_1) )
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
      if (!isConnected())
      {
        m_state = State::NO_DATA_AVAILABLE;
        serialerror("Es liegen keine neuen Sensorendaten vor.", m_state);
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
      delayMicroseconds(500);
    }

    void Nunchuk::disable() const
    {
      if (m_pinLevelshifter == 0xFF)
        return;
        
      serialverbose("Pegelwandler deaktiviert.");
      digitalWrite(m_pinLevelshifter, LOW);
      delayMicroseconds(500);
    }

  const Button::State Nunchuk::ButtonC::getState() const
	{
		return m_device->decodeButtonC() ? State::PRESSED : State::RELEASED;
	}

	const Button::State Nunchuk::ButtonZ::getState() const
	{
		return m_device->decodeButtonZ() ? State::PRESSED : State::RELEASED;
	}
}