#include <Windows.h>
#include <iostream>
#include <string.h>
#include <cassert>
#include <fstream>
#include <thread>
#include <chrono>
#include <cstdint>
// EOS SDK headers
#include <eos_sdk.h>
#include <eos_init.h>
#include <eos_logging.h>
#include <eos_common.h>
#include <eos_auth.h>
#include <eos_connect.h>
#include <eos_stats.h>
#include <eos_achievements.h>
#include "SDKConfig.h"
#include "PlatformHandler.h"
#include "AuthHandler.h"
#include <eos_stats.h>
#include <eos_logging.h>

// Log function that writes messages to both console and log file
void LogMessage(const std::string& message, bool isError = false) {
    // Log to console
    if (isError) {
        std::cerr << message << std::endl;
    } else {
        std::cout << message << std::endl;
    }

    // Log to file
    std::ofstream logFile("game_log.txt", std::ios::app);
    if (logFile.is_open()) {
        logFile << (isError ? "[ERROR] " : "[INFO] ") << message << std::endl;
        logFile.close();
    } else {
        std::cerr << "Failed to open log file!" << std::endl;
    }
}

// Callback function for when the stat ingestion is complete
// this will automatically unlock the swampstatachievement linked on the dev portal
void IngestStatCallback(const EOS_Stats_IngestStatCompleteCallbackInfo* Data)
{
    if (Data == nullptr)
    {
        LogMessage("IngestStat callback received null data.", true);
        return;
    }

    // Check if the ingestion was successful
    if (Data->ResultCode == EOS_EResult::EOS_Success)
    {
        LogMessage("############# STAT SUCCESSFULLY INGESTED AND YOU HAVE NOW UNLOCKED THE swampstatachievement! #################");
    }
    else
    {
        LogMessage("Stat ingestion failed with result code: " + std::string(EOS_EResult_ToString(Data->ResultCode)), true);
    }
}

void ingestStat(EOS_HPlatform platformHandle, EOS_ProductUserId puid, int amount)
{
    EOS_HStats statsHandle = EOS_Platform_GetStatsInterface(platformHandle);
    if (!statsHandle)
    {
        LogMessage("Error: Could not get stats interface.", true);
    }
    else
    {
        LogMessage("Successfully acquired Stats Interface handle: " + std::to_string(reinterpret_cast<uintptr_t>(statsHandle)));
    }

    EOS_Stats_IngestStatOptions ingestStatOptions = {};
    ingestStatOptions.ApiVersion = EOS_STATS_INGESTSTAT_API_LATEST;
    ingestStatOptions.LocalUserId = puid;
    ingestStatOptions.TargetUserId = puid;
    ingestStatOptions.StatsCount = 1;

    // define the stat to ingest
    EOS_Stats_IngestData ingestData = {};
    ingestData.ApiVersion = EOS_STATS_INGESTSTAT_API_LATEST;
    ingestData.StatName = "swampstat"; 
    ingestData.IngestAmount = amount; 

    ingestStatOptions.Stats = &ingestData;
    EOS_Stats_IngestStat(statsHandle, &ingestStatOptions, nullptr, IngestStatCallback);
}

// Callback for Unlock Achievement
void UnlockAchievementCallback(const EOS_Achievements_OnUnlockAchievementsCompleteCallbackInfo* Data)
{
    if (Data->ResultCode == EOS_EResult::EOS_Success)
    {
        std::string successMessage = "################### swampachievement ACHIEVEMENT UNLOCKED...CONGRATULATIONS! ##############";
        LogMessage(successMessage);

        LogMessage(">>>>>>>>>>>>>>>> Notification: Achievement unlocked! Press Enter to continue...");
        std::cin.get(); 
    }
    else
    {
        std::string errorMessage = "Failed to unlock achievement. Error: "
            + std::string(EOS_EResult_ToString(Data->ResultCode));
        LogMessage(errorMessage, true);

        LogMessage("Notification: Failed to unlock achievement. Press Enter to continue...");
        std::cin.get(); 
    }
}

// manual achievement unlock
void UnlockAchievement(EOS_HPlatform platformHandle, EOS_ProductUserId userId)
{
    // get the Achievements Interface
    EOS_HAchievements achievementsHandle = EOS_Platform_GetAchievementsInterface(platformHandle);
    if (!achievementsHandle)
    {
        LogMessage("Error: Could not get achievements interface.", true);
        return;
    }

    // set up the unlock options
    EOS_Achievements_UnlockAchievementsOptions unlockOptions = {};
    unlockOptions.ApiVersion = EOS_ACHIEVEMENTS_UNLOCKACHIEVEMENTS_API_LATEST;
    unlockOptions.UserId = userId;

    const char* achievementIds[] = { "swampachievement" };
    unlockOptions.AchievementIds = achievementIds;
    unlockOptions.AchievementsCount = 1;
    EOS_Achievements_UnlockAchievements(achievementsHandle, &unlockOptions, nullptr, UnlockAchievementCallback);
    // no immediate result so confirm request sent
    LogMessage("Unlock achievement request has been sent.");
}

// perform ticks and keep the platform running
void ProcessPlatformTicks(EOS_HPlatform platformHandle)
{
    while (true)
    {
        EOS_Platform_Tick(platformHandle);  // process pending events
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

int main()
{
    // initialize config and platform
    SDKConfig* config = new SDKConfig;
    PlatformHandler* platformInitializer = new PlatformHandler();
    EOS_HPlatform platformHandle = platformInitializer->InitializePlatform(config);
    assert(platformHandle != nullptr);

    LogMessage("Platform Handle Address: " + std::to_string(reinterpret_cast<uintptr_t>(platformHandle)));

    // login and retrieve a PUID
    AuthHandler* auth = new AuthHandler();
    EOS_ProductUserId puid = auth->Login(config, platformHandle);
    assert(puid != nullptr);
    std::cout << "Logged in and PUID is: " << puid << std::endl;
    std::cout << "Product ID: " << config->ProductId << std::endl;
    std::cout << "Sandbox ID: " << config->SandboxId << std::endl;

    // create platform interface
    EOS_Platform_Options PlatformOptions = {};
    PlatformOptions.ApiVersion = EOS_PLATFORM_OPTIONS_API_LATEST;
    PlatformOptions.bIsServer = EOS_FALSE;
    PlatformOptions.EncryptionKey = config->EncryptionKey;
    PlatformOptions.OverrideCountryCode = nullptr;
    PlatformOptions.OverrideLocaleCode = nullptr;
    PlatformOptions.ProductId = config->ProductId;
    PlatformOptions.SandboxId = config->SandboxId;
    PlatformOptions.DeploymentId = config->DeploymentId;
    PlatformOptions.ClientCredentials.ClientId = config->ClientCredentialsId;
    PlatformOptions.ClientCredentials.ClientSecret = config->ClientCredentialsSecret;
    PlatformOptions.Flags = EOS_PF_DISABLE_OVERLAY;

    EOS_HPlatform platformInterfaceHandle = EOS_Platform_Create(&PlatformOptions);

    if (platformInterfaceHandle == nullptr)
    {
        LogMessage("Failed to create the EOS platform interface.", true);
        return -1;
    }

    LogMessage("Successfully created platform interface handle.");

    std::cout << ">>>>>>>>>>>>>>>>> Press Enter to unlock the achievement 'swampachievement'" << std::endl;
    std::cin.get(); // wait for user input

    std::thread tickThread(ProcessPlatformTicks, platformHandle);
    tickThread.detach();
    UnlockAchievement(platformHandle, puid);

    // Wait for user input to ingest stat
    std::cout << ">>>>>>>>>>>>>>>>>>>>> Press Enter to ingest the stat 'swampstat'" << std::endl;
    std::cin.get(); // wait for user input
    int ingestAmount = 1;
    ingestStat(platformHandle, puid, ingestAmount);

    std::cout << ">>>>>>>>>>>>>>>>> Press Enter to exit (remaining tasks will complete)" << std::endl;
    std::cin.get(); // keep program open to allow async callback to return

    EOS_Platform_Release(platformInterfaceHandle);

    return 0;
}
