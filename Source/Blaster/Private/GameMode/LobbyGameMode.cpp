// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/LobbyGameMode.h"

#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	int32 NumPlayers = GameState->PlayerArray.Num();
	if (NumPlayers == 2)
	{
		if (UWorld* World = GetWorld())
		{
			bUseSeamlessTravel = true;
			// Send all players to BlasterMap
			World->ServerTravel("/Game/Maps/BlasterMap");
		}
	}
}

void ALobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
}
