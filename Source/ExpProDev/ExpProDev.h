// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/** Main log category used across the project */
// TECH_DEBT(TD-DEAD-12): "used across the project" is not true — the only three UE_LOG calls using
// this category are in template/variant code. Every real gameplay class logs to LogTemp, so the
// game's output can't be filtered or given a verbosity level of its own.
DECLARE_LOG_CATEGORY_EXTERN(LogExpProDev, Log, All);

// TECH_DEBT(TD-ARCH-11): a preprocessor macro standing in for a collision channel. It bypasses the
// type safety of the ECollisionChannel enum and must be kept manually in sync with
// DefaultChannelResponses in Config/DefaultEngine.ini — nothing catches a mismatch.
// TECH_DEBT(TD-ARCH-12): DefaultEngine.ini also declares a "Projectile" channel
// (ECC_GameTraceChannel1) that no C++ ever references; AProjectile blocks against Visibility,
// WorldStatic and SkeletalMesh instead of its own channel.
#define ECC_SkeletalMesh ECollisionChannel(ECC_GameTraceChannel2)