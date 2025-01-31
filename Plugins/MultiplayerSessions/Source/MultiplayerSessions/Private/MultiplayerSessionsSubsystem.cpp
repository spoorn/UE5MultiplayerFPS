// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionsSubsystem.h"

#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Online/OnlineSessionNames.h"

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem() :
	OnCreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
	OnFindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)),
	OnJoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
	OnDestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete)),
	OnStartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete))
{
	if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		SessionInterface = Subsystem->GetSessionInterface();
	}
}

void UMultiplayerSessionsSubsystem::CreateSession(int32 NumPublicConnections, FString MatchType)
{
	if (!SessionInterface.IsValid())
	{
		MultiplayerOnCreateSessionComplete.Broadcast(false);
		return;
	}
	// Destroy any existing session
	if (SessionInterface->GetNamedSession(NAME_GameSession))
	{
		bCreateSessionOnDestroy = true;
		LastNumPublicConnections = NumPublicConnections;
		LastMatchType = MatchType;
		// If there is an existing session, destroy session and wait for completion before trying to create again
		DestroySession(NAME_GameSession);
	} else
	{
		// Register delegate and save handle
		CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);

		// Settings
		LastSessionSettings = MakeShared<FOnlineSessionSettings>();
		// If no backend subsystem is available, default to LAN
		LastSessionSettings->bIsLANMatch = ShouldUseLAN();
		LastSessionSettings->NumPublicConnections = NumPublicConnections;
		LastSessionSettings->bAllowJoinInProgress = true;
		LastSessionSettings->bAllowJoinViaPresence = true;
		LastSessionSettings->bShouldAdvertise = true;
		LastSessionSettings->bUsesPresence = true;
		LastSessionSettings->bUseLobbiesIfAvailable = true;
		LastSessionSettings->BuildUniqueId = 0313;
		// Set match type
		LastSessionSettings->Set(MatchTypeName, MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
		if (!SessionInterface->CreateSession(*GetLocalPlayer()->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettings))
		{
			// Remove delegate from delegate list if failed to create session
			SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
			// Broadcast failure to delegates
			MultiplayerOnCreateSessionComplete.Broadcast(false);
		}
	}
}

void UMultiplayerSessionsSubsystem::FindSessions(int32 MaxSearchResults)
{
	if (!SessionInterface.IsValid())
	{
		MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
	}
	// Register delegate
	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegate);
	// Settings
	LastSessionSearch = MakeShared<FOnlineSessionSearch>();
	LastSessionSearch->MaxSearchResults = MaxSearchResults;
	LastSessionSearch->bIsLanQuery = ShouldUseLAN();
	// Only find sessions using presence
	LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
	if (!SessionInterface->FindSessions(*GetLocalPlayer()->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef()))
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
		MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
	}
}

void UMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionSearchResult)
{
	if (!SessionInterface.IsValid())
	{
		MultiplayerOnJoinSessionComplete.Broadcast(NAME_GameSession, EOnJoinSessionCompleteResult::UnknownError);
		return;
	}
	// Register delegate
	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);
	if (!SessionInterface->JoinSession(*GetLocalPlayer()->GetPreferredUniqueNetId(), NAME_GameSession, SessionSearchResult))
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		MultiplayerOnJoinSessionComplete.Broadcast(NAME_GameSession, EOnJoinSessionCompleteResult::Success);
	}
}

void UMultiplayerSessionsSubsystem::DestroySession(FName SessionName)
{
	if (!SessionInterface.IsValid())
	{
		MultiplayerOnDestroySessionComplete.Broadcast(SessionName, false);
		return;
	}
	// Register delegate
	DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegate);
	if (!SessionInterface->DestroySession(SessionName))
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		MultiplayerOnDestroySessionComplete.Broadcast(SessionName, false);
	}
}

void UMultiplayerSessionsSubsystem::StartSession()
{
	if (!SessionInterface.IsValid())
	{
		MultiplayerOnStartSessionComplete.Broadcast(NAME_GameSession, false);
		return;
	}
	// Register delegate
	StartSessionCompleteDelegateHandle = SessionInterface->AddOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegate);
	if (!SessionInterface->StartSession(NAME_GameSession))
	{
		SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
		MultiplayerOnStartSessionComplete.Broadcast(NAME_GameSession, false);
	}
}

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}
	MultiplayerOnCreateSessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	}
	MultiplayerOnFindSessionsComplete.Broadcast(LastSessionSearch->SearchResults, bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	}
	MultiplayerOnJoinSessionComplete.Broadcast(SessionName, Result);
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	}
	if (bCreateSessionOnDestroy && bWasSuccessful)
	{
		bCreateSessionOnDestroy = false;
		CreateSession(LastNumPublicConnections, LastMatchType);
	}
	MultiplayerOnDestroySessionComplete.Broadcast(SessionName, bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
	}
	MultiplayerOnStartSessionComplete.Broadcast(SessionName, bWasSuccessful);
}

ULocalPlayer* UMultiplayerSessionsSubsystem::GetLocalPlayer() const
{
	if (UWorld* World = GetWorld())
	{
		return World->GetFirstLocalPlayerFromController();
	}
	return nullptr;
}

bool UMultiplayerSessionsSubsystem::ShouldUseLAN()
{
	return IOnlineSubsystem::Get()->GetSubsystemName() == "NULL";
}
