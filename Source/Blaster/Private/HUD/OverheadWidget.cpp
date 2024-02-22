// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/OverheadWidget.h"

#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"

void UOverheadWidget::SetDisplayText(const FString& TextToDisplay, const FColor& Color)
{
	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(TextToDisplay));
		DisplayText->SetColorAndOpacity(FSlateColor(Color));
	}
}

void UOverheadWidget::ShowPlayerNetRole(APawn* InPawn)
{
	const ENetRole LocalRole = InPawn->GetLocalRole();
	FString Role;
	switch (LocalRole) {
	case ROLE_Authority:
		Role = FString("Authority");
		break;
	case ROLE_AutonomousProxy:
		Role = FString("Autonomous Proxy");
		break;
	case ROLE_SimulatedProxy:
		Role = FString("Simulated Proxy");
		break;
	default:
		Role = FString("None");
		break;
	}
	const FString LocalRoleString = FString::Printf(TEXT("Local Role: %s"), *Role);
	SetDisplayText(LocalRoleString);
}

void UOverheadWidget::ShowPlayerName(APawn* InPawn)
{
	const FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &ThisClass::TrySetPlayerName, InPawn);
	GetWorld()->GetTimerManager().SetTimer(ShowPlayerNameHandle, TimerDelegate, PollPlayerStateRateSec, true);
}

void UOverheadWidget::NativeDestruct()
{
	RemoveFromParent();
	Super::NativeDestruct();
}

void UOverheadWidget::TrySetPlayerName(APawn* InPawn)
{
	if (++PollPlayerStateCount > PollPlayerStateMaxTicks)
	{
		PollPlayerStateCount = 0;
		GetWorld()->GetTimerManager().ClearTimer(ShowPlayerNameHandle);
	}
	if (const APlayerState* PlayerState = InPawn->GetPlayerState())
	{
		const FColor Color = GetOwningLocalPlayer()->GetPlayerController(GetWorld()) == InPawn->GetController() ? SeaGreen : SalmonRed;
		// TODO: show all player names, doing this for security initially
		if (GetOwningLocalPlayer()->GetPlayerController(GetWorld()) == InPawn->GetController())
		{
			SetDisplayText(PlayerState->GetPlayerName(), Color);
		} else
		{
			SetDisplayText(FString("Enemy"), Color);
		}
		PollPlayerStateCount = 0;
		GetWorld()->GetTimerManager().ClearTimer(ShowPlayerNameHandle);
	}
}
