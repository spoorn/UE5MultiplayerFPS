// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BlasterCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Asset/AssetMacros.h"
#include "BlasterComponents/CombatComponent.h"
#include "Camera/CameraComponent.h"
#include "Character/CharacterTypes.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "HUD/OverheadWidget.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/Weapon.h"

ABlasterCharacter::ABlasterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Default mesh
	LOAD_ASSET_TO_CALLBACK(USkeletalMesh, "/Game/Assets/LearningKit_Games/Assets/Characters/Character/Mesh/SK_EpicCharacter", GetMesh()->SetSkeletalMeshAsset);
	GetMesh()->SetRelativeLocation(FVector(0, 0, -88));
	GetMesh()->SetRelativeRotation(FRotator(0, -90, 0));

	// Camera
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComponent->SetupAttachment(GetMesh());
	SpringArmComponent->TargetArmLength = 350;
	SpringArmComponent->SetRelativeRotation(FRotator(-30, 90, 0));
	// Bring back up to Z=0
	SpringArmComponent->SetRelativeLocation(FVector(0, 0, 88));
	// Socket offset center point to right of character
	SpringArmComponent->SocketOffset = FVector(0, 75, 0);
	SpringArmComponent->bUsePawnControlRotation = true;
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false;

	// Enable controller rotation on character to follow camera movement
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = true;

	// Orient character to movement rotation
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0, 800, 0);
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
	GetCharacterMovement()->SetCrouchedHalfHeight(60);
	GetCharacterMovement()->MaxWalkSpeedCrouched = 350;
	GetCharacterMovement()->JumpZVelocity = 1600;
	GetCharacterMovement()->GravityScale = 4;
	
	// Overhead widget
	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(GetRootComponent());
	OverheadWidget->SetWidgetSpace(EWidgetSpace::Screen);
	OverheadWidget->SetDrawAtDesiredSize(true);
	OverheadWidget->SetRelativeLocation(FVector(0, 0, 150));
	LOAD_ASSET_CLASS_TO_CALLBACK(UUserWidget, "/Game/Blueprints/HUD/WBP_OverheadWidget", OverheadWidget->SetWidgetClass);

	// Combat
	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	CombatComponent->SetIsReplicated(true);

	// ignore camera
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	// Input
	LOAD_ASSET_TO_VARIABLE(UInputMappingContext, "/Game/Input/IMC_Blaster", MappingContext);
	LOAD_ASSET_TO_VARIABLE(UInputAction, "/Game/Input/Actions/IA_Jump", JumpAction);
	LOAD_ASSET_TO_VARIABLE(UInputAction, "/Game/Input/Actions/IA_Move", MoveAction);
	LOAD_ASSET_TO_VARIABLE(UInputAction, "/Game/Input/Actions/IA_Turn", TurnAction);
	LOAD_ASSET_TO_VARIABLE(UInputAction, "/Game/Input/Actions/IA_Equip", EquipAction);
	LOAD_ASSET_TO_VARIABLE(UInputAction, "/Game/Input/Actions/IA_Crouch", CrouchAction);
	LOAD_ASSET_TO_VARIABLE(UInputAction, "/Game/Input/Actions/IA_Aim", AimAction);
	LOAD_ASSET_TO_VARIABLE(UInputAction, "/Game/Input/Actions/IA_Fire", FireAction);

	// Animation montages
	LOAD_ASSET_TO_VARIABLE(UAnimMontage, "/Game/Blueprints/Character/Animations/AM_FireWeapon", FireWeaponMontage);

	// Net
	MinNetUpdateFrequency = 33;
	NetUpdateFrequency = 66;
}

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	AimOffset(DeltaTime);
}

void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);
		EnhancedInputComponent->BindAction(TurnAction, ETriggerEvent::Triggered, this, &ThisClass::Turn);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ThisClass::Jump);
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Triggered, this, &ThisClass::EquipButtonPressed);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &ThisClass::CrouchButtonPressed);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered, this, &ThisClass::AimButtonPressed);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &ThisClass::FireButtonPressed);
	}
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (CombatComponent)
	{
		CombatComponent->Character = this;
	}
}

void ABlasterCharacter::Jump()
{
	if (bIsCrouched)
	{
		UnCrouch();
	} else
	{
		Super::Jump();
	}
}

void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(false);
		}
		if (Weapon)
		{
			Weapon->ShowPickupWidget(true);
		}
	}
	OverlappingWeapon = Weapon;
}

void ABlasterCharacter::PlayFireMontage(bool bAiming)
{
	if (!CombatComponent || !CombatComponent->EquippedWeapon || !FireWeaponMontage) return;
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(MappingContext, 0);
		}
	}

	// Widget class needs to be set in BP
	if (OverheadWidget)
	{
		if (UOverheadWidget* OverheadWidgetObj = Cast<UOverheadWidget>(OverheadWidget->GetWidget()))
		{
			// By default, show the player name
			OverheadWidgetObj->ShowPlayerName(this);
		}
	}
}

void ABlasterCharacter::Move(const FInputActionValue& Value)
{
	if (const FVector2D MoveValue = Value.Get<FVector2D>(); GetController() && (MoveValue.X != 0 || MoveValue.Y != 0))
	{
		// Only care about yaw rotation as we only go parallel to bottom plane
		const FRotator YawRotation{0, GetControlRotation().Yaw, 0};

		// Get forward direction of controller
		// take unit vector, rotate it according to YawRotation, and extract the forward X direction
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(ForwardDirection, MoveValue.Y);

		// In UE5, Y axis is right direction
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(RightDirection, MoveValue.X);
	}
}

void ABlasterCharacter::Turn(const FInputActionValue& Value)
{
	if (const FVector2D DirectionValue = Value.Get<FVector2D>(); GetController() && (DirectionValue.Y != 0 || DirectionValue.X != 0))
	{
		AddControllerYawInput(DirectionValue.X);
		AddControllerPitchInput(DirectionValue.Y);
	}
}

void ABlasterCharacter::EquipButtonPressed()
{
	if (CombatComponent)
	{
		if (HasAuthority())
		{
			CombatComponent->EquipWeapon(OverlappingWeapon);
		} else
		{
			// By calling the server fn here instead of in CombatComponent, we don't need to send
			// Weapon class over RPC for better network efficiency, but it does make the code a little messier
			ServerEquipButtonPressed();
		}
	}
}

void ABlasterCharacter::CrouchButtonPressed()
{
	if (bIsCrouched)
	{
		UnCrouch();
	} else
	{
		// Crouch is already replicated
		Crouch();
	}
}

void ABlasterCharacter::AimButtonPressed(const FInputActionValue& Value)
{
	if (CombatComponent)
	{
		// Pressed = false, Released = true
		CombatComponent->SetAiming(Value.Get<bool>());
	}
}

void ABlasterCharacter::FireButtonPressed(const FInputActionValue& Value)
{
	if (CombatComponent)
	{
		// Pressed = false, Released = true
		CombatComponent->FireButtonPressed(Value.Get<bool>());
	}
}

void ABlasterCharacter::AimOffset(float DeltaTime)
{
	if (!CombatComponent || !CombatComponent->EquippedWeapon)
	{
		// TODO: optimize
		StartingAimRotation = FRotator(0, GetBaseAimRotation().Yaw, 0);
		return;
	};
	float Speed = GetVelocity().Size2D();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	// Only set aim offset for Yaw when standing idle
	if (Speed == 0 && !bIsInAir)
	{
		// Standing idle
		//AO_Yaw = FMath::GetMappedRangeValueClamped(FVector2d{0, 360}, FVector2d{10, 20}, GetBaseAimRotation().Yaw);
		
		FRotator CurrentAimRotation = FRotator(0, GetBaseAimRotation().Yaw, 0);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}
	if (Speed > 0 || bIsInAir)
	{
		// Running or jumping, reset movement to follow controller
		StartingAimRotation = FRotator(0, GetBaseAimRotation().Yaw, 0);
		AO_Yaw = 0;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::NotTurning;
	}

	// Aim offset pitch can change even when jumping
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90 && !IsLocallyControlled())
	{
		// map pitch from [270, 360) to [-90, 0)
		AO_Pitch = FMath::GetMappedRangeValueClamped(FVector2D{270, 360}, FVector2D{-90, 0}, AO_Pitch);
	}
}

void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90)
	{
		TurningInPlace = ETurningInPlace::Right;
	} else if (AO_Yaw < -90)
	{
		TurningInPlace = ETurningInPlace::Left;
	}
	
	if (TurningInPlace != ETurningInPlace::NotTurning)
	{
		// Interp towards 0 rotation. Root bone will rotate interpolated towards AO_Yaw
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0, DeltaTime, 6);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 5)
		{
			TurningInPlace = ETurningInPlace::NotTurning;
			StartingAimRotation = FRotator(0, GetBaseAimRotation().Yaw, 0);
		}
	}
}

void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	EquipButtonPressed();
}


