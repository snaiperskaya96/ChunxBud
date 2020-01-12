#pragma once

#include <Network/HttpClient.h>
#include <functional>
#include <SimpleTimer.h>

class UpdateManager
{
public:
    static void CheckForUpdates();
    static void Update();
protected:
    static int OnVersionRequestCompleted(HttpConnection& Client, bool Successful);
    static void OnUpdateCallback(class RbootHttpUpdater& Updater, bool Result);
    static void BroadcastVersionCheckResult(bool NewVersionAvailable);
public:
    static std::function<void(bool)> OnUpdateCheckComplete;
    static std::function<void()> OnUpdateCheckFailed;
    static class RbootHttpUpdater* HttpUpdater;
protected:
    static SimpleTimer UpdateCheckTimer;
    static HttpClient Client;
};