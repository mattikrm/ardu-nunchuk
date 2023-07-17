    /**
     * Copyright (c) 2023, Mattheo Krümmel
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
     *   @file   Button.h
     * 
     *   @brief  Klassendefinition Buttons des Wii Nuchuks, die erst nach einer gewissen Zeitspanne
	 * 			 auslösen sollen
     * 
     *   @author Mattheo Krümmel
     *
     *   @date   23-05-2023
     */

#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

namespace communication
{
class Nunchuk;

class Button
{

public: // Enumerationen
	/**
	 * @brief Zustände des Automaten
	 */
	enum class State
	{
		RELEASED, // nicht gedrückt
		PRESSED_TIMEOUT, // wird gedrückt, wartend auf Timeout
		PRESSED, // gedrückt
		RELEASED_TIMEOUT // wurde losgelassen, wartend auf das Timeout
	};

public: // public Methoden
	/**
	 * @brief Konstruktor der Klasse Button.
	 * 		  Setzt den Nunchuk, dessen Knopf geprüft werden soll, sowie die
	 * 		  Zeit nach der er seinen neuen Zustand annehmen soll.
	 * 
	 * @param duration Zeitspanne, nachder die Zustandsänderung angenommen wird
	 */
	Button(const unsigned long duration = 30);

	/**
	 * @brief Gibt den Gedrücktzustand zurück
	 * 
	 * @return true Button wird gedrückt
	 * @return false Button wird nicht gedrückt
	 */
	const bool isPressed() const;

	/**
	 * @brief Gibt den Gedrücktzustand zurück
	 * 
	 * @return true Button wird nicht gedrückt
	 * @return false Button wird gedrückt
	 */
	const bool isReleased() const;

	/**
	 * @brief Registriert einen Callback, der beim Drücken des Buttons aufgerufen wird.
	 *
	 * @param pressedCallback Zeiger auf die Callback-Funktion, nullptr deregistriert den Callback
	*/
	void onPressed(void const (*pressedCallback)(void))
	{
		m_pressedCallback = pressedCallback;
	}

	/**
	 * @brief Registriert einen Callback, der beim Loslassen des Buttons aufgerufen wird.
	 *
	 * @param releasedCallback Zeiger auf die Callback-Funktion, nullptr deregistriert den Callback
	*/
	void onReleased(void const (*releasedCallback)(void))
	{
		m_releasedCallback = releasedCallback;
	}

	/**
	 * @brief Bestimmt den Zutand des Buttons
	 * 
	 */
	void exec();

private: // private-Methoden
	/**
	 * @brief Gibt den aktuellen Zustand des Buttons direkt von der Hardware zurück
	 * 
	 * @return State::PRESSED Button wird gedrückt
	 * @return State::RELEASED Button wird nicht gedrückt
	 */
	virtual const State getState() const = 0;

private: // private Member
	const unsigned long m_duration; // Zeitspanne, die der Button mindestens gedrück sein muss
	unsigned long m_lastChange; // Zeitpunkt der letzten Änderung des Gedrücktzustands
	State m_state; // Zustand des Automaten
	void (*m_pressedCallback)(void);
	void (*m_releasedCallback)(void);

};
	
} // namespace communication

#endif // !BUTTON_H