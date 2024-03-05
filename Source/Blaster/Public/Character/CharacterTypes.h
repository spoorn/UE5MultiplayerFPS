#pragma once

/**
 * Sockets
 */

inline static FName RightHandSocketName("RightHandSocket");
// Left Hand socket on weapons held by characters
inline static FName LeftHandSocketName("LeftHandSocket");
// Right hand bone
inline static FName RightHandBoneName("hand_r");

UENUM(BlueprintType)
enum class ETurningInPlace : uint8
{
    Left UMETA(DisplayName = "Turning Left"),
	Right UMETA(DisplayName = "Turning Right"),
	NotTurning UMETA(DisplayName = "Not Turning"),
	MAX
};