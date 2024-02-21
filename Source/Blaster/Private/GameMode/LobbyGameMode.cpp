// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/LobbyGameMode.h"

#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	if (const APlayerState* PlayerState = NewPlayer->GetPlayerState<APlayerState>(); PlayerState && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Purple, FString::Printf(TEXT("%s joined"), *PlayerState->GetPlayerName()));
	}
	if (GameState->PlayerArray.Num() == 2 && !GetWorldTimerManager().IsTimerActive(StartGameTimer))
	{
		// Start timer to send all players to game map
		GetWorldTimerManager().SetTimer(StartGameTimer, this, &ThisClass::StartGame, 1, true);
	}
}

void ALobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
}

void ALobbyGameMode::StartGame()
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(2, 15, FColor::Orange, FString::Printf(TEXT("Starting in %d..."), StartGameCountDown));
	// Start game after a countdown
	if (--StartGameCountDown > 0) return;
	GetWorldTimerManager().ClearTimer(StartGameTimer);
	if (UWorld* World = GetWorld())
	{
		bUseSeamlessTravel = true;
		// Send all players to BlasterMap
		World->ServerTravel("/Game/Maps/BlasterMap");
		if (GEngine) GEngine->ClearOnScreenDebugMessages();
	}
}
