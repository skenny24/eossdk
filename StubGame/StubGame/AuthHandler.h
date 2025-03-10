// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include <eos_init.h>
#include <eos_logging.h>
#include <eos_common.h>
#include <eos_auth.h>
#include <eos_connect.h>
#include <eos_sdk.h>

class AuthHandler {
public:
	AuthHandler();

	EOS_ProductUserId Login(SDKConfig* Config, EOS_HPlatform PlatformHandle);

private:
	static EOS_HPlatform platformHandle;
	static EOS_EpicAccountId EpicId;
	static EOS_ProductUserId puid;
	static bool bAuthLoginCompleted;
	static bool bConnectLoginCompleted;

	static void EOS_CALL AuthLoginCompleteCallbackFn(const EOS_Auth_LoginCallbackInfo* Data);
	static void EOS_CALL ConnectLoginCompleteCb(const EOS_Connect_LoginCallbackInfo* Data);
	static void EOS_CALL ConnectCreateUserCompleteCb(const EOS_Connect_CreateUserCallbackInfo* Data);
};
