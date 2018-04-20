// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "SpiderGwenCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"

//////////////////////////////////////////////////////////////////////////
// ASpiderGwenCharacter

ASpiderGwenCharacter::ASpiderGwenCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	lastJumpChargeStartTime = 0.f;
	fullJumpChargeTime = 1.5f;
	jumpIntensity_current = 0.f;
	jumpIntensity_chargeThreshold = 0.15f;
	jumpIntensity_max = 1.f;
	jumpIntensity_speed = 1.f;
	jumpVelocity_ScaleMax = 5.f;

	bRunPressed = false;

	bWebSwingPressed = false;

	cameraOffset_currTarget = FVector(0.f);
	cameraOffset_zoom = FVector(125.f, 60.f, 50.f);
	cameraOffset_idle = FVector(-25.f, 0.f, 60.f);
	cameraOffset_walking = FVector(-50.f, 0.f, 75.f);
	cameraOffset_running = FVector(-100.f, 0.f, 100.f);
	cameraOffset_swinging_xMin = -250.f;
	cameraOffset_swinging_xMax = -550.f;
	cameraOffset_swinging = FVector(cameraOffset_swinging_xMin, 0.f, 150.f);

	cameraOffset_InterpSpeed = 5.f;
}

//////////////////////////////////////////////////////////////////////////
// Input

void ASpiderGwenCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASpiderGwenCharacter::JumpPressed);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ASpiderGwenCharacter::JumpReleased);

	PlayerInputComponent->BindAction("Run", IE_Pressed, this, &ASpiderGwenCharacter::RunPressed);
	PlayerInputComponent->BindAction("Run", IE_Released, this, &ASpiderGwenCharacter::RunReleased);

	PlayerInputComponent->BindAction("WebSwing", IE_Pressed, this, &ASpiderGwenCharacter::WebSwingPressed);
	PlayerInputComponent->BindAction("WebSwing", IE_Released, this, &ASpiderGwenCharacter::WebSwingReleased);

	PlayerInputComponent->BindAction("WebDash", IE_Pressed, this, &ASpiderGwenCharacter::TryWebDash);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASpiderGwenCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASpiderGwenCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ASpiderGwenCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ASpiderGwenCharacter::LookUpAtRate);

	// handle touch devices
	//PlayerInputComponent->BindTouch(IE_Pressed, this, &ASpiderGwenCharacter::TouchStarted);
	//PlayerInputComponent->BindTouch(IE_Released, this, &ASpiderGwenCharacter::TouchStopped);

	// VR headset functionality
	//PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ASpiderGwenCharacter::OnResetVR);
}

void ASpiderGwenCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

//void ASpiderGwenCharacter::OnResetVR()
//{
//	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
//}
//
//void ASpiderGwenCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
//{
//		Jump();
//}
//
//void ASpiderGwenCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
//{
//		StopJumping();
//}

void ASpiderGwenCharacter::JumpPressed()
{
	jumpIntensity_current = 0.f;

	bIsChargingJump = true;
	lastJumpChargeStartTime = GetGameTimeSinceCreation();
}

void ASpiderGwenCharacter::JumpReleased()
{
	jumpIntensity_current = FMath::Clamp(((GetGameTimeSinceCreation() - lastJumpChargeStartTime) / fullJumpChargeTime), 0.f, 1.f);

	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("Jump Released :: " + FString::SanitizeFloat(jumpIntensity_current)));

	if (bIsChargingJump && CanJump())
		OnJump();

	bIsChargingJump = false;
	jumpIntensity_current = 0.f;
}

void ASpiderGwenCharacter::OnJump()
{
	float currJumpVelocityZ = 0.f;
	float currJumpVelocityX = 0.f;

	if (jumpIntensity_current >= jumpIntensity_chargeThreshold)
	{
		currJumpVelocityZ = GetCharacterMovement()->JumpZVelocity * (1.f + (jumpIntensity_current * (jumpVelocity_ScaleMax - 1.f)));

		if (bRunPressed)
			currJumpVelocityX = 2500.f;
	}
	else
		currJumpVelocityZ = GetCharacterMovement()->JumpZVelocity;

	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow, TEXT("Jump Z Vel :: " + FString::SanitizeFloat(currJumpVelocityZ)));

	FVector finalJumpImpulse = (GetActorUpVector() * currJumpVelocityZ) + (GetActorForwardVector() * currJumpVelocityX);

	GetCharacterMovement()->AddImpulse(finalJumpImpulse, true);
}

void ASpiderGwenCharacter::RunPressed()
{
	if (!bRunPressed)
	{
		bRunPressed = true;
		GetCharacterMovement()->MaxWalkSpeed = 3000.f;
	}
}

void ASpiderGwenCharacter::RunReleased()
{
	bRunPressed = false;
	GetCharacterMovement()->MaxWalkSpeed = 600.f;
}

void ASpiderGwenCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ASpiderGwenCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ASpiderGwenCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ASpiderGwenCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void ASpiderGwenCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PrevCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PrevCustomMode);
}
