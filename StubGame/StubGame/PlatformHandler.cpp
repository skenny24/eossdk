// Copyright Epic Games, Inc. All Rights Reserved.

#include <Windows.h>
#include <iostream>
#include <Windows/eos_Windows.h>
#include <eos_init.h>
#include <eos_logging.h>
#include <eos_common.h>
#include <eos_sdk.h>
#include <cassert>
#include <string.h>
#include "SDKConfig.h"

#include "PlatformHandler.h"

PlatformHandler::PlatformHandler() {

}

EOS_HPlatform PlatformHandler::InitializePlatform(SDKConfig* Config)
{
	// Init the EOS SDK
	EOS_InitializeOptions SDKOptions = {};
	SDKOptions.ApiVersion = EOS_INITIALIZE_API_LATEST;
	SDKOptions.AllocateMemoryFunction = nullptr;
	SDKOptions.ReallocateMemoryFunction = nullptr;
	SDKOptions.ReleaseMemoryFunction = nullptr;
	SDKOptions.ProductName = Config->GameName;
	SDKOptions.ProductVersion = "1.0";
	SDKOptions.Reserved = nullptr;
	SDKOptions.SystemInitializeOptions = nullptr;
	SDKOptions.OverrideThreadAffinity = nullptr;

	EOS_EResult InitResult = EOS_Initialize(&SDKOptions);
	assert(InitResult == EOS_EResult::EOS_Success);

	// Register the logging callback function and set logging level
	EOS_EResult SetLogCallbackResult = EOS_Logging_SetCallback(&EOSSDKLoggingCallback);
	assert(SetLogCallbackResult == EOS_EResult::EOS_Success);
	EOS_EResult SetLogLevelResult = EOS_Logging_SetLogLevel(EOS_ELogCategory::EOS_LC_ALL_CATEGORIES, EOS_ELogLevel::EOS_LOG_VeryVerbose);
	assert(SetLogLevelResult == EOS_EResult::EOS_Success);

	// Create the platform using product credentials taken from the dev portal (configured in SDKConfig.h)
	double timeoutSetting = 5;
	EOS_Platform_Options PlatformOptions = {};
	PlatformOptions.ApiVersion = EOS_PLATFORM_OPTIONS_API_LATEST;
	PlatformOptions.bIsServer = EOS_FALSE;
	PlatformOptions.EncryptionKey = Config->EncryptionKey;
	PlatformOptions.OverrideCountryCode = nullptr;
	PlatformOptions.OverrideLocaleCode = nullptr;
	PlatformOptions.ProductId = Config->ProductId;
	PlatformOptions.SandboxId = Config->SandboxId;
	PlatformOptions.DeploymentId = Config->DeploymentId;
	PlatformOptions.ClientCredentials.ClientId = Config->ClientCredentialsId;
	PlatformOptions.ClientCredentials.ClientSecret = Config->ClientCredentialsSecret;
	PlatformOptions.Flags = EOS_PF_DISABLE_OVERLAY;
	PlatformOptions.TaskNetworkTimeoutSeconds = &timeoutSetting;

	EOS_Platform_RTCOptions RtcOptions = { 0 };
	RtcOptions.ApiVersion = EOS_PLATFORM_RTCOPTIONS_API_LATEST;

	EOS_HPlatform platformHandle = EOS_Platform_Create(&PlatformOptions);

	return platformHandle;
}


void EOS_CALL PlatformHandler::EOSSDKLoggingCallback(const EOS_LogMessage* InMsg)
{
	std::string LogMessage = InMsg->Message;
	if (LogMessage == "[INTERNAL] Tick") // logs will get spammed with tick messages
	{
		return;
	}
	if (InMsg->Level != EOS_ELogLevel::EOS_LOG_Off)
	{
		std::cout << InMsg->Category << "::" << InMsg->Message << std::endl;
	}
}
