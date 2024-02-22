// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BlasterCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Asset/AssetMacros.h"
#include "BlasterComponents/CombatComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "HUD/OverheadWidget.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/Weapon.h"

ABlasterCharacter::ABlasterCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// Default mesh
	LOAD_ASSET_TO_CALLBACK(USkeletalMesh, "/Game/Assets/LearningKit_Games/Assets/Characters/Character/Mesh/SK_EpicCharacter", GetMesh()->SetSkeletalMeshAsset);
	GetMesh()->SetRelativeLocation(FVector(0, 0, -88));
	GetMesh()->SetRelativeRotation(FRotator(0, -90, 0));

	// Camera
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComponent->SetupAttachment(GetMesh());
	SpringArmComponent->TargetArmLength = 600;
	SpringArmComponent->SetRelativeRotation(FRotator(-30, 90, 0));
	// Bring back up to Z=0
	SpringArmComponent->SetRelativeLocation(FVector(0, 0, 88));
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
	GetCharacterMovement()->RotationRate = FRotator(0, 400, 0);

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

	// Input
	LOAD_ASSET_TO_VARIABLE(UInputMappingContext, "/Game/Input/IMC_Blaster", MappingContext);
	LOAD_ASSET_TO_VARIABLE(UInputAction, "/Game/Input/Actions/IA_Jump", JumpAction);
	LOAD_ASSET_TO_VARIABLE(UInputAction, "/Game/Input/Actions/IA_Move", MoveAction);
	LOAD_ASSET_TO_VARIABLE(UInputAction, "/Game/Input/Actions/IA_Turn", TurnAction);
	LOAD_ASSET_TO_VARIABLE(UInputAction, "/Game/Input/Actions/IA_Equip", EquipAction);
}

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
	if (CombatComponent && HasAuthority())
	{
		CombatComponent->EquipWeapon(OverlappingWeapon);
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


