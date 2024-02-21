// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverheadWidget.generated.h"

class UTextBlock;
/**
 * Widget to display above actors
 */
UCLASS()
class BLASTER_API UOverheadWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	/// Text to display overhead
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DisplayText;

	/// Set overhead display text
	FORCEINLINE void SetDisplayText(const FString& TextToDisplay, const FColor& Color = FColor::White);

	/// Show player net role
	UFUNCTION(BlueprintCallable)
	void ShowPlayerNetRole(APawn* InPawn);

	/// Show player name
	UFUNCTION(BlueprintCallable)
	void ShowPlayerName(APawn* InPawn);

protected:
	inline static FColor SeaGreen = FColor::FromHex(TEXT("b5ffb9"));
	inline static FColor SalmonRed = FColor::FromHex(TEXT("ffb5bf"));
	
	virtual void NativeDestruct() override;

private:
	/// In multiplayer, the PlayerState may not be immediately available, so we poll up to a max time for it
	int32 PollPlayerStateCount = 0;
	UPROPERTY(EditAnywhere, Category = Configuration)
	float PollPlayerStateRateSec = 0.1;
	/// Max number of times we poll player state before giving up
	/// Time in seconds is PollPlayerStateMaxTicks * PollPlayerStateRateSec
	UPROPERTY(EditAnywhere, Category = Configuration)
	int32 PollPlayerStateMaxTicks = 60;
	FTimerHandle ShowPlayerNameHandle;
	UFUNCTION()
	void TrySetPlayerName(APawn* InPawn);
	
};
