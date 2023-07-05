#include "Button.h"

#include "Nunchuk.h"

#include <Arduino.h>

namespace communication
{
	Button::Button(const unsigned long duration)
		: m_state{State::RELEASED},
		m_duration{duration},
		m_lastChange{millis()}
	{
	}

	void Button::exec()
	{
		const State currentState = getState();

		switch (m_state)
		{
		case State::RELEASED:
			if (currentState == State::PRESSED)
			{
				m_state = State::PRESSED_TIMEOUT;
				m_lastChange = millis();
			}
			break;
		
		case State::PRESSED_TIMEOUT:
			if (currentState == State::RELEASED)
			{
				m_state = State::RELEASED;
			}
			else if ((millis() - m_lastChange) >= m_duration)
			{
				m_state = State::PRESSED;
			}
			break;
		
		case State::PRESSED:
			if (currentState == State::RELEASED)
			{
				m_state = State::RELEASED_TIMEOUT;
				m_lastChange = millis();
			}
			break;

		case State::RELEASED_TIMEOUT:
			if (currentState == State::PRESSED)
			{
				m_state = State::PRESSED;
			}
			else if ((millis() - m_lastChange) >= m_duration)
			{
				m_state = State::RELEASED;
			}
			break;
		
		default:
			break;
		}
	}

	const bool Button::isPressed() const
	{
		return (m_state == State::PRESSED || m_state == State::RELEASED_TIMEOUT) ? true : false;
	}

	const bool Button::isReleased() const
	{
		return (m_state == State::RELEASED || m_state == State::PRESSED_TIMEOUT) ? true : false;
	}
} // namespace communication
