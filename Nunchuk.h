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

#include "Button.h"

namespace communication
{
    /**************************
     * Definitionen von Konstanten *
     **************************/

    // Zustand des Automaten
    enum class State
    {
        // Startzustand
        BEGIN = 0,

        // Nunchuk über I2C verbunden (Endzustand)
        CONNECTED,

        /******************
         * Fehlerzustände *
         ******************/

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
         *          Initialisiert den I2C-Bus mit der übergebenen Taktfrequenz (Standard: Fast-Mode).
         * 
         * @param buttonTimeout Dauer bis der Zustand der Buttons angepasst wird in ms
         * @param cycletime Zykluszeit nach der wieder Daten angefordert werden in ms
         * @param mode Taktfrequenz der I2C-Schnittstelle
         */
        Nunchuk(const unsigned long buttonTimeout,
            const unsigned long cycletime = 30,
            const ClockMode mode = ClockMode::I2C_CLOCK_FAST_400_kHz);

        /**
         * @brief   Konstruktor der Klasse Nunchuk.
         *          Initialisiert den I2C-Bus mit der übergebenen Taktfrequenz.
         *          Aktiviert außerdem den Enablepin des Pegelwandlers.
         * 
         * @param lvlshft Enable-Pin des Pegelwandlers für den I2C-Bus
         * @param buttonTimeout Dauer bis der Zustand der Buttons angepasst wird in ms
         * @param cycletime Zykluszeit nach der wieder Daten angefordert werden in ms
         * @param mode Taktfrequenz der I2C-Schnittstelle
         */
        Nunchuk(const uint8_t lvlshft,
            const unsigned long buttonTimeout,
            const unsigned long cycletime = 30,
            const ClockMode mode = ClockMode::I2C_CLOCK_FAST_400_kHz);

        /**
         * @brief   Konstruktor der Klasse Nunchuk.
         *          Initialisiert den I2C-Bus mit der übergebenen Taktfrequenz.
         *          Aktiviert außerdem den Enablepin des Pegelwandlers.
         * 
         * @param lvlshft Enable-Pin des Pegelwandlers für den I2C-Bus
         * @param cTimeout Dauer bis der Zustand des C-Buttons angepasst wird in ms
         * @param zTimeout Dauer bis der Zustand des Z-Buttons angepasst wird in ms
         * @param cycletime Zykluszeit nach der wieder Daten angefordert werden in ms
         * @param mode Taktfrequenz der I2C-Schnittstelle
         */
        Nunchuk(const uint8_t lvlshft,
            const unsigned long cTimeout, const unsigned long zTimeout,
            const unsigned long cycletime = 30,
            const ClockMode mode = ClockMode::I2C_CLOCK_FAST_400_kHz);

        /**
         * @brief   Destruktor der Klasse Nunchuk.
         *          Gibt den I2C-Bus wieder frei.
         */
        ~Nunchuk();

        // Getter und Setter

        /**
         * @brief   Liest den Verbindungsstatus des Nuchuks aus
         * 
         * @return  boolean Verbindungsstatus [true: verbunden | false: nicht verbunden] 
         */
        const bool isConnected() const;

        /**
         * @brief   Gibt den aktuellen Zustand des Automaten zurück
         * 
         * @return enum class State des Automaten
         */
        const State getState() const;

        // Andere Methoden

        /**
         * @brief   Initialisierungssequenz für den Nunchuk, um mit ihm kommunizieren zu können.
         *          Deaktiviert die Verschlüsselung.
         *
         * @return  enum class Exitcode der Methode
         */
        State begin();

        /**
         * @brief   Liest die aktuellen Sensorwerte vom Nunchuk über den I2C-Bus.
         *
         * @return  enum class Exitcode der Methode
         */
        State read();
        
        /**
         * @brief Bestimmt den Gedrücktzustand des Buttons C
         * 
         * @return  boolean Gedrücktstatus des Buttons C [true: gedrückt | false: losgelassen]
         */
        const bool pressedC() const;

        /**
         * @brief Bestimmt den Gedrücktzustand des Buttons Z
         * 
         * @return  boolean Gedrücktstatus des Buttons Z [true: gedrückt | false: losgelassen]
         */
        const bool pressedZ() const;

        /**
        * @brief Registriert einen Callback, der beim Drücken des Buttons C aufgerufen wird.
        *
        * @param pressedCallback Zeiger auf die Callback-Funktion, nullptr deregistriert den Callback
        */
        void onPressedC(void const (*pressedCallback)(void))
        {
            m_buttonC.onPressed(pressedCallback);
        }

        /**
        * @brief Registriert einen Callback, der beim Drücken des Buttons Z aufgerufen wird.
        *
        * @param pressedCallback Zeiger auf die Callback-Funktion, nullptr deregistriert den Callback
        */
        void onPressedZ(void const (*pressedCallback)(void))
        {
            m_buttonZ.onPressed(pressedCallback);
        }

        /**
         * @brief   Extrahiert den Gedrücktstatus des Buttons Z aus dem zusammengesetzten Register.
         *
         * @return  boolean Gedrücktstatus des Buttons Z [true: gedrückt | false: losgelassen]
         */
        const bool decodeButtonZ() const;

        /**
         * @brief   Extrahiert den Gedrücktstatus des Buttons C aus dem zusammengesetzten Register.
         *
         * @return  boolean Gedrücktstatus des Buttons C [true: gedrückt | false: losgelassen]
         */
        const bool decodeButtonC() const;

        /**
         * @brief   Extrahiert die Bits [0:1] des Beschleunigungswerts in X-Richtung aus dem
         *          zusammengesetzten Register, setzt sie mit den Bits [2:1] zusammen.
         * 
         * @return  int16_t Beschleunigungswert in X-Richtung (-512;512]
         */
        const int16_t decodeAccelerationX() const;

        /**
         * @brief   Extrahiert die Bits [0:1] des Beschleunigungswerts in Y-Richtung aus dem
         *          zusammengesetzten Register, setzt sie mit den Bits [2:10] zusammen.
         * 
         * @return  int16_t Beschleunigungswert in Y-Richtung (-512;512]
         */
        const int16_t decodeAccelerationY() const;

        /**
         * @brief   Extrahiert die Bits [0:1] des Beschleunigungswerts in Z-Richtung aus dem
         *          zusammengesetzten Register, setzt sie mit den Bits [2:10] zusammen.
         * 
         * @return  int16_t Beschleunigungswert in Z-Richtung (-512;512]
         */
        const int16_t decodeAccelerationZ() const;

        /**
         * @brief   Subtrahiert den Mittenwert vom Registerwert für die Position in X-Richtung.
         *          
         * @return  int8_t Joystickauslenkung relativ zur Mitte in X-Richtung (-125;130]
         */
        const int16_t decodeJoystickX() const;

        /**
         * @brief   Subtrahiert den Mittenwert vom Registerwert für die Position in Y-Richtung.
         *          
         * @return  int8_t Joystickauslenkung relativ zur Mitte in Y-Richtung (-126;129]
         */
        const int16_t decodeJoystickY() const;

        void print();

    private:
        /**
         * @brief   Setzt den enable-Pin des Levelshifters auf HIGH
         * 
         * @return  none
         */
        void enable() const;
        
        /**
         * @brief   Setzt den enable-Pin des Levelshifters auf LOW
         * 
         * @return  none
         */
        void disable() const;

        class ButtonC : public Button
        {
        	public:
                using Button::Button;
                ButtonC(const Nunchuk *dev, const unsigned long duration);

        		const State getState() const override;

            private:
                const Nunchuk *m_device; // Gerät dessen Button überwacht werden soll 
        };


        class ButtonZ : public Button
        {
        	public:
                using Button::Button;
                ButtonZ(const Nunchuk *dev, const unsigned long duration);

        		const State getState() const override;

            private:
                const Nunchuk *m_device; // Gerät dessen Button überwacht werden soll 
        };
        
        ButtonC m_buttonC; // Repräsentation des C-Buttons
        ButtonZ m_buttonZ; // Repräsentation des Z-Buttons

        // Enable Pin des Pegelwandlers für den I2C-Bus
        const uint8_t m_pinLevelshifter;

        // Rohdaten vom Nunchuk
        uint8_t m_raw[Control::LEN_RAW_DATA];

        // aktueller Zustand des Automaten
        State m_state;

        // Zeitspanne nach der erneut Daten vom Nunchuk angefordert werden
        const unsigned long m_cycletime;

        // Zeitpunkt zu dem zuletzt neue Daten angefordert wurden
        unsigned long m_lastFetch;
    };
}
#endif // !NUNCHUK_H
