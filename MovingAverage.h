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

		constexpr size_t size() const;
		constexpr const bool empty() const;

		T &front();
		const T &front() const;

		T &back();
		const T &back() const;

		constexpr T *next(const T *iter) const;
		constexpr T *prev(const T *iter) const;

		constexpr const bool checkBounds(const T *iter) const;

	private: // private Methoden
		constexpr T *begin();
		constexpr const T *cbegin() const noexcept;

		constexpr T *end();
		constexpr const T *cend() const noexcept;

	private: // private Member
		T *m_front; // nächstes zu lesendes Element
		T *m_back; // nächstes zu beschreibendes Element
		const T m_data[Length]; // zugrundeliegender Speicher
};

template<
	typename Integral, // Inhaltstyp des zugrundeliegenden Speichers
	size_t Width // Anzahl der Elemente über die der Mittelwert gebildet werden soll
>
class MovingAverage
{
	public: // public Methoden
		MovingAverage();

		void shift(Integral next);

		const double arithmeticMean() const;
		const double median() const;

	private: // private Member
		RingBuffer<Integral, Width> m_data;
};

} // namespace communication

#endif // !MOVING_AVERAGE_H