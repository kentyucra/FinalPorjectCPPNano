#include <mutex>
#include <deque>
#include <condition_variable>

template <class T>
class MessageQueue
{
public:
    T receive()
    {
        std::unique_lock<std::mutex> uLock(_mutex);
        _conditionVar.wait(uLock, [this]
                           { return !_queue.empty(); });

        T msg = std::move(_queue.front());
        _queue.pop_front();
        return msg;
    }
    void send(T &&msg)
    {
        std::unique_lock<std::mutex> uLock(_mutex);
        _queue.emplace_back(std::move(msg));
        _conditionVar.notify_one();
    }

private:
    std::deque<T> _queue;
    std::condition_variable _conditionVar;
    std::mutex _mutex;
};