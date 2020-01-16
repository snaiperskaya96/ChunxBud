#include "UpdateManager.h"
#include "SmingCore.h"
#include "Version.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "proto/updater.pb.h"

#ifdef PROD
const char* VersionUrl = "https://chunx.skayahack.uk/updater/version/";
const char* LatestUrl = "https://chunx.skayahack.uk/updater/latest/";
const char* ProtoUrl = "skayahack.uk";
const uint32_t ProtoPort = 6684;
#else
const char* VersionUrl = "http://192.168.1.146:8080/updater/version/";
const char* LatestUrl = "http://192.168.1.146:8080/updater/latest/";
const char* ProtoUrl = "192.168.1.146";
const uint16_t ProtoPort = 6684;
#endif

std::function<void(bool)> UpdateManager::OnUpdateCheckComplete;
std::function<void()> UpdateManager::OnUpdateCheckFailed;
SimpleTimer UpdateManager::UpdateCheckTimer;
TcpClient UpdateManager::ProtoClient = TcpClient(&UpdateManager::OnProtoClientComplete, &UpdateManager::OnProtoClientEvent, &UpdateManager::OnProtoClientDataReceived);

FChunxMessage ChunxMessageWrapper = FChunxMessage_init_zero;
uint8_t ChunxMessageBuffer[256] = { 0x0 };
RbootOutputStream UpdateManager::RomStream = RbootOutputStream(0);

bool AskForVersion = false;
bool AskForUpdate = false;

void UpdateManager::OnProtoClientEvent(TcpClient& Client, TcpConnectionEvent SourceEvent)
{
    if (SourceEvent == TcpConnectionEvent::eTCE_Connected)
    {
        if (AskForVersion)
        {
            ChunxMessageWrapper.MessageType = 1;
            pb_ostream_t BufferStream = pb_ostream_from_buffer(&ChunxMessageBuffer[1], 255);
            
            if (pb_encode(&BufferStream, FChunxMessage_fields, &ChunxMessageWrapper))
            {
                uint8_t MessageSize = (uint8_t) BufferStream.bytes_written;
                ChunxMessageBuffer[0] = MessageSize;
                UpdateManager::ProtoClient.write((const char*)ChunxMessageBuffer, sizeof(uint8_t) + MessageSize);
            }
            AskForVersion = false;
        }

        if (AskForUpdate)
        {
                ChunxMessageWrapper.MessageType = 2;
                pb_ostream_t BufferStream = pb_ostream_from_buffer(&ChunxMessageBuffer[1], 255);
                
                if (pb_encode(&BufferStream, FChunxMessage_fields, &ChunxMessageWrapper))
                {
                    uint8_t MessageSize = (uint8_t) BufferStream.bytes_written;
                    ChunxMessageBuffer[0] = MessageSize;
                    UpdateManager::ProtoClient.write((const char*)ChunxMessageBuffer, sizeof(uint8_t) + MessageSize);
                }
                AskForUpdate = false;
        }
    }
}

void UpdateManager::OnProtoClientComplete(TcpClient& Client, bool Successful)
{
    
}

#define TCP_BUFFER_SIZE 1024 * 5

bool UpdateManager::OnProtoClientDataReceived(TcpClient& Client, char* Data, int Size)
{
    static char RequestBuffer[TCP_BUFFER_SIZE] = { 0x0 };
    static int BufferLength = 0;
    static bool IsWaitingForPacket = false;
    static uint8_t PacketSize = 0;
    static FChunxMessage Packet = FChunxMessage_init_zero;
    
    auto DecodePacket = [&]()
    {
        pb_istream_t Stream = pb_istream_from_buffer((const pb_byte_t*)&RequestBuffer[1], PacketSize);
        if (!pb_decode(&Stream, FChunxMessage_fields, &Packet))
        {
            Serial.println("Decoding failed");
        }
    };

    auto HandlePacket = [&]()
    {
        if (Packet.MessageType == 1)
        {
            Serial.printf("Latest version: %s\n", Packet.Message.Version.VersionString);
            std::string Version = std::string(Packet.Message.Version.VersionString);
            OnVersionRequestCompleted(Version);
            //UpdateManager::ProtoClient.close();
        }
        if (Packet.MessageType == 2)
        {
            Serial.printf("Received rom packet %d/%d\n", Packet.Message.UpdateBinary.ChunkId, Packet.Message.UpdateBinary.ChunkMax);
            //fileWrite(UpdateManager::UpdateFile, (void*)Packet.Message.UpdateBinary.RomChunk.bytes, Packet.Message.UpdateBinary.RomChunk.size);
            UpdateManager::RomStream.write(Packet.Message.UpdateBinary.RomChunk.bytes, Packet.Message.UpdateBinary.RomChunk.size);
            if (Packet.Message.UpdateBinary.ChunkId == Packet.Message.UpdateBinary.ChunkMax)
            {
                Serial.println("");
                Serial.println("Update rom downloaded successfuly");
                UpdateManager::OnRomDownloaded();
                return;
            }
        }

        //Packet = FChunxMessage_init_zero;
    };

    auto ShiftBuffer = [&](uint8_t Size)
    {
        memcpy(RequestBuffer, &RequestBuffer[Size], TCP_BUFFER_SIZE - Size);
        BufferLength -= Size;
    };
    
    if (BufferLength + Size > TCP_BUFFER_SIZE)
    {
        Serial.printf("Buffering too much data (%d/%d) from the TCP socket. Something is going wrong. Aborting.\n", BufferLength + Size, TCP_BUFFER_SIZE);
        UpdateManager::ProtoClient.close();
        return false;
    }
    memcpy(&RequestBuffer[BufferLength], Data, Size);
    BufferLength += Size;

    Serial.printf("Buffered tcp packet: %d out %d bytes\n", BufferLength, TCP_BUFFER_SIZE);

    if (IsWaitingForPacket)
    {
        if (BufferLength >= PacketSize)
        {
            DecodePacket();
            HandlePacket();
            ShiftBuffer(PacketSize + 1);
            IsWaitingForPacket = false;
        }
    }

    if (!IsWaitingForPacket)
    {
        while (true)
        {
            PacketSize = (uint8_t)RequestBuffer[0];

            if (PacketSize + 1 > BufferLength)
            {
                Serial.printf("Not enough data to decode packet (need %d got %d)\n", PacketSize + 1, BufferLength);
                IsWaitingForPacket = true;
            }
            else
            {
                Serial.printf("Decoding packet\n");
                DecodePacket();
                HandlePacket();
                ShiftBuffer(PacketSize + 1);
            }

            if (IsWaitingForPacket || BufferLength == 0)
            {
                break;
            }
        }
    }
    
    return true;
}

void UpdateManager::OnRomDownloaded()
{
    UpdateManager::RomStream.close();
    UpdateManager::ProtoClient.close();

    Serial.println("Update applied successfully");
    const uint8_t TargetSlot = (rboot_get_current_rom() == 0 ? 1 : 0);
    Serial.printf("Rebooting to slot %d..\r\n", TargetSlot);
    rboot_set_current_rom(TargetSlot);
    System.restart();
}

void UpdateManager::Update()
{
	Serial.println("Updating...");


	rboot_config BootConf = rboot_get_config();
	uint8_t UpdateSlot = (BootConf.current_rom == 1 ? 0 : 1);

    UpdateManager::RomStream = RbootOutputStream(BootConf.roms[UpdateSlot]);
    AskForUpdate = true;
    ProtoClient.connect(ProtoUrl, ProtoPort);
}

void UpdateManager::CheckForUpdates()
{
    UpdateManager::UpdateCheckTimer.initializeUs<10000000>([]()
    {
        if (UpdateManager::OnUpdateCheckFailed)
        {
            UpdateManager::ProtoClient.close();
            UpdateManager::OnUpdateCheckFailed();
        }
    });

    UpdateManager::UpdateCheckTimer.startOnce();
    
    Serial.println("UpdateManager::CheckForUpdates: Sending version check request...");

    AskForVersion = true;
    ProtoClient.connect(ProtoUrl, ProtoPort);
    return;
    //Client.downloadString(VersionUrl, &UpdateManager::OnVersionRequestCompleted);
}

void UpdateManager::BroadcastVersionCheckResult(bool NewVersionAvailable)
{
    if (UpdateManager::OnUpdateCheckComplete)
    {
        UpdateManager::OnUpdateCheckComplete(NewVersionAvailable);
    }
}

void UpdateManager::OnVersionRequestCompleted(const std::string& Version)
{
    UpdateManager::UpdateCheckTimer.stop();

    EVersionCompareResult CompareResult = CompareVersion(Version);
    if (CompareResult == EVersionCompareResult::NewerVersion)
    {
        Serial.println("Newer version of ChunxBud found.");
        BroadcastVersionCheckResult(true);
    }
    else if (CompareResult == EVersionCompareResult::InvalidVersion)
    {
        Serial.print("Invalid version retrieved from the server: ");
        Serial.println(Version.c_str());
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
        Serial.print(Version.c_str());
        Serial.print("seems to be older than ");
        Serial.println(CurrentVersion);
        BroadcastVersionCheckResult(false);
    }
}