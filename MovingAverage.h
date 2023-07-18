#ifndef MOVING_AVERAGE_H
#define MOVING_AVERAGE_H

#include <Arduino.h>

namespace communication
{

/**
 * @brief Klassen-Template eines Ringpuffers.
 * 
 * @tparam T Datentyp der Elemente
 * @tparam Length Anzahl der Elemente
 */
template<
	class T,
	size_t Length
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

		/**
		 * @brief Schreibt in den Ringpuffer und inkrementiert die Position
		 * 
		 * @param value Referenz auf zu schreibenden Wert
		 */
		void write(const T& value)
		{
			front() = value;
			m_index = next();
		}

		/**
		 * @brief Gibt Referenz auf erstes (aktuelles) Elemment im Speicher zurück
		 * 
		 * @return T& Referenz auf erstes Element
		 */
		T &front()
		{
			return m_data[m_index];
		}
		const T &front() const
		{
			return m_data[m_index];
		}

		/**
		 * @brief Gibt Referenz auf letztes (nächsten) Element im Speicher zurück
		 * 
		 * @return T& Referenz auf letztes Element im Speicher
		 */
		T &back()
		{
			return m_data[(m_index - 1) % Length];
		}
		const T &back() const
		{
			return m_data[(m_index - 1) % Length];
		}

	private: // private Methoden
		/**
		 * @brief Gibt den nächsten Index im Ringpuffer zurück
		 * 
		 * @return constexpr size_t nächster Index in [0;Length)
		 */
		constexpr size_t next() const
		{
			return (m_index + 1) % Length;
		}

	private: // private Member
		size_t m_index; // Index des ersten Elements
		T m_data[Length]; // zugrundeliegender Speicher
};

/**
 * @brief Klassen-Template zur Ermittlung des ungewichteten,
 * gleitenden Mittelwertes einer (Ganz-)Zahlenreihe
 * 
 * @tparam T (Ganzzahl-)Datentyp der Elemente
 * @tparam Width Anzahl der Elemente
 */
template<
	class T,
	size_t Width
>
class MovingAverage
{
	public: // public Methoden
		/**
		 * @brief Kontruiert eine neues Objekt der Klasse Moving Average
		 */
		MovingAverage()
		: m_data{},
		  m_cumsum{0}
		{
		}

		/**
		 * @brief Fügt neues Element hinzu, löscht ggf. ältestes Element.
		 * Aktualisiert die kumulative Summe der Elemente.
		 * 
		 * @param next neu hinzuzufügendes Element
		 */
		void shift(T next)
		{
			m_data.write(next);
			m_cumsum += m_data.front() - m_data.back();
		}

		/**
		 * @brief Gibt den ungewichteten arithm. Mittelwert zurück
		 * 
		 * @return const double arithm. Mittelwert der Elemente
		 */
		const double arithmeticMean() const
		{
			return static_cast<double>(m_cumsum) / Width;
		}

		/**
		 * @brief Gibt die kumulative Summe der Elemente aus
		 * 
		 * @return const int32_t kumulative Summe der Elemente
		 */
		const int32_t cumulativeSum() const
		{
			return m_cumsum;
		}

	private: // private Member
		RingBuffer<T, Width> m_data;
		int32_t m_cumsum;
};

} // namespace communication

#endif // !MOVING_AVERAGE_H