#include "MovingAverage.h"

namespace communication
{
	
template<
	class T,
	size_t Length
>
RingBuffer<T, Length>::RingBuffer()
 : m_data{0},
 m_index{0}
{

}

template<
	class T,
	size_t Length
>
void RingBuffer<T, Length>::write(const T &value)
{
	front() = value;
	m_index = next();
}

template <
	class T,
	size_t Length
>
const T &RingBuffer<T, Length>::back() const
{
	return m_data[next()];
}

template <
	class T,
	size_t Length
>
T &RingBuffer<T, Length>::front()
{
	return m_data[m_index];
}

template <
	class T,
	size_t Length>
const T &RingBuffer<T, Length>::front() const
{
	return m_data[m_index];
}

template <
	class T,
	size_t Length
>
T &RingBuffer<T, Length>::back()
{
	return m_data[next()];
}

template<
	class T,
	size_t Length
>
constexpr size_t RingBuffer<T, Length>::next() const
{
	return (m_index + 1) % Length;
}

template <
	size_t Width
>
MovingAverage<Width>::MovingAverage()
	: m_data{},
	m_cumsum{0}
{
}

template <
	size_t Width
>
void MovingAverage<Width>::shift(uint8_t next)
{
	m_data.write(next);
	m_cumsum += m_data.front() - m_data.back();
}

template <
	size_t Width
>
const double MovingAverage<Width>::arithmeticMean() const
{
	return static_cast<double>(m_cumsum) / Width;
}

template <
	size_t Width
>
const int32_t MovingAverage<Width>::cumulativeSum() const
{
	return m_cumsum;
}

} // namespace communication
