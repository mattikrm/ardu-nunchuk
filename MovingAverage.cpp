#include "MovingAverage.h"

namespace communication
{
	
template<
	class T,
	size_t Length
>
RingBuffer<T, Length>::RingBuffer()
 : m_data{0},
 m_front{m_data},
 m_back{m_data}
{

}

template<
	class T,
	size_t Length
>
void RingBuffer<T, Length>::write(const T &value)
{
	m_back = next(m_back);
	back() = value;
}

template<
	class T,
	size_t Length
>
const T &RingBuffer<T, Length>::read()
{
	const auto result = front();
	m_front = next(m_front);
	return result;
}

template<
	class T,
	size_t Length
>
constexpr size_t RingBuffer<T, Length>::size() const
{
	// Startzustand
	if (m_front == m_back)
		return 0;

	else if (m_front < m_back)
		return next(m_back) - m_front;
	else if (m_back < m_front)
		return Length - (m_front - next(m_back));
}

template<
	class T,
	size_t Length
>
constexpr const bool RingBuffer<T, Length>::empty() const
{
	// Startzustand
	return next(m_front) == m_back;
}

template<
	class T,
	size_t Length
>
constexpr T *RingBuffer<T, Length>::next(const T *iter) const
{
	if (!checkBounds(iter))
		return nullptr;

	return iter < cend() ? iter + 1 : cbegin();
}

template<
	class T,
	size_t Length
>
constexpr T *RingBuffer<T, Length>::prev(const T *iter) const
{
	if (!checkBounds(iter))
		return nullptr;

	return iter > cbegin() ? iter - 1 : cend();
}

template<
	class T,
	size_t Length
>
constexpr const bool RingBuffer<T, Length>::checkBounds(const T *iter) const
{
	return (iter >= cbegin() && iter < cend());
}

template<
	class T,
	size_t Length
>
constexpr T *RingBuffer<T, Length>::begin()
{
	return m_data;
}

template<
	class T,
	size_t Length
>
constexpr T *RingBuffer<T, Length>::end()
{
	return m_data + Length;
}

template<
	class T,
	size_t Length
>
constexpr const T *RingBuffer<T, Length>::cbegin() const noexcept
{
	return m_data;
}

template<
	class T,
	size_t Length
>
constexpr const T *RingBuffer<T, Length>::cend() const noexcept
{
	return m_data + Length;
}

template<
	class T,
	size_t Length
>
T &RingBuffer<T, Length>::front()
{
	return *m_front;
}

template<
	class T,
	size_t Length
>
const T &RingBuffer<T, Length>::front() const
{
	return const_cast<const T &>(*m_front);
}

template<
	class T,
	size_t Length
>
T &RingBuffer<T, Length>::back()
{
	return const_cast<const T &>(*m_back);
}

template<
	class T,
	size_t Length
>
const T &RingBuffer<T, Length>::back() const
{
	return *m_back;
}

} // namespace communication
