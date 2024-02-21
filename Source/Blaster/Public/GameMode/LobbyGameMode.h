// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LobbyGameMode.generated.h"

/**
 * Lobby Game Mode which tracks players in lobby
 */
UCLASS()
class BLASTER_API ALobbyGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

private:
	/// Countdown delay to start game
	int8 StartGameCountDown = 10;
	
	/// Timer to send players to game map
	FTimerHandle StartGameTimer;
	UFUNCTION()
	void StartGame();
};
