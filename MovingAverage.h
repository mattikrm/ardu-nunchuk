#ifndef MOVING_AVERAGE_H
#define MOVING_AVERAGE_H

#include <Arduino.h>

namespace communication
{

template<
	class T, // Inhaltsdatentyp
	size_t Length // Anzahl der Elemente im Speicher
>
class RingBuffer
{
	public: // public typedefs
		using value_type = T;
		using size_type = size_t;

		using reference = T&;
		using const_reference = const T&;

		using pointer = T*;
		using const_pointer = const T*;
	
	public: // public static Member

		static constexpr const size_t npos = static_cast<size_t>(-1);

	public: // public Methoden
		RingBuffer()
		: m_data{0},
		  m_index{0}
		{

		}

		const T &read();
		void write(const T& value)
		{
			front() = value;
			m_index = next();
		}

		T &front()
		{
			return m_data[m_index];
		}
		const T &front() const
		{
			return m_data[m_index];
		}
		T &back()
		{
			return m_data[(m_index - 1) % Length];
		}
		const T &back() const
		{
			return m_data[(m_index - 1) % Length];
		}

	private: // private Methoden
		constexpr size_t next() const
		{
			return (m_index + 1) % Length;
		}

	private: // private Member
		size_t m_index; // index des aktuellen Elements
		T m_data[Length]; // zugrundeliegender Speicher
};

template<
	size_t Width
>
class MovingAverage
{
	public: // public Methoden
		MovingAverage()
		: m_data{},
		  m_cumsum{0}
		{
		}

		void shift(uint8_t next)
		{
			m_data.write(next);
			m_cumsum += m_data.front() - m_data.back();
		}

		const double arithmeticMean() const
		{
			return static_cast<double>(m_cumsum) / Width;
		}
		const int32_t cumulativeSum() const
		{
			return m_cumsum;
		}

	private: // private Member
		RingBuffer<uint8_t, Width> m_data;
		int32_t m_cumsum;
};

} // namespace communication

#endif // !MOVING_AVERAGE_H