// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 80000


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
	
	/// [Server] Equips weapon locally, caller has to handle splitting between client/server
	void EquipWeapon(AWeapon* Weapon);

	/// Set character to aiming stance
	void SetAiming(bool bIsAiming);

	void FireButtonPressed(bool bPressed);
	
protected:
	virtual void BeginPlay() override;

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

private:
	/// Character base walk speed, overridden by this combat component
	UPROPERTY(EditAnywhere, Category = Movement)
	float BaseWalkSpeed{600};
	/// Walk speed while aiming
	UPROPERTY(EditAnywhere, Category = Movement)
	float AimWalkSpeed{400};
	
	/// Back reference to owning character
	TObjectPtr<ABlasterCharacter> Character;
	/// Equipped weapon on actor
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	TObjectPtr<AWeapon> EquippedWeapon;
	UFUNCTION()
	void OnRep_EquippedWeapon();

	/// Is aiming down sights
	UPROPERTY(Replicated)
	bool bAiming;
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	/// Fire weapon, client -> server -> multicast
	bool bFireButtonPressed;
	UFUNCTION(Server, Reliable)
	void ServerFire();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire();
};
