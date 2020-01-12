#include "WifiManager.h"
#include "UpdateManager.h"
#include "Version.h"

#include <SmingCore.h>
#include <Network/HttpClient.h>

HttpClient httpClient;

int onDownload(HttpConnection& connection, bool success)
{
	debugf("\n=========[ URL: %s ]============", connection.getRequest()->uri.toString().c_str());
	debugf("RemoteIP: %s", connection.getRemoteIp().toString().c_str());
	debugf("Got response code: %d", connection.getResponse()->code);
	debugf("Success: %d", success);
	if(connection.getRequest()->method != HTTP_HEAD) {
		debugf("Got content starting with: %s", connection.getResponse()->getBody().substring(0, 1000).c_str());
	}

	auto ssl = connection.getSsl();
	if(ssl != nullptr) {
		ssl->printTo(Serial);
	}

	return 0; // return 0 on success in your callbacks
}

void sslRequestInit(Ssl::Session& session, HttpRequest& request)
{
	/*
	 * SSL validation: We check the remote server certificate against a fingerprint
	 * Note that fingerprints _may_ change, in which case these need to be updated.
	 *
	 * Note: SSL is not compiled by default. In our example we set the ENABLE_SSL directive to 1
	 * (See: ../Makefile-user.mk )
	 */
	session.options.verifyLater = true;

	// These are the fingerprints for httpbin.org
	static const uint8_t sha1Fingerprint[] PROGMEM = {0x2B, 0xF0, 0x48, 0x9D, 0x78, 0xB4, 0xDE, 0xE9, 0x69, 0xE2,
													  0x73, 0xE0, 0x14, 0xD0, 0xDC, 0xCC, 0xA8, 0xD8, 0x3B, 0x40};

	static const uint8_t publicKeyFingerprint[] PROGMEM = {
		0xE3, 0x88, 0xC4, 0x0A, 0x2A, 0x99, 0x8F, 0xA4, 0x8C, 0x38, 0x4E, 0xE7, 0xCB, 0x4F, 0x8B, 0x99,
		0x19, 0x48, 0x63, 0x9A, 0x2E, 0xD6, 0x05, 0x7D, 0xB1, 0xD3, 0x56, 0x6C, 0xC0, 0x7E, 0x74, 0x1A};

	Ssl::Fingerprints fingerprints;

	// Trust only a certificate in which the public key matches the SHA256 fingerprint...
	fingerprints.setSha256_P(publicKeyFingerprint, sizeof(publicKeyFingerprint));

	// ... or a certificate that matches the SHA1 fingerprint.
	fingerprints.setSha1_P(sha1Fingerprint, sizeof(sha1Fingerprint));

	// Set fingerprints for validation
	session.validators.add(fingerprints);
}

void OtaCallback(RbootHttpUpdater& client, bool result)
{
	if (result)
	{
		Serial.println("Ota update successful");
	}
	else
	{
		Serial.println("Ota update failed");
		return;
	}
}

void Sleep()
{
	System.deepSleep(10000, DeepSleepOptions::eDSO_RF_CAL_NEVER);
}

void DoMeasurement()
{
	Serial.println("Doing measurement");

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