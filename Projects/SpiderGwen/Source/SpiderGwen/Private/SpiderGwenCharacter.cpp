// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "SpiderGwenCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
//#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "DrawDebugHelpers.h"

//////////////////////////////////////////////////////////////////////////
// ASpiderGwenCharacter


ASpiderGwenCharacter::ASpiderGwenCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<USpiderGwenCharacterMovement>(ACharacter::CharacterMovementComponentName))
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

	SpiderMovement = Cast<USpiderGwenCharacterMovement>(CharacterMovement);

	// Configure character movement
	SpiderMovement->bOrientRotationToMovement = true; // Character moves in the direction of input...
	SpiderMovement->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	SpiderMovement->JumpZVelocity = 600.f;
	SpiderMovement->AirControl = 0.2f;

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

	WebSwing_TraceLength = 10000.f;
	WebSwing_TraceSpread_MAX = 40.f;
	WebSwing_TraceSpread_Speed = 30.f;
	WebSwing_TraceSpreadAxisAngleOffset_MIN = 25.f;
	WebSwing_TraceSpreadAxisAngleOffset_MAX = 50.f;
	WebSwing_TraceCount = 30;
	WebSwing_MinRequiredValidLength = 1000.f;
	WebSwing_InputForce_MAX = 150000.f;

	WebDash_TraceDistance = 10000.f;
	WebDash_Impulse_Forward = 6000.f;
	WebDash_Impulse_Up = 2000.f;
	WebDash_MaxVelocity = 4000.f;
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
}

void ASpiderGwenCharacter::BeginPlay()
{
	Super::BeginPlay();

	if(SpiderMovement->IsA(USpiderGwenCharacterMovement::StaticClass()))
		CapsuleComponent->OnComponentHit.AddDynamic(((USpiderGwenCharacterMovement*)SpiderMovement), &USpiderGwenCharacterMovement::OnOwningCharCapsuleHit);
}

void ASpiderGwenCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (CanTickWebSwing())
	{
		Tick_WebSwingTrace(DeltaSeconds);
	}
	else if (IsWebSwinging())
	{
		if (!CanWebSwing())
			WebSwing_Stop();
	}

	if (bIsChargingJump)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, -1.f, (FMath::Lerp(FLinearColor::White, FLinearColor::Yellow, GetJumpChargeRatio())).ToFColor(true), FString::Printf(TEXT("Jump Charge: %f"), GetJumpChargeRatio()));
	}
}

bool ASpiderGwenCharacter::CanTickWebSwing()
{
	return CanWebSwing(true) && bWebSwingPressed && !IsWebSwinging();
}

void ASpiderGwenCharacter::Tick_WebSwingTrace(float DeltaSeconds)
{
	WebSwing_TraceSpreadAxisAngleOffset_Current = FMath::Lerp(WebSwing_TraceSpreadAxisAngleOffset_MIN, WebSwing_TraceSpreadAxisAngleOffset_MAX, SpiderMovement->GetVelocityRatio());

	if (WebSwing_TraceSpread_Current > WebSwing_TraceSpread_MAX)
	{
		WebSwing_TraceSpread_Current = WebSwing_TraceSpread_MAX;
	}
	else
	{
		for (int i = 0; i < WebSwing_TraceCount; i++)
		{
			float currAngle = (float)i * (360.f / (float)WebSwing_TraceCount);

			FRotator offsetRot = FRotator(0.f, GetControlRotation().Yaw, 0.f);
			FVector CurrStart = GetActorLocation();
			FVector StartNormal = FVector::UpVector.RotateAngleAxis(WebSwing_TraceSpreadAxisAngleOffset_MAX, (FRotationMatrix(offsetRot).GetScaledAxis(EAxis::Y)));
			FVector RotAxis = (FRotationMatrix(offsetRot).GetScaledAxis(EAxis::Y)).RotateAngleAxis(currAngle, StartNormal);
			FVector SpreadNormal = StartNormal.RotateAngleAxis(WebSwing_TraceSpread_Current, RotAxis);
			FVector CurrEnd = CurrStart + (SpreadNormal * WebSwing_TraceLength);

			FHitResult hit;
			bool bResult = GetWorld()->LineTraceSingleByChannel(hit, CurrStart, CurrEnd, ECC_Visibility);
			if (bResult)
			{
				float distToAnchor = (hit.ImpactPoint - GetActorLocation()).SizeSquared();

				if(distToAnchor >= FMath::Square(WebSwing_MinRequiredValidLength))
					WebSwing_Start(hit.ImpactPoint);
			}

			//DrawDebugLine(GetWorld(), CurrStart, (CurrStart + RotAxis * 50.f), FColor::Orange, false, -1.f, 0, 1.f);
			DrawDebugLine(GetWorld(), CurrStart, CurrEnd, bResult ? FColor::Green : FColor::Purple, false, -1.f, 0, 3.f);
		}

		WebSwing_TraceSpread_Current = FMath::FInterpConstantTo(WebSwing_TraceSpread_Current, WebSwing_TraceSpread_MAX, DeltaSeconds, WebSwing_TraceSpread_Speed);
	}
}

void ASpiderGwenCharacter::JumpPressed()
{
	jumpIntensity_current = 0.f;

	bIsChargingJump = true;
	lastJumpChargeStartTime = GetGameTimeSinceCreation();
}

void ASpiderGwenCharacter::JumpReleased()
{
	jumpIntensity_current = GetJumpChargeRatio();

	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("Jump Released :: " + FString::SanitizeFloat(jumpIntensity_current)));

	if (bIsChargingJump)
	{
		if (CanJump() || IsWebSwinging())
		{
			OnJump();
		}
	}

	bIsChargingJump = false;
	jumpIntensity_current = 0.f;
}

void ASpiderGwenCharacter::OnJump()
{
	float currJumpVelocityZ = 0.f;
	float currJumpVelocityX = 0.f;

	if (GetJumpChargeRatio() >= jumpIntensity_chargeThreshold)
	{
		if (IsWebSwinging())
		{
			currJumpVelocityZ = GetJumpChargeRatio() * (FMath::Max(FVector::DotProduct(SpiderMovement->Velocity, FVector::UpVector), 0.f) *  2.f);
			currJumpVelocityX = GetJumpChargeRatio() * (FVector::DotProduct(SpiderMovement->Velocity, GetActorForwardVector()) * 2.f);

			WebSwing_Stop();
		}
		else
		{
			currJumpVelocityZ = SpiderMovement->JumpZVelocity * (1.f + (GetJumpChargeRatio() * (jumpVelocity_ScaleMax - 1.f)));

			if (bRunPressed)
				currJumpVelocityX = 2500.f;
		}
	}
	else
		currJumpVelocityZ = SpiderMovement->JumpZVelocity;

	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Orange, TEXT("Jump Z Vel :: " + FString::SanitizeFloat(currJumpVelocityZ)));

	FVector finalJumpImpulse = (GetActorUpVector() * currJumpVelocityZ) + (GetActorForwardVector() * currJumpVelocityX);

	SpiderMovement->AddImpulse(finalJumpImpulse, true);
}

float ASpiderGwenCharacter::GetJumpChargeRatio()
{
	return FMath::Clamp(((GetGameTimeSinceCreation() - lastJumpChargeStartTime) / fullJumpChargeTime), 0.f, 1.f);
}

void ASpiderGwenCharacter::RunPressed()
{
	if (!bRunPressed)
	{
		bRunPressed = true;
		SpiderMovement->MaxWalkSpeed = 3000.f;
	}
}

void ASpiderGwenCharacter::RunReleased()
{
	bRunPressed = false;
	SpiderMovement->MaxWalkSpeed = 600.f;
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
	if (Controller != NULL && Value != 0.f)
	{
		if (IsWebSwinging())
		{
			// find out which way is forward
			const FRotator Rotation = Controller->GetControlRotation();

			// get forward vector, projected onto the plane created by the dir from this pawn to the anchor
			const FVector Direction = (FVector::VectorPlaneProject(FRotationMatrix(Rotation).GetUnitAxis(EAxis::X), (GetWebAnchorLocation() - GetActorLocation()).GetSafeNormal())).GetSafeNormal();

			// add movement in that direction
			AddMovementInput(Direction, Value);
		}
		else
		{
			// find out which way is forward
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);

			// get forward vector
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

			// add movement in that direction
			AddMovementInput(Direction, Value);
		}
	}

	LastMovementInput.X = Value;

	if (LastMovementInput == FVector2D::ZeroVector)
		WebSwing_InputForce_Current = FVector::ZeroVector;
}

void ASpiderGwenCharacter::MoveRight(float Value)
{
	if (Controller != NULL && Value != 0.f)
	{
		if (IsWebSwinging())
		{
			// find out which way is forward
			const FRotator Rotation = Controller->GetControlRotation();
			//const FRotator YawRotation(0, Rotation.Yaw, 0);

			// get forward vector, projected onto the plane created by the dir from this pawn to the anchor
			const FVector Direction = (FVector::VectorPlaneProject(FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y), (GetWebAnchorLocation() - GetActorLocation()).GetSafeNormal())).GetSafeNormal();

			// add movement in that direction
			AddMovementInput(Direction, Value);
		}
		else
		{
			// find out which way is forward
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);

			// get forward vector
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

			// add movement in that direction
			AddMovementInput(Direction, Value);
		}
	}

	LastMovementInput.Y = Value;

	if (LastMovementInput == FVector2D::ZeroVector)
		WebSwing_InputForce_Current = FVector::ZeroVector;
}

void ASpiderGwenCharacter::AddMovementInput(FVector WorldDirection, float ScaleValue, bool bForce /*=false*/)
{
	if (IsWebSwinging())
	{
		WebSwing_InputForce_Current = WorldDirection * ScaleValue * WebSwing_InputForce_MAX;
		DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + WebSwing_InputForce_Current, FColor::Yellow, false, -1.f, 0, 10.f);
	}
	else
	{
		Super::AddMovementInput(WorldDirection, ScaleValue, bForce);
	}
}

void ASpiderGwenCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PrevCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PrevCustomMode);

	switch (SpiderMovement->MovementMode)
	{
	case EMovementMode::MOVE_Walking:
		if(PrevMovementMode == EMovementMode::MOVE_Falling && PrevCustomMode == 1)
			WebSwing_Stop();
		break;
	case EMovementMode::MOVE_Falling:
		break;
	}
}

FVector ASpiderGwenCharacter::OverrideCharacterVelocity(const FVector & InitialVelocity, const FVector & Gravity, const float & DeltaTime)
{
	return InitialVelocity;
}

// AS: Web Swinging ==========================================================================================
bool ASpiderGwenCharacter::CanWebSwing(bool bToStart /*= false*/)
{
	return SpiderMovement->IsFalling() && (bToStart ? !IsWebSwinging() : SpiderMovement->CustomMovementMode == 1);
}

bool ASpiderGwenCharacter::IsWebSwinging()
{
  	return bIsWebSwinging;
}

FVector ASpiderGwenCharacter::GetWebAnchorLocation(EWebAnchorSide ForSide /*= EWebAnchorSide::SIDE_None*/) const
{
	if (ForSide > EWebAnchorSide::SIDE_None && ForSide <= EWebAnchorSide::SIDE_Both)
	{
		switch (ForSide)
		{
		case EWebAnchorSide::SIDE_Primary:
			if (WebAnchor_Primary)
				return WebAnchor_Primary->GetActorLocation();
			break;
		case EWebAnchorSide::SIDE_Secondary:
			if (WebAnchor_Secondary)
				return WebAnchor_Secondary->GetActorLocation();
		case EWebAnchorSide::SIDE_Both:
			if (WebAnchor_Primary && WebAnchor_Secondary)
				return ((WebAnchor_Primary->GetActorLocation() + WebAnchor_Secondary->GetActorLocation()) * 0.5f);
			break;
		}
	}
	else if (WebAnchor_Primary != NULL || WebAnchor_Secondary != NULL)
	{
		int validAnchorCount = 0;
		FVector retVal = FVector::ZeroVector;

		if (WebAnchor_Primary)
		{
			retVal += WebAnchor_Primary->GetActorLocation();
			validAnchorCount++;
		}

		if (WebAnchor_Secondary)
		{
			retVal += WebAnchor_Secondary->GetActorLocation();
			validAnchorCount++;
		}

		if (validAnchorCount > 0)
			return (retVal / (float)validAnchorCount);
	}

	return FVector::ZeroVector;
}

float ASpiderGwenCharacter::GetWebLength(EWebAnchorSide ForSide /*= EWebAnchorSide::SIDE_None*/) const
{
	if (ForSide > EWebAnchorSide::SIDE_None && ForSide < EWebAnchorSide::SIDE_Both)
	{
		switch (ForSide)
		{

		case EWebAnchorSide::SIDE_Primary:
			if (WebAnchor_Primary)
				return WebAnchor_Primary->WebLength;
			break;
		case EWebAnchorSide::SIDE_Secondary:
			if (WebAnchor_Secondary)
				return WebAnchor_Secondary->WebLength;
			break;
		}
	}
	else if (WebAnchor_Primary != NULL || WebAnchor_Secondary != NULL)
	{
		if (WebAnchor_Primary)
		{
			return WebAnchor_Primary->WebLength;
		}

		if (WebAnchor_Secondary)
		{
			return WebAnchor_Secondary->WebLength;
		}
	}

	return -1.f;
}

void ASpiderGwenCharacter::WebSwingPressed()
{
	bWebSwingPressed = true;
	WebSwing_TraceSpread_Current = 0.f;
}

void ASpiderGwenCharacter::WebSwingReleased()
{
	bWebSwingPressed = false;

	if (IsWebSwinging())
		WebSwing_Stop();
}

void ASpiderGwenCharacter::WebSwing_Start(FVector AnchorLoc)
{
	bIsWebSwinging = true;

	SpiderMovement->SetMovementMode(EMovementMode::MOVE_Falling);
	SpiderMovement->CustomMovementMode = 1;
	SpiderMovement->GroundFriction = 0.f;
	SpiderMovement->FallingLateralFriction = 0.f;
	SpiderMovement->AirControl = 0.f;

	SpiderMovement->bOrientRotationToMovement = false;

	FTransform spawnTransform = FTransform();
	spawnTransform.SetTranslation(AnchorLoc);
	spawnTransform.SetScale3D(FVector::OneVector);

	FActorSpawnParameters spawnParams;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	spawnParams.Instigator = this;

	BP_OnWebSwingStart(AnchorLoc);
}

void ASpiderGwenCharacter::WebSwing_Stop()
{
	bIsWebSwinging = false;

	if(SpiderMovement->MovementMode == EMovementMode::MOVE_Custom)
		SpiderMovement->SetMovementMode(EMovementMode::MOVE_Falling);

	SpiderMovement->CustomMovementMode = 0;
	SpiderMovement->bOrientRotationToMovement = true;
	SpiderMovement->GroundFriction = GetDefault<USpiderGwenCharacterMovement>()->GroundFriction;
	SpiderMovement->FallingLateralFriction = GetDefault<USpiderGwenCharacterMovement>()->FallingLateralFriction;
	SpiderMovement->AirControl = GetDefault<USpiderGwenCharacterMovement>()->AirControl;
	ResetCharacterRotation(false, true);

	BP_OnWebSwingStop();
}

// AS: Web Dashing ===========================================================================================
void ASpiderGwenCharacter::TryWebDash()
{
	FHitResult hit;
	if (GetWorld()->LineTraceSingleByChannel(hit, FollowCamera->GetComponentLocation(), FollowCamera->GetComponentLocation() + FollowCamera->GetForwardVector() * WebDash_TraceDistance, ECC_Visibility))
	{
		//SpiderMovement->AddImpulse(((hit.ImpactPoint - GetActorLocation()).GetSafeNormal() * WebDash_Impulse_Forward) + FVector(0.f, 0.f, WebDash_Impulse_Up), true);

		SpiderMovement->SetMovementMode(MOVE_Falling);
		SpiderMovement->Velocity += (((hit.ImpactPoint - GetActorLocation()).GetSafeNormal() * WebDash_Impulse_Forward) + FVector(0.f, 0.f, WebDash_Impulse_Up));
		SpiderMovement->Velocity = SpiderMovement->Velocity.GetClampedToMaxSize(WebDash_MaxVelocity);

		DrawDebugLine(GetWorld(), FollowCamera->GetComponentLocation(), hit.ImpactPoint, FColor::Emerald, false, 5.f, 0, 5.f);
	}
	else
	{
		DrawDebugLine(GetWorld(), FollowCamera->GetComponentLocation(), FollowCamera->GetComponentLocation() + FollowCamera->GetForwardVector() * WebDash_TraceDistance, FColor::Black, false, 5.f, 0, 5.f);
	}
}
