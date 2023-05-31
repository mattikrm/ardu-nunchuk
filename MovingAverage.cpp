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
	typename Integral,
	size_t Width
>
MovingAverage<Integral, Width>::MovingAverage()
	: m_data{},
	m_cumsum{0}
{
}

template <
	typename Integral,
	size_t Width
>
void MovingAverage<Integral, Width>::shift(Integral next)
{
	m_data.write(next);
	m_cumsum += m_data.front() - m_data.back();
}

template <
	typename Integral,
	size_t Width
>
const double MovingAverage<Integral, Width>::arithmeticMean() const
{
	return static_cast<double>(m_cumsum) / Width;
}

template <
	typename Integral,
	size_t Width
>
const int32_t MovingAverage<Integral, Width>::cumulativeSum() const
{
	return m_cumsum;
}

} // namespace communication
