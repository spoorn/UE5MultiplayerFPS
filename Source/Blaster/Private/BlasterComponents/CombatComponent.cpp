// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterComponents/CombatComponent.h"

#include "Character/BlasterCharacter.h"
#include "Character/CharacterTypes.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/Weapon.h"
#include "Weapon/WeaponTypes.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	// TODO: Check if this is called multiple times when underlying property of AWeapon that is NOT replicated is changed
	UE_LOG(LogTemp, Warning, TEXT("OnRep_EquippedWeapon"));
	if (Character && EquippedWeapon)
	{
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
}


void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
}

void UCombatComponent::EquipWeapon(AWeapon* Weapon)
{
	if (!Character || !Weapon) return;
	EquippedWeapon = Weapon;
	EquippedWeapon->SetWeaponState(EWeaponState::Equipped);
	if (const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(RightHandleSocketName))
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}
	// Note: owner is replicated already
	EquippedWeapon->SetOwner(Character);
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
    Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	bAiming = bIsAiming;  // Set on client immediately
	// Don't need to check authority as RPC is ran on server only, which is fine as any changes we make are replicated
	ServerSetAiming(bIsAiming);
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
}

