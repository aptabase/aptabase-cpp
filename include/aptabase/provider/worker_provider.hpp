#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "provider.hpp"
#include "aptabase/net/client.hpp"

class AptabaseWorkerProvider : public AptabaseProvider {
private:
    std::unique_ptr<AptabaseHttpClient> m_Client;
    std::string m_AppKey;
    std::string m_ApiUrl;

    std::vector<AptabaseEventPayload> m_PendingEvents;
    mutable std::mutex m_EventMutex;

    std::atomic<int> m_PendingSends{0};
    std::atomic<bool> m_StopFlag{false};

    std::thread m_WorkerThread;
    std::condition_variable m_Condition;
    std::mutex m_ConditionMutex;

    std::chrono::seconds m_FlushInterval;

    LogFunctionType m_LogFunction = DefaultLogFunction;
public:
    AptabaseWorkerProvider(std::unique_ptr<AptabaseHttpClient> &&client, const std::string &app_key, const std::string &api_url);

    ~AptabaseWorkerProvider();

    void RecordEvent(AptabaseEventPayload &&event) override;

    void Flush() override;

    bool AnyPending() const override;

    bool AnySending() const override;

    bool IsTickRequired() const override { return false; }

    void SetLog(LogFunctionType&& log) { m_LogFunction = std::move(log); }

private:
    void WorkerLoop();
    void CopyAndFlushBatch();

};