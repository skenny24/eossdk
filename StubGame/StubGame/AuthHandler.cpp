// Copyright Epic Games, Inc. All Rights Reserved.

#include <Windows.h>
#include <iostream>
#include <Windows/eos_Windows.h>
#include <eos_init.h>
#include <eos_logging.h>
#include <eos_common.h>
#include <eos_auth.h>
#include <eos_connect.h>
#include <eos_sdk.h>
#include <cassert>
#include <string.h>
#include "SDKConfig.h"
#include "PlatformHandler.h"

#include "AuthHandler.h"

EOS_HPlatform AuthHandler::platformHandle;
EOS_EpicAccountId AuthHandler::EpicId;
EOS_ProductUserId AuthHandler::puid;
bool AuthHandler::bAuthLoginCompleted = false;
bool AuthHandler::bConnectLoginCompleted = false;

struct FConnectLoginContext
{
	EOS_EpicAccountId AccountId;
};

AuthHandler::AuthHandler() {

}

EOS_ProductUserId AuthHandler::Login(SDKConfig* Config, EOS_HPlatform PlatformHandle)
{
	platformHandle = PlatformHandle;

	// If Set to EOS_ENetworkStatus::EOS_NS_Online the log in works fine
	EOS_EResult SetNetworkStatusResult = EOS_Platform_SetNetworkStatus(platformHandle, EOS_ENetworkStatus::EOS_NS_Online);
	assert(SetNetworkStatusResult == EOS_EResult::EOS_Success);

	EOS_HAuth AuthHandle = EOS_Platform_GetAuthInterface(platformHandle);
	assert(AuthHandle != nullptr);

	// Set the credentials for the DevAuthTool 
	EOS_Auth_Credentials Credentials = {};
	Credentials.ApiVersion = EOS_AUTH_CREDENTIALS_API_LATEST;
	Credentials.Type = EOS_ELoginCredentialType::EOS_LCT_Developer;
	Credentials.Token = Config->CredentialName;
	Credentials.Id = Config->Port;

	// Log into an Epic Games account using the Auth Interface
	EOS_Auth_LoginOptions LoginOptions;
	LoginOptions.ApiVersion = EOS_AUTH_LOGIN_API_LATEST;
	LoginOptions.ScopeFlags = EOS_EAuthScopeFlags::EOS_AS_BasicProfile;
	LoginOptions.Credentials = &Credentials;
	EOS_Auth_Login(AuthHandle, &LoginOptions, nullptr, AuthLoginCompleteCallbackFn);

	// Tick the platform until the callback is received
	while (!bAuthLoginCompleted)
	{
		EOS_Platform_Tick(platformHandle);
	}

	// Copy the Auth token
	EOS_Auth_Token* UserAuthToken = nullptr;
	EOS_Auth_CopyUserAuthTokenOptions CopyTokenOptions = { 0 };
	CopyTokenOptions.ApiVersion = EOS_AUTH_COPYUSERAUTHTOKEN_API_LATEST;

	// Get a handle to the Connect Interface
	EOS_HConnect ConnectHandle = EOS_Platform_GetConnectInterface(platformHandle);
	assert(ConnectHandle != nullptr);

	if (EOS_Auth_CopyUserAuthToken(AuthHandle, &CopyTokenOptions, EpicId, &UserAuthToken) == EOS_EResult::EOS_Success)
	{
		// With a valid auth token, login to EOS using the Connect Interface
		EOS_Connect_Credentials Credentials;
		Credentials.ApiVersion = EOS_CONNECT_CREDENTIALS_API_LATEST;
		Credentials.Token = UserAuthToken->AccessToken;
		Credentials.Type = EOS_EExternalCredentialType::EOS_ECT_EPIC;

		EOS_Connect_LoginOptions Options = { 0 };
		Options.ApiVersion = EOS_CONNECT_LOGIN_API_LATEST;
		Options.Credentials = &Credentials;
		Options.UserLoginInfo = nullptr;

		std::unique_ptr<FConnectLoginContext> ClientData(new FConnectLoginContext);
		ClientData->AccountId = EpicId;

		EOS_Connect_Login(ConnectHandle, &Options, ClientData.release(), ConnectLoginCompleteCb);
		EOS_Auth_Token_Release(UserAuthToken);
	}
	else
	{
		std::cout << "Failed to copy user auth token!" << std::endl;
		return nullptr;
	}

	// Tick the platform untuil the callback is received
	while (!bConnectLoginCompleted)
	{
		EOS_Platform_Tick(platformHandle);
	}

	// Return this player's PUID
	return puid;
}

void EOS_CALL AuthHandler::AuthLoginCompleteCallbackFn(const EOS_Auth_LoginCallbackInfo* Data)
{
	assert(Data != NULL);

	if (Data->ResultCode == EOS_EResult::EOS_Success)
	{
		std::cout << "Auth Login Completed!" << std::endl;
		EpicId = Data->LocalUserId;
		bAuthLoginCompleted = true;
		return;
	}
	else
	{
		std::cout << "Auth Login Failed, terminate manually" << std::endl;
		return;

	}
}

void EOS_CALL AuthHandler::ConnectLoginCompleteCb(const EOS_Connect_LoginCallbackInfo* Data)
{
	assert(Data != NULL);

	std::unique_ptr<FConnectLoginContext> ClientData(static_cast<FConnectLoginContext*>(Data->ClientData));

	if (Data->ResultCode == EOS_EResult::EOS_Success)
	{
		// Player has successfully logged in
		std::cout << "Connect Login Completed!" << std::endl;
		bConnectLoginCompleted = true;
		puid = Data->LocalUserId;
		return;
	}
	else if (Data->ResultCode == EOS_EResult::EOS_InvalidUser)
	{
		// This player has no existing PUID so we need to create a new one
		EOS_HConnect ConnectHandle = EOS_Platform_GetConnectInterface(platformHandle);
		assert(ConnectHandle != NULL);
		EOS_Connect_CreateUserOptions Options;
		Options.ApiVersion = EOS_CONNECT_CREATEUSER_API_LATEST;
		if (Data->ContinuanceToken != NULL)
		{
			Options.ContinuanceToken = Data->ContinuanceToken;
		}
		// NOTE: We're not deleting the received context because we're passing it down to another SDK call
		std::cout << "___________CALLING CONNECT_CREATEUSER_______!" << std::endl;
		EOS_Connect_CreateUser(ConnectHandle, &Options, nullptr, ConnectCreateUserCompleteCb);
	}
	else
	{
		std::cout << "Connect Login Failed, terminate manually." << std::endl;
		return;
	}
}

void EOS_CALL AuthHandler::ConnectCreateUserCompleteCb(const EOS_Connect_CreateUserCallbackInfo* Data)
{
	assert(Data != NULL);

	std::unique_ptr<FConnectLoginContext> ClientData(static_cast<FConnectLoginContext*>(Data->ClientData));

	if (Data->ResultCode == EOS_EResult::EOS_Success)
	{
		// New user successfully created and PUID returned
		std::cout << "New user created!" << std::endl;
		std::cout << "Connect Login Completed!" << std::endl;
		bConnectLoginCompleted = true;
		puid = Data->LocalUserId;
		return;
	}

	return void EOS_CALL();
}
