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
     *   @file   Nunchuk.h
     * 
     *   @brief  Klassendefinition für Wii Nunchuk und Kommunikation über I2C.
     * 
     *   @author Mattheo Krümmel
     *
     *   @date   22-07-2020
     */

#ifndef NUNCHUK_H
#define NUNCHUK_H

#include <Arduino.h>

namespace communication
{
    /**************************
     * Definitionen von Konstanten *
     **************************/

    // Exitcodes der Methoden
    enum class ExitCode : uint16_t
        {
            // kein Fehler
            NO_ERROR = 0,

            // allgemeiner Fehler
            ERROR_OCCURED,

            // ungültiger Wert
            BAD_VALUE,

            // keine Verbindung zum Nunchuk möglich
            NOT_CONNECTED,

            // Datentyp nicht initialisiert
            NOT_INITIALIZED,

            // keine Daten vorliegend
        NO_DATA_AVAILABLE,

        // Timeout wurde überschritten
        TIMEOUT
        };

    // I2C Bus Taktfrequenzen
    enum class ClockMode : uint32_t
    {
        // I2C-Frequenz im Standardmode
        I2C_CLOCK_STANDARD_100_kHz = 100000,

        // I2C-Frequenz im Fastmode
        I2C_CLOCK_FAST_400_kHz = 400000
    };

    namespace WireReturnCode
    {
        using WireReturnConstant = const uint16_t;

        constexpr WireReturnConstant SUCCESS{0};
        constexpr WireReturnConstant DATA_TOO_LONG{1};
        constexpr WireReturnConstant NACK_ON_ADDR{2};
        constexpr WireReturnConstant NACK_ON_DATA{3};
        constexpr WireReturnConstant OTHER{4};
        constexpr WireReturnConstant TIMEOUT{5};
    };

    // Mittenwert des Joysticks in angegebener Richtung
    namespace Joystick
        {
        using JoystickConstant = const int8_t;

            // Mittenwert des Joysticks (links <-> rechts)
        constexpr JoystickConstant X_NULL{0x7D};

            // Mittenwert des Joysticks (oben <-> unten)
        constexpr JoystickConstant Y_NULL{0x7E};
    };

    // Neutralwert der Gyrosensoren in angebener Richtung
    namespace Acceleration
    {
        using AccelerationConstant = const int16_t;

            // Neutralwert des Gyrosensors (links <-> rechts)
        constexpr AccelerationConstant X_NULL{512};

            // Neutralwert des Gyrosensors (vor <-> zurück)
        constexpr AccelerationConstant Y_NULL{512};

            // Neutralwert des Gyrosensors (oben <-> unten)
        constexpr AccelerationConstant Z_NULL{512};
    };

    // Allgemeine Standardwerte
    namespace Control
    {
        using ControlConstant = const uint8_t;

            // Länge des Arrays für Sensorendaten
        constexpr ControlConstant LEN_RAW_DATA{6};

            // Länge des Arrays für Kalibrierungsdaten
        constexpr ControlConstant LEN_CAL_DATA{16};

            // I2C-Adresse des Nunchuks
        constexpr ControlConstant ADDR_NUNCHUK{0x52};

            // Registeradresse der Sensorendaten
        constexpr ControlConstant REG_RAW_DATA{0x00};

            // Registeradresse der Kalibrierungsdaten
        constexpr ControlConstant REG_CAL_DATA{0x20};

            // Registeradresse der Nunchuk-ID
        constexpr ControlConstant REG_ID{0xFA};

            // Registeradresse zum Überprüfen des Verschlüsselungsstatus
        constexpr ControlConstant REG_IS_ENCR{0};
    };

    
    // Bitmasken der zusammengesetzten Register, die der Nunchuck ausgibt
    namespace Bitmask
    {
        using BitmaskConstant = const uint8_t;

            // Bit 0 des zusammengesetzten Registers
            // entspricht Gedrücktstatus des Buttons Z [1 = pressed/0 = released]
        constexpr BitmaskConstant BUTTON_Z_STATE{0x01};

            // Bit 1 des zusammengesetzten Regosters
            // entspricht Gedrücktstatus des Buttons C [1 = pressed/0 = released]]
        constexpr BitmaskConstant BUTTON_C_STATE{0x02};

            // Bits [3:2] des zusammengesetzten Registers;
            // entsprechen Bits [1:0] des Beschleunigungswertes in X-Richtung (rechts - links)
        constexpr BitmaskConstant ACC_X_BIT_0_1{0x0C};

            // Bits [5:4] des zusammengesetzten Registers
            // entsprechen Bits [1:0] des Beschleunigungswertes in Y-Richtung (vorne - hinten)
        constexpr BitmaskConstant ACC_Y_BIT_0_1{0x30};

            // Bits [7:6] des zusammengesetzten Registers
            // entsprechen Bits [1:0] des Beschleunigungswertes in Z-Richtung (oben - unten)
        constexpr BitmaskConstant ACC_Z_BIT_0_1{0xC0};
    };

    /************************************
     * Definitionen statischer Methoden *
     ************************************/

 //   int8_t operator&(const int8_t left, const Bitmasks right);
 //
 //   int16_t operator-(const int8_t left, const Acceleration right);

  /**
   * @brief         Sendet eine formatierte Ausgabe an den Seriellen Monitor
   * 
   * @mode          Art der Ausgabe, siehe debugmode
   * @annotation    Inhalt der Meldung
   */
  void serialwrite(const char *mode, const char *annotation);

  /**
   * @brief         Sendet eine formatierte erweitererte Information an den Seriellen Monitor
   * 
   * @annotation    Inhalt der Meldung
   */
  void serialverbose(const char *annotation);

  /**
   * @brief         Sendet eine formatierte informelle Benachrichtigung an den Seriellen Monitor
   * 
   * @annotation    Inhalt der Meldung
   */
  void serialinfo(const char *annotation);

  /**
   * @brief         Sendet eine formatierte Fehlermeldung mit entsprechendem Fehlercode
   *                an den Seriellen Monitor
   * 
   * @annotation    Inhalt der Meldung
   * @code          Code des auslösenden Fehlers
   */
  void serialerror(const char *annotation, const uint16_t code);

    /*************************************
     * Definitionen statischer Variablen *
     *************************************/

  // Ausgabemodus der Bibliothek
  // >= 0   Fehlermedlungen werden ausgegeben
  // > 0    informelle Benachrichtungen werden ausgegeben
  // > 1    erweiterte Informationen (v. a. für Debugging) werden ausgegeben
  constexpr const int8_t debugmode{2};

    /******************************
     * Definition der Hauptklasse *
     ******************************/

    /**
     * @brief   Klasse Nunchuk kommuniziert mit einem Nunchuk und verarbeitet und speichert die
     *          empfangenen Sensorendaten.
     */
    class Nunchuk
    {
    public:
        // Konstruktoren

        /**
         * @brief   Konstruktor der Klasse Nunchuk.
         *          Initialisiert die Memeber mit den Neutralwerten.
         */
        Nunchuk();

        /**
         * @brief   Konstruktor der Klasse Nunchuk.
         *          Initialisiert die Memeber mit den Neutralwerten und definiert die Adresse des Nunchuks
         *
         * @addr    I2C-Adresse des korrespondierenden Nunchuks
         */
        explicit Nunchuk(uint8_t addr);

        // Getter und Setter

        /*
         * @brief   Getter für m_addr
         */
        const uint8_t getAddress();

        /**
         * @brief   Getter für m_isConnected
         */
        bool isConnected();

        /**
         * @brief   Setter für m_clock
         */
        ExitCode getLastError();

        // Andere Methoden
        
        /*
         * @brief   Initialisiert die erforderlichen Bibliotheken und Peripherie.
         *          Sollte vor Nunchuk::begin() aufgerufen werden.
         */
        void libinit();

        /**
         * @brief   Initialisierungssequenz für den Nunchuk, um mit ihm kommunizieren zu können.
         *
         * @return  Exitcode der Methode
         */
        ExitCode begin();

        /**
         * @brief   Liest die aktuellen Sensorwerte vom Nunchuk über den I2C-Bus.
         *
         * @return  Exitcode der Mehtode
         */
        ExitCode read();

        /**
         * @brief   Extrahiert den Gedrücktstatus des Buttons Z aus dem zusammengesetzten Register.
         *
         * @return  Gedrücktstatus des Buttons Z [false = released/true = pressed]
         */
        const bool decodeButtonZ() const;

        /**
         * @brief   Extrahiert den Gedrücktstatus des Buttons C aus dem zusammengesetzten Register.
         *
         * @return  Gedrücktstatus des Buttons C [false = released/true = pressed]
         */
        const bool decodeButtonC() const;

        /**
         * @brief   Extrahiert die Bits [0:1] des Beschleunigungswerts in X-Richtung aus dem
         *          zusammengesetzten Register, setzt sie mit den Bits [2:1] zusammen.
         * 
         * @return  Beschleunigungswert in X-Richtung (-512;512]
         */
        const int16_t decodeAccelerationX() const;

        /**
         * @brief   Extrahiert die Bits [0:1] des Beschleunigungswerts in Y-Richtung aus dem
         *          zusammengesetzten Register, setzt sie mit den Bits [2:10] zusammen.
         * 
         * @return  Beschleunigungswert in Y-Richtung (-512;512]
         */
        const int16_t decodeAccelerationY() const;

        /**
         * @brief   Extrahiert die Bits [0:1] des Beschleunigungswerts in Z-Richtung aus dem
         *          zusammengesetzten Register, setzt sie mit den Bits [2:10] zusammen.
         * 
         * @return  Beschleunigungswert in Z-Richtung (-512;512]
         */
        const int16_t decodeAccelerationZ() const;

        /**
         * @brief   Subtrahiert den Mittenwert vom Registerwert für die Position in X-Richtung.
         *          
         * @return  Position relativ zur Mitte in X-Richtung (-125;130]
         */
        const int8_t decodeJoystickX() const;

        /**
         * @brief   Subtrahiert den Mittenwert vom Registerwert für die Position in Y-Richtung.
         *          
         * @return  Position relativ zur Mitte in Y-Richtung (-126;129]
         */
        const int8_t decodeJoystickY() const;

        void print();

    private:
        // Initialisierungsstatus der Wire-Bibliothek [true = initialisiert]
        static bool m_isWireInit;

        // Initialisierungsstatus der Serial-Bibliothek [true = initialisiert]
        static bool m_isSerialInit;

        // Rohdaten vom Nunchuk
        uint8_t m_raw[Control::LEN_RAW_DATA];

        // Adresse des korrespondierenden Nunchuks
        uint8_t m_addr;

        // Taktfrequenz der I2C-Clock
        uint32_t m_clock;

        // Code des letzten Aufgetretenen Fehlers
        ExitCode m_lastError;

        // Verbundenheitsstatus des Geräts
        bool m_isConnected;
    };
}
#endif // !NUNCHUK_H
