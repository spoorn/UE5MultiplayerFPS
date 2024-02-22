// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/LobbyGameMode.h"

#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	const int32 NumberOfPlayers = GameState->PlayerArray.Num();
	if (GEngine)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Purple, FString::Printf(TEXT("%s joined"), *PlayerState->GetPlayerName()));
		GEngine->AddOnScreenDebugMessage(3, 15, FColor::Purple, FString::Printf(TEXT("Players in lobby: %d"), NumberOfPlayers));
	}
	if (NumberOfPlayers == 2 && !GetWorldTimerManager().IsTimerActive(StartGameTimer))
	{
		// Start timer to send all players to game map
		GetWorldTimerManager().SetTimer(StartGameTimer, this, &ThisClass::StartGame, 1, true);
	}
}

void ALobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(3, 15, FColor::Purple, FString::Printf(TEXT("Players in lobby: %d"), GameState->PlayerArray.Num()));
	}
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
