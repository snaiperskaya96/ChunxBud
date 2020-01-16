#include "WifiManager.h"
#include "UpdateManager.h"
#include "Version.h"

#include <SmingCore.h>
#include <Network/HttpClient.h>

void Sleep()
{
#ifdef PROD
	// 30 mins
	System.deepSleep(30 * 60 * 1000, DeepSleepOptions::eDSO_RF_CAL_NEVER);
#else
	// 3 secs
	System.deepSleep(3000, DeepSleepOptions::eDSO_RF_CAL_NEVER);
#endif
}

void DoMeasurement()
{
	Serial.println("Doing measurement");
	pinMode(A0, INPUT);
	Serial.println(analogRead(A0));

	Sleep();
}

void OnUpdateCheckComplete(bool NewUpdateAvailable)
{
	Serial.println("Check for updates complete");
	if (NewUpdateAvailable)
	{
		Serial.println("New update available");
		UpdateManager::Update();
	}
	else
	{
		Serial.println("No new update available");
		DoMeasurement();
	}
}

void OnConnectionFailed()
{
	DoMeasurement();
	Sleep();
}

void OnConnected()
{
	Serial.println("Checking for updates...");
	UpdateManager::OnUpdateCheckComplete = std::bind(OnUpdateCheckComplete, std::placeholders::_1);
	UpdateManager::OnUpdateCheckFailed = std::bind(OnConnectionFailed);
	UpdateManager::CheckForUpdates();
}

void OnReady()
{
	Serial.print("Hello. This is ChunxBud ");
	Serial.println(CurrentVersion);

	Serial.println("System Ready.");

	WifiManager::OnConnected = std::bind(OnConnected);
	WifiManager::OnConnectionFail = std::bind(OnConnectionFailed);
	WifiManager::Init();
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true);
	if (rboot_get_current_rom() == 0)
	{
		spiffs_mount_manual(0x100000, SPIFF_SIZE);
	}
	else
	{
		spiffs_mount_manual(0x300000, SPIFF_SIZE);
	}

	System.onReady(OnReady);
}