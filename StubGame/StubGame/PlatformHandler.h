// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include <eos_init.h>
#include "SDKConfig.h"

class PlatformHandler {
public:
	PlatformHandler();
	
	EOS_HPlatform InitializePlatform(SDKConfig* Config);

private:
	static void EOS_CALL EOSSDKLoggingCallback(const EOS_LogMessage* InMsg);
};
