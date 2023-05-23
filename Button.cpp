#include "Button.h"

#include "Nunchuk.h"

#include <Arduino.h>

namespace communication
{
	Button::Button(const Nunchuk *dev, const unsigned long duration)
		: m_state{State::RELEASED},
		m_duration{duration},
		m_lastChange{millis()},
		m_device {m_device}
	{
	}

	const bool Button::isPressed()
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
				m_state = State::RELEASED_TIMOUT;
				m_lastChange = millis();
			}
			break;

		case State::RELEASED_TIMOUT:
			if (currentState == State::RELEASED)
			{
				m_state = State::RELEASED;
			}
			else if ((millis() - m_lastChange) >= m_duration)
			{
				m_state = State::PRESSED;
			}
			break;
		
		default:
			break;
		}
	}

	const bool Button::isReleased()
	{
		return !isPressed();
	}
} // namespace communication
