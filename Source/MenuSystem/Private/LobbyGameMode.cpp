// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"

#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (GameState)
	{
		int32 NumPlayers = GameState->PlayerArray.Num();
		if (GEngine) GEngine->AddOnScreenDebugMessage(1, 15, FColor::Yellow, FString::Printf(TEXT("Players in game: %d"), NumPlayers));
		if (APlayerState* PlayerState =  NewPlayer->GetPlayerState<APlayerState>())
		{
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Cyan, FString::Printf(TEXT("%s joined"), *PlayerState->GetPlayerName()));
		}
	}
}

void ALobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if (GameState)
	{
		int32 NumPlayers = GameState->PlayerArray.Num();
		if (GEngine) GEngine->AddOnScreenDebugMessage(1, 15, FColor::Yellow, FString::Printf(TEXT("Players in game: %d"), NumPlayers - 1));
		if (APlayerState* PlayerState =  Exiting->GetPlayerState<APlayerState>())
		{
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Cyan, FString::Printf(TEXT("%s left"), *PlayerState->GetPlayerName()));
		}
	}
}
