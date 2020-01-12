#pragma once

#include <SmingCore.h>
#include <functional>

struct WifiConnectionData {
	const char* Ssid = nullptr;
	const char* Password = nullptr;
};

class WifiManager
{
public:
    static void Init();
protected:
    static void Connect();
    static void OnWifiDisconnected(const String&, MacAddress, WifiDisconnectReason);
    static void OnWifiConnected(IpAddress Ip, IpAddress Netmask, IpAddress Gateway);
public:
    static std::function<void()> OnConnected;
    static std::function<void()> OnConnectionFail;
    static IpAddress Ip;
    static IpAddress Netmask;
    static IpAddress Gateway;
    static bool Connected;
protected:
    static uint8_t CurrentConnectionIndex;
    static constexpr WifiConnectionData Connections[] = {
        {"BTHub6-F35M_2GEXT_", "Lq7RCE6JnVcN"},
        {"BTHub6-F35M_2GEXT", "Lq7RCE6JnVcN"},
        {"Pixel_5361", "febbraio"}
    };
};