// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterTypes.h"
#include "BlasterComponents/CombatComponent.h"
#include "GameFramework/Character.h"
#include "BlasterCharacter.generated.h"

enum class ETurningInPlace : uint8;
class UCombatComponent;
class AWeapon;
class UWidgetComponent;
struct FInputActionValue;
class UInputAction;
class UInputMappingContext;
class UCameraComponent;
class USpringArmComponent;

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABlasterCharacter();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	/// Immediately perform logic after components are initialized
	virtual void PostInitializeComponents() override;
	virtual void Jump() override;

	/// Should only be called on server
	void SetOverlappingWeapon(AWeapon* Weapon);

	/// Play fire weapon animation montage
	void PlayFireMontage(bool bAiming);
	
	FORCEINLINE bool IsWeaponEquipped() { return CombatComponent && CombatComponent->EquippedWeapon; }
	FORCEINLINE AWeapon* GetEquippedWeapon() { return CombatComponent ? CombatComponent->EquippedWeapon : nullptr; }
	FORCEINLINE bool IsAiming() { return CombatComponent && CombatComponent->bAiming; }
	FORCEINLINE float GetAOYaw() { return AO_Yaw; }
	FORCEINLINE float GetAOPitch() { return AO_Pitch; }
	FORCEINLINE ETurningInPlace GetTurningInPlace() { return TurningInPlace; }

protected:
	/**
	 * Input
	 */

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputMappingContext> MappingContext;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> MoveAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> TurnAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> JumpAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> EquipAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> CrouchAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> AimAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> FireAction;

	virtual void BeginPlay() override;
	
	void Move(const FInputActionValue& Value);
	void Turn(const FInputActionValue& Value);
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void AimButtonPressed(const FInputActionValue& Value);
	/// True for Hold, False for Released
	void FireButtonPressed(const FInputActionValue& Value);

	/// Calculate aim offset parameters
	void AimOffset(float DeltaTime);

private:
	/**
	 * Components
	 */
	
	UPROPERTY(VisibleAnywhere, Category = Camera)
	TObjectPtr<USpringArmComponent> SpringArmComponent;
	UPROPERTY(VisibleAnywhere, Category = Camera)
	TObjectPtr<UCameraComponent> CameraComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = HUD, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWidgetComponent> OverheadWidget;

	UPROPERTY(VisibleAnywhere, Category = Combat)
	TObjectPtr<UCombatComponent> CombatComponent;

	/**
	 * Combat
	 */
	
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	TObjectPtr<AWeapon> OverlappingWeapon;
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	/**
	 * RPCs
	 */

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	/**
	 * Aim offset
	 */
	
	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	/**
	 * Animations for turning
	 */
	ETurningInPlace TurningInPlace{ETurningInPlace::NotTurning};
	void TurnInPlace(float DeltaTime);

	/**
	 * Animation Montages
	 */
	UPROPERTY(EditAnywhere, Category = Combat)
	TObjectPtr<UAnimMontage> FireWeaponMontage;
};
