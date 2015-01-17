// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "BattleChairs.h"
#include "BattleChairsCharacter.h"
#include "BattleChairsProjectile.h"
#include "Animation/AnimInstance.h"


//////////////////////////////////////////////////////////////////////////
// ABattleChairsCharacter




ABattleChairsCharacter::ABattleChairsCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	leftFire = false;
	rightFire = false;
	leftFireDelay = 30;
	rightFireDelay = 30;

	// Create a CameraComponent	
	FirstPersonCameraComponent = ObjectInitializer.CreateDefaultSubobject<UCameraComponent>(this, TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->AttachParent = GetCapsuleComponent();
	FirstPersonCameraComponent->RelativeLocation = FVector(0, 0, 64.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 30.0f, 10.0f);

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	Mesh1P->AttachParent = FirstPersonCameraComponent;
	Mesh1P->RelativeLocation = FVector(0.f, 0.f, -150.f);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P are set in the
	// derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

//////////////////////////////////////////////////////////////////////////
// Input

void ABattleChairsCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	// set up gameplay key bindings
	check(InputComponent);

	InputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	InputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	
	InputComponent->BindAction("Fire", IE_Pressed, this, &ABattleChairsCharacter::OnFire);
	InputComponent->BindAction("SecondaryFire", IE_Pressed, this, &ABattleChairsCharacter::RightFire);
	InputComponent->BindAction("Fire", IE_Released, this, &ABattleChairsCharacter::StopLeftFire);
	InputComponent->BindAction("SecondaryFire", IE_Released, this, &ABattleChairsCharacter::StopRightFire);

	InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &ABattleChairsCharacter::TouchStarted);

	InputComponent->BindAxis("MoveForward", this, &ABattleChairsCharacter::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &ABattleChairsCharacter::MoveRight);
	
	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	InputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	InputComponent->BindAxis("TurnRate", this, &ABattleChairsCharacter::TurnAtRate);
	InputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	InputComponent->BindAxis("LookUpRate", this, &ABattleChairsCharacter::LookUpAtRate);
}
void ABattleChairsCharacter::OnFire()
{

	// try and fire a projectile
	if (ProjectileClass != NULL)
	{
		const FRotator SpawnRotation = GetControlRotation();
		// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
		FVector offSet = FVector(0.0f, -60.0f, 0.0f);
		FRotator turn = FRotator(0.0);
		turn.Add(0.0f, 3.0f, 0.0f);

		const FVector SpawnLocation = GetActorLocation() + SpawnRotation.RotateVector(GunOffset) + SpawnRotation.RotateVector(offSet);
		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			// spawn the projectile at the muzzle
			World->SpawnActor<ABattleChairsProjectile>(ProjectileClass, SpawnLocation, SpawnRotation);
			//World->SpawnActor<AProjectileParent>(BulletClass, SpawnLocation, SpawnRotation);
		}

		ClientSetRotation(SpawnRotation - turn);
		LaunchPawn(-1000 * GetActorForwardVector(), false, false);
		leftFire = true;
	}
	/*
	// try and play the sound if specified
	if (FireSound != NULL)
	{
	UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}
	*/
	/*
	// try and play a firing animation if specified
	if(FireAnimation != NULL)
	{
	// Get the animation object for the arms mesh
	UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
	if(AnimInstance != NULL)
	{
	AnimInstance->Montage_Play(FireAnimation, 1.f);
	}
	}
	*/
}


void ABattleChairsCharacter::RightFire()
{

	// try and fire a projectile
	if (ProjectileClass != NULL)
	{
		const FRotator SpawnRotation = GetControlRotation();
		// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
		//FVector offSet = FVector(0.0f, -60.0f, 0.0f);
		FRotator turn = FRotator(0.0);
		turn.Add(0.0f, -3.0f, 0.0f);

		const FVector SpawnLocation = GetActorLocation() + SpawnRotation.RotateVector(GunOffset);
		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			// spawn the projectile at the muzzle
			World->SpawnActor<ABattleChairsProjectile>(ProjectileClass, SpawnLocation, SpawnRotation);
		}

		ClientSetRotation(SpawnRotation - turn);
		LaunchPawn(-1000 * GetActorForwardVector(), false, false);
		rightFire = true;
	}
	/*
	// try and play the sound if specified
	if (FireSound != NULL)
	{
	UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}
	*/
	/*
	// try and play a firing animation if specified
	if(FireAnimation != NULL)
	{
	// Get the animation object for the arms mesh
	UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
	if(AnimInstance != NULL)
	{
	AnimInstance->Montage_Play(FireAnimation, 1.f);
	}
	}
	*/
}


void ABattleChairsCharacter::StopLeftFire()
{
	leftFire = false;
	leftFireDelay = 30;
}

void ABattleChairsCharacter::StopRightFire()
{
	rightFire = false;
	rightFireDelay = 30;
}
void ABattleChairsCharacter::TouchStarted(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	// only fire for first finger down
	if (FingerIndex == 0)
	{
		OnFire();
	}
}

void ABattleChairsCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void ABattleChairsCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void ABattleChairsCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ABattleChairsCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ABattleChairsCharacter::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction) {
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);
	if (leftFire) {
		leftFireDelay--;
		if (leftFireDelay <= 0) {
			OnFire();
			leftFireDelay = 30;
		}
	}
	if (rightFire) {
		rightFireDelay--;
		if (rightFireDelay <= 0) {
			RightFire();
			rightFireDelay = 30;
		}
	}

}