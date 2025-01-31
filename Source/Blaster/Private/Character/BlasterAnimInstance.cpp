// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BlasterAnimInstance.h"

#include "Character/BlasterCharacter.h"
#include "Character/CharacterTypes.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapon/Weapon.h"

void UBlasterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
}

void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (!BlasterCharacter)
	{
		BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
	}
	if (!BlasterCharacter) return;
	
	Speed = BlasterCharacter->GetVelocity().Size2D();
	bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0;
	bWeaponEquipped = BlasterCharacter->IsWeaponEquipped();
	EquippedWeapon = BlasterCharacter->GetEquippedWeapon();
	bIsCrouched = BlasterCharacter->bIsCrouched;
	bAiming = BlasterCharacter->IsAiming();
	TurningInPlace = BlasterCharacter->GetTurningInPlace();
	
	// Global rotation of camera aim
	const FRotator AimRotation = BlasterCharacter->GetBaseAimRotation();
	// Global rotation of movement
	const FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(BlasterCharacter->GetVelocity());
	// yaw difference between movement and aim for strafing
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	// Smoothly interpolates, even wrapping around -180 to 180 to prevent sudden jumping through the blend space
	// Better than using smoothing time in blend space to avoid that wrap around issue
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaSeconds, 6);
	YawOffset = DeltaRotation.Yaw;

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = BlasterCharacter->GetActorRotation();
	// Get rotational change in Yaw
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaSeconds;
	// Interpolate the yaw change for lean
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaSeconds, 6);
	Lean = FMath::Clamp(Interp, -90, 90);

	// Aim offset
	AO_Yaw = BlasterCharacter->GetAOYaw();
	AO_Pitch = BlasterCharacter->GetAOPitch();

	// Left Hand IK
	if (bWeaponEquipped && EquippedWeapon && BlasterCharacter->GetMesh())
	{
		if (USkeletalMeshComponent* WeaponMesh = EquippedWeapon->GetWeaponMesh())
		{
			LeftHandTransform = WeaponMesh->GetSocketTransform(LeftHandSocketName, RTS_World);
			FVector OutPosition;
			FRotator OutRotation;
			// transform to be relative to right hand bone
			BlasterCharacter->GetMesh()->TransformToBoneSpace(RightHandBoneName, LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
			LeftHandTransform.SetLocation(OutPosition);
			LeftHandTransform.SetRotation(FQuat(OutRotation));
		}
	}
}
