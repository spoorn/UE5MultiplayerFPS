// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
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
	void MenuSetup(int32 NumberOfPublicConnections = 4, FString TypeOfMatch = FString("FreeForAll"));

protected:
	virtual bool Initialize() override;
	virtual void NativeDestruct() override;

	/**
	 * Callbacks for MultiplayerSessionsSubsystem
	 */
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);
	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
	void OnJoinSession(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	UFUNCTION()
	void OnDestroySession(FName SessionName, bool bWasSuccessful);
	UFUNCTION()
	void OnStartSession(FName SessionName, bool bWasSuccessful);

private:
	TObjectPtr<UMultiplayerSessionsSubsystem> MultiplayerSessionsSubsystem;
	int32 NumPublicConnections;
	FString MatchType;
	
	/// Bind to HostButton
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> HostButton;

	/// Bind to JoinButton
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> JoinButton;

	void MenuTearDown();

	/**
	 * Button callbacks
	 */
	
	UFUNCTION()
	void HostButtonClicked();
	UFUNCTION()
	void JoinButtonClicked();
};
