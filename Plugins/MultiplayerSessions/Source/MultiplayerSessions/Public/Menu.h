// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Menu.generated.h"

class UMultiplayerSessionsSubsystem;
class UButton;
/**
 * Simple menu for multiplayer sessions
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void MenuSetup();

protected:
	virtual bool Initialize() override;

private:
	TObjectPtr<UMultiplayerSessionsSubsystem> MultiplayerSessionsSubsystem;
	
	/// Bind to HostButton
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> HostButton;

	/// Bind to JoinButton
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> JoinButton;

	/**
	 * Button callbacks
	 */
	
	UFUNCTION()
	void HostButtonClicked();
	UFUNCTION()
	void JoinButtonClicked();
};
