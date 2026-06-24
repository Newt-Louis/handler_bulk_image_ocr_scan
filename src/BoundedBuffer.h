#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

template<typename T>
class BoundedBuffer
{
public:
    explicit BoundedBuffer(size_t maxSize)
        : m_maxSize(maxSize)
    {
    }

    void push(T item)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_notFull.wait(lock, [this] { return m_queue.size() < m_maxSize || m_closed; });
        if (m_closed) {
            return;
        }
        m_queue.push(std::move(item));
        m_notEmpty.notify_one();
    }

    std::optional<T> pop()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_notEmpty.wait(lock, [this] { return !m_queue.empty() || m_closed; });
        if (m_queue.empty()) {
            return std::nullopt;
        }
        T item = std::move(m_queue.front());
        m_queue.pop();
        m_notFull.notify_one();
        return item;
    }

    void close()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_closed = true;
        m_notEmpty.notify_all();
        m_notFull.notify_all();
    }

    bool isClosed() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_closed;
    }

    size_t size() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.size();
    }

private:
    mutable std::mutex m_mutex;
    std::condition_variable m_notEmpty;
    std::condition_variable m_notFull;
    std::queue<T> m_queue;
    size_t m_maxSize;
    bool m_closed = false;
};
