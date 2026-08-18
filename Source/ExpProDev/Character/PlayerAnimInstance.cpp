// No Rights Reserved @ Team Expedition 

#include "Character/PlayerAnimInstance.h"
#include "PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	PlayerCharacter = Cast<APlayerCharacter>(TryGetPawnOwner());
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (PlayerCharacter == nullptr)
	{
		PlayerCharacter = Cast<APlayerCharacter>(TryGetPawnOwner());
	}
	if (PlayerCharacter == nullptr) return;

	FVector Velocity = PlayerCharacter->GetVelocity();
	Velocity.Z = 0.f; // We don't care for the verticle speed up and down. 
	Speed = Velocity.Size();

	bIsInAir = PlayerCharacter->GetCharacterMovement()->IsFalling();

	bIsAccelerating = PlayerCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;

	bWeaponEquipped = PlayerCharacter->IsWeaponEquipped();

	// TECH_DEBT(TD-STUB-1): this flag is plumbed through to the anim BP but no crouch animations,
	// capsule resize or speed change exist — see APlayerCharacter::CrouchButtonPressed.
	bIsCrouched = PlayerCharacter->bIsCrouched;

	bAiming = PlayerCharacter->IsAiming();

	AO_Yaw = PlayerCharacter->GetAO_Yaw();
}
