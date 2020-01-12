#include "WifiManager.h"

uint8_t WifiManager::CurrentConnectionIndex = 0;
bool WifiManager::Connected = false;
IpAddress WifiManager::Ip = IpAddress();
IpAddress WifiManager::Gateway = IpAddress();
IpAddress WifiManager::Netmask = IpAddress();
std::function<void()> WifiManager::OnConnected = nullptr;
std::function<void()> WifiManager::OnConnectionFail = nullptr;

void WifiManager::OnWifiDisconnected(const String&, MacAddress, WifiDisconnectReason)
{
	if (WifiManager::Connected)
	{
		return;
	}

	Serial.print(F("Couldn't connect to "));
	Serial.println(WifiManager::Connections[WifiManager::CurrentConnectionIndex].Ssid);
	Serial.print(F("Trying with "));

	WifiManager::CurrentConnectionIndex++;
	if (WifiManager::CurrentConnectionIndex >= sizeof(WifiManager::Connections) / sizeof(WifiConnectionData))
	{
		WifiManager::CurrentConnectionIndex = 0;
		if (WifiManager::OnConnectionFail)
		{
			WifiManager::OnConnectionFail();
		}
		return;
	}

	Serial.println(WifiManager::Connections[WifiManager::CurrentConnectionIndex].Ssid);
	WifiManager::Connect();
}

void WifiManager::OnWifiConnected(IpAddress Ip, IpAddress Netmask, IpAddress Gateway)
{
	Serial.println("WifiManager::OnWifiConnected: Connected.");
	WifiManager::Ip = Ip;
	WifiManager::Netmask = Netmask;
	WifiManager::Gateway = Gateway;

	WifiManager::Connected = true;

	if (WifiManager::OnConnected)
	{
		Serial.println("WifiManager::OnWifiConnected: Calling OnWifiConnected callback.");
		WifiManager::OnConnected();
	}
}

void WifiManager::Connect()
{
	Serial.println("Connecting...");
	WifiStation.enable(true);
	WifiStation.config(WifiManager::Connections[WifiManager::CurrentConnectionIndex].Ssid, WifiManager::Connections[WifiManager::CurrentConnectionIndex].Password, false, false);
	WifiStation.connect();
}

void WifiManager::Init()
{
	WifiEvents.onStationGotIP(&WifiManager::OnWifiConnected);
	WifiEvents.onStationDisconnect(&WifiManager::OnWifiDisconnected);

	WifiManager::Connect();
}