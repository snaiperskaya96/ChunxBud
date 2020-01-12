#include "UpdateManager.h"
#include "SmingCore.h"
#include "mjson.h"
#include "Version.h"

#ifdef PROD
const char* VersionUrl = "https://chunx.skayahack.uk/updater/version/";
const char* LatestUrl = "https://chunx.skayahack.uk/updater/latest/";
#else
const char* VersionUrl = "http://192.168.1.146:8080/updater/version/";
const char* LatestUrl = "http://192.168.1.146:8080/updater/latest/";
#endif

HttpClient UpdateManager::Client = HttpClient();
std::function<void(bool)> UpdateManager::OnUpdateCheckComplete;
std::function<void()> UpdateManager::OnUpdateCheckFailed;
RbootHttpUpdater* UpdateManager::HttpUpdater = nullptr;
SimpleTimer UpdateManager::UpdateCheckTimer;

void UpdateManager::Update()
{
	Serial.println("Updating...");

    WDT.enable(false);

	if(UpdateManager::HttpUpdater)
    {
        delete UpdateManager::HttpUpdater;
        UpdateManager::HttpUpdater = nullptr;
    }
	UpdateManager::HttpUpdater = new RbootHttpUpdater();

	rboot_config BootConf = rboot_get_config();
	uint8_t UpdateSlot = (BootConf.current_rom == 1 ? 0 : 1);

    UpdateManager::HttpUpdater->addItem(BootConf.roms[UpdateSlot], LatestUrl);
    UpdateManager::HttpUpdater->setCallback(&UpdateManager::OnUpdateCallback);
    UpdateManager::HttpUpdater->start();
	Serial.println("Update started...");
}

void UpdateManager::OnUpdateCallback(class RbootHttpUpdater& Updater, bool Result)
{
    if (Result)
    {
        Serial.println("Update applied successfully");
        const uint8_t TargetSlot = (rboot_get_current_rom() == 0 ? 1 : 0);
        Serial.printf("Rebooting to slot %d..\r\n", TargetSlot);
        rboot_set_current_rom(TargetSlot);
        System.restart();
    }
    else
    {
        Serial.println("There was an error while trying to update.");
    }
}

void UpdateManager::CheckForUpdates()
{
    UpdateManager::UpdateCheckTimer.initializeUs<10000000>([]()
    {
        if (UpdateManager::OnUpdateCheckFailed)
        {
            UpdateManager::OnUpdateCheckFailed();
        }
    });
    UpdateManager::UpdateCheckTimer.startOnce();
    
    Serial.println("UpdateManager::CheckForUpdates: Sending version check request...");

    Client.downloadString(VersionUrl, &UpdateManager::OnVersionRequestCompleted);
}

void UpdateManager::BroadcastVersionCheckResult(bool NewVersionAvailable)
{
    if (UpdateManager::OnUpdateCheckComplete)
    {
        UpdateManager::OnUpdateCheckComplete(NewVersionAvailable);
    }
}

int UpdateManager::OnVersionRequestCompleted(HttpConnection& Client, bool Successful)
{
    UpdateManager::UpdateCheckTimer.stop();
    
	Serial.println("UpdateManager::OnVersionRequestCompleted: Version check request complete");
    if (Successful)
    {
        if (HttpResponse* Response = Client.getResponse())
        {
            String ResponseText = Response->getBody();
            char VersionBuffer[32];
            if (mjson_get_string(ResponseText.c_str(), ResponseText.length(), "$.version", VersionBuffer, sizeof(VersionBuffer)) != -1)
            {
                std::string RemoteVersion = std::string(VersionBuffer);
                EVersionCompareResult CompareResult = CompareVersion(RemoteVersion);
                if (CompareResult == EVersionCompareResult::NewerVersion)
                {
                    Serial.println("Newer version of ChunxBud found.");
                    BroadcastVersionCheckResult(true);
                }
                else if (CompareResult == EVersionCompareResult::InvalidVersion)
                {
                    Serial.print("Invalid version retrieved from the server: ");
                    Serial.println(RemoteVersion.c_str());
                    BroadcastVersionCheckResult(false);
                }
                else if (CompareResult == EVersionCompareResult::SameVersion)
                {
                    Serial.println("ChunxBud is up to date!");
                    BroadcastVersionCheckResult(false);
                }
                else
                {
                    Serial.print("Lmao remote version ");
                    Serial.print(RemoteVersion.c_str());
                    Serial.print("seems to be older than ");
                    Serial.println(CurrentVersion);
                    BroadcastVersionCheckResult(false);
                }
            }
            else
            {
                Serial.println("Non-string version value");
                BroadcastVersionCheckResult(false);
            }
        }
    }
    else
    {
        Serial.println("Couldn't retrieve latest version");
        BroadcastVersionCheckResult(false);
    }

    return 0;
}