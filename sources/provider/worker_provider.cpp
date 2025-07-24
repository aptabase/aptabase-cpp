#include "aptabase/provider/worker_provider.hpp"

Aptabase::WorkerProvider::WorkerProvider(std::unique_ptr<HttpClient> &&client,
                                               const std::string &app_key,
                                               const std::string &api_url)
    : m_Client(std::move(client)),
      m_AppKey(app_key),
      m_ApiUrl(api_url),
      m_FlushInterval(std::chrono::seconds(5))
{
    m_WorkerThread = std::thread(&Aptabase::WorkerProvider::WorkerLoop, this);
}

Aptabase::WorkerProvider::~WorkerProvider()
{
    m_StopFlag = true;
    m_Condition.notify_all();

    Flush();   // Final flush for any remaining data

    if (m_WorkerThread.joinable())
        m_WorkerThread.join();
}

void Aptabase::WorkerProvider::RecordEvent(Event &&event)
{
    {
        std::lock_guard<std::mutex> lock(m_EventMutex);
        m_PendingEvents.emplace_back(std::move(event));
    }
    m_Condition.notify_one();  // wake up flush thread
}

bool Aptabase::WorkerProvider::AnyPending() const
{
    std::lock_guard<std::mutex> lock(m_EventMutex);
    return !m_PendingEvents.empty();
}

bool Aptabase::WorkerProvider::AnySending() const
{
    return m_PendingSends.load() > 0;
}

void Aptabase::WorkerProvider::Flush()
{
    CopyAndFlushBatch();
}

void Aptabase::WorkerProvider::WorkerLoop()
{
    std::unique_lock<std::mutex> lock(m_ConditionMutex);
    while (!m_StopFlag.load()) {
        m_Condition.wait_for(lock, m_FlushInterval, [this]() {
            return m_StopFlag || AnyPending();
        });

        if (m_StopFlag)
            break;

        if (AnyPending()) {
            CopyAndFlushBatch();
        }
    }
}

void Aptabase::WorkerProvider::CopyAndFlushBatch()
{
    std::vector<Event> events;
    {
        std::lock_guard<std::mutex> lock(m_EventMutex);
        if (m_PendingEvents.empty())
            return;
        events.swap(m_PendingEvents);
    }

    m_PendingSends.fetch_add(1);

    m_Client->PostEvents(
        m_ApiUrl,
        m_AppKey,
        events,
        [this, count = events.size()](std::int32_t status) {
            if (status >= 200 && status < 300) {
                m_LogFunction(Verbosity::Info, std::to_string(count) + " event(s) sent.");
            } else {
                m_LogFunction(Verbosity::Error, "HTTP " + std::to_string(status) + " while sending " + std::to_string(count) + " event(s).");
            }
            m_PendingSends.fetch_sub(1);
        });
}