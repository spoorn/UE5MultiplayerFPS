// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"


class AWeapon;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();
	// This component is tightly coupled with BlasterCharacter
	friend class ABlasterCharacter;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void EquipWeapon(AWeapon* Weapon);
	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);
	
protected:
	virtual void BeginPlay() override;

private:
	/// Back reference to owning character
	TObjectPtr<ABlasterCharacter> Character;
	/// Equipped weapon on actor
	UPROPERTY(Replicated)
	TObjectPtr<AWeapon> EquippedWeapon;

	/// Is aiming down sights
	UPROPERTY(Replicated)
	bool bAiming;
	
};
