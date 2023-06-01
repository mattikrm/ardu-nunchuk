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
		RingBuffer();

		const T &read();
		void write(const T& value);

		T &front();
		const T &front() const;
		T &back();
		const T &back() const;

	private: // private Methoden
		constexpr size_t next() const;

	private: // private Member
		size_t m_index; // index des aktuellen Elements
		const T m_data[Length]; // zugrundeliegender Speicher
};

template<
	size_t Width
>
class MovingAverage
{
	public: // public Methoden
		MovingAverage();

		void shift(uint8_t next);

		const double arithmeticMean() const;
		const int32_t cumulativeSum() const;

	private: // private Member
		RingBuffer<uint8_t, Width> m_data;
		int32_t m_cumsum;
};

} // namespace communication

#endif // !MOVING_AVERAGE_H