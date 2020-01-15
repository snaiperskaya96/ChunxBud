#pragma once

#include <Network/HttpClient.h>
#include <functional>
#include <SimpleTimer.h>
#include <Core/Data/Stream/RbootOutputStream.h>

class UpdateManager
{
public:
    static void CheckForUpdates();
    static void Update();
protected:
    static void OnRomDownloaded();
    static void OnVersionRequestCompleted(const std::string& Version);
    static void BroadcastVersionCheckResult(bool NewVersionAvailable);
    static void OnProtoClientEvent(TcpClient& Client, TcpConnectionEvent SourceEvent);
    static void OnProtoClientComplete(TcpClient& Client, bool Successful);
    static bool OnProtoClientDataReceived(TcpClient& Client, char* Data, int Size);
public:
    static std::function<void(bool)> OnUpdateCheckComplete;
    static std::function<void()> OnUpdateCheckFailed;
protected:
    static TcpClient ProtoClient;
    static SimpleTimer UpdateCheckTimer;
    static RbootOutputStream RomStream;
};