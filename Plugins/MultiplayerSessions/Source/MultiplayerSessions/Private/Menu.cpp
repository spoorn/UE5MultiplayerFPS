// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"

#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"

void UMenu::MenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch, FString PathToLobby)
{
	NumPublicConnections = NumberOfPublicConnections;
	MatchType = TypeOfMatch;
	LobbyPath = FString::Printf(TEXT("%s?listen"), *PathToLobby);
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);

	if (const UWorld* World = GetWorld())
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			// Switch to menu widget mode
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}

	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}

	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
		MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSessions);
		MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
		MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
	}
}

bool UMenu::Initialize()
{
	if (!Super::Initialize()) return false;
	// Bind callback functions to buttons
	if (HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &ThisClass::HostButtonClicked);
	}
	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClicked);
	}
	return true;
}

void UMenu::NativeDestruct()
{
	MenuTearDown();
	Super::NativeDestruct();
}

void UMenu::OnCreateSession(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Green, FString(TEXT("Session created successfully!")));
		if (UWorld* World = GetWorld())
		{
			// Join the lobby level
			World->ServerTravel(LobbyPath);
		}
	} else
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Red, FString(TEXT("Failed to create session!")));
		HostButton->SetIsEnabled(true);  // Re-enable host button
	}
}

void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Green, FString(TEXT("Find sessions query success")));
	} else
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Red, FString(TEXT("Find sessions query failed!")));
	}

	if (!MultiplayerSessionsSubsystem) return;
	if (SessionResults.Num() == 0)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Blue, FString(TEXT("Did not find any sessions")));
	}
	for (auto Result : SessionResults)
	{
		FString MatchTypeValue;
		Result.Session.SessionSettings.Get(UMultiplayerSessionsSubsystem::MatchTypeName, MatchTypeValue);
		// Check that result match type matches our query
		if (MatchTypeValue == MatchType)
		{
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Yellow, FString::Printf(TEXT("Joining session: %s, owner: %s"), *Result.Session.GetSessionIdStr(), *Result.Session.OwningUserName));
			// TODO: Joins the first matching session only for now
			MultiplayerSessionsSubsystem->JoinSession(Result);
			return;
		}
	}

	if (!bWasSuccessful || SessionResults.Num() == 0)
	{
		JoinButton->SetIsEnabled(true);
	}
}

void UMenu::OnJoinSession(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		if (const IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
		{
			if (const IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface(); SessionInterface.IsValid())
			{
				FString Address;
				if (APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController(); PlayerController && SessionInterface->GetResolvedConnectString(SessionName, Address))
				{
					PlayerController->ClientTravel(Address, TRAVEL_Absolute);
				}
			}
		}
	} else
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Red, FString::Printf(TEXT("Failed to join session: %s"), LexToString(Result)));
		JoinButton->SetIsEnabled(true);
	}
}

void UMenu::OnDestroySession(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Green, FString::Printf(TEXT("Destroyed session: %s"), *SessionName.ToString()));
	} else
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Red, FString::Printf(TEXT("Failed to destroy session: %s"), *SessionName.ToString()));
	}
}

void UMenu::OnStartSession(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Green, FString::Printf(TEXT("Started session: %s"), *SessionName.ToString()));
	} else
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Red, FString::Printf(TEXT("Failed to start session: %s"), *SessionName.ToString()));
	}
}

void UMenu::MenuTearDown()
{
	RemoveFromParent();
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			// Go back to game mode
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
		}
	}
}

void UMenu::HostButtonClicked()
{
	HostButton->SetIsEnabled(false);
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
	}
}

void UMenu::JoinButtonClicked()
{
	JoinButton->SetIsEnabled(false);
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->FindSessions(10000);
	}
}
