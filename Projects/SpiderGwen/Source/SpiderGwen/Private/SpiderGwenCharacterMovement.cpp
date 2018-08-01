// Fill out your copyright notice in the Description page of Project Settings.

#include "SpiderGwenCharacterMovement.h"
#include "EngineGlobals.h"
#include <Runtime/Engine/Classes/Engine/Engine.h>
#include "DrawDebugHelpers.h"
#include "SpiderGwenCharacter.h"

USpiderGwenCharacterMovement::USpiderGwenCharacterMovement(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AirControl = 1.f;
}

void USpiderGwenCharacterMovement::InitializeComponent()
{
	Super::InitializeComponent();
}

void USpiderGwenCharacterMovement::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void USpiderGwenCharacterMovement::PostLoad()
{
	Super::PostLoad();	
}

void USpiderGwenCharacterMovement::OnOwningCharCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(0, 5.f, FColor::Yellow, FString::Printf(TEXT("CAPSULE HIT :: %s"), *Hit.ImpactNormal.ToString()));

	ASpiderGwenCharacter * SpiderOwner = (ASpiderGwenCharacter*)PawnOwner;

	if (!SpiderOwner)
		return;

	if (SpiderOwner->IsWebSwinging())
	{
		float hitDot = FVector::DotProduct(-Hit.ImpactNormal, Velocity.GetSafeNormal());

		DrawDebugLine(GetWorld(), Hit.ImpactPoint, Hit.ImpactPoint + (Hit.ImpactNormal * 300.f), hitDot > 0.f ? FColor::White : FColor::Orange, false, 10.f, 0, 10.f);

		if (hitDot > 0.f)
		{
			float VelocityDot = FVector::DotProduct(-Hit.ImpactNormal, Velocity);
			FVector BounceNormal = -(Velocity.GetSafeNormal().RotateAngleAxis(180.f, Hit.ImpactNormal));

			DrawDebugLine(GetWorld(), Hit.ImpactPoint, Hit.ImpactPoint + (BounceNormal * 500.f), FColor::Magenta, false, 10.f, 0, 15.f);

			//Velocity = Velocity.ProjectOnToNormal(Hit.ImpactNormal);

			// AS: Add bouncing logic, reflect some of the velocity away from the impacted surface
			//Velocity += BounceNormal * (VelocityDot * hitDot);
			
			//SimulateWebTension();
		}
	}
}

void USpiderGwenCharacterMovement::PhysFalling(float deltaTime, int32 Iterations)
{
	Super::PhysFalling(deltaTime, Iterations);

	if (((ASpiderGwenCharacter*)CharacterOwner)->IsWebSwinging())
	{
		PhysWebSwinging(deltaTime, Iterations);
	}
}

void USpiderGwenCharacterMovement::PhysCustom(float deltaTime, int32 Iterations)
{
	Super::PhysCustom(deltaTime, Iterations);

	if (CharacterOwner)
	{
		switch (CustomMovementMode)
		{
		case 0: // AS: Wall Crawling
			break;
		case 1: // AS: Web Hanging?
			break;
		case 2: // AS: Tightrope?
			break;
		}
	}
}

void USpiderGwenCharacterMovement::PhysWebSwinging(float DeltaTime, int32 Iterations)
{
	ASpiderGwenCharacter * SpiderOwner = (ASpiderGwenCharacter*)PawnOwner;

	if (!SpiderOwner)
		return;

	if ((SpiderOwner->GetWebAnchorLocation() - GetActorLocation()).SizeSquared() >= FMath::Square(SpiderOwner->GetWebLength()))
	{
		SimulateWebTension();

		// AS: Simulate centripetal force
		AddForce((SpiderOwner->GetWebAnchorLocation() - GetActorLocation()).GetSafeNormal() * ((Mass * Velocity.Size2D()) / SpiderOwner->GetWebLength()));
	}

	DrawDebugLine(GetWorld(), GetActorLocation(), SpiderOwner->GetWebAnchorLocation(), FColor::Green, false, -1.f, 0, 10.f);
}

void USpiderGwenCharacterMovement::SimulateWebTension()
{
	ASpiderGwenCharacter * SpiderOwner = (ASpiderGwenCharacter*)PawnOwner;

	if (!SpiderOwner || ((SpiderOwner->GetWebAnchorLocation() - GetActorLocation()).SizeSquared() < FMath::Square(SpiderOwner->GetWebLength())))
		return;
	
	FVector DirToAnchor = (SpiderOwner->GetWebAnchorLocation() - GetActorLocation()).GetSafeNormal();

	// AS: Cancel out velocity away from the anchor
	FVector cancelVelocity = DirToAnchor * FVector::DotProduct(Velocity, -DirToAnchor);
	Velocity += cancelVelocity;

	if ((SpiderOwner->GetWebAnchorLocation() - GetActorLocation()).SizeSquared() > FMath::Square(SpiderOwner->GetWebLength()))
	{
		SpiderOwner->SetActorLocation(SpiderOwner->GetWebAnchorLocation() + (-DirToAnchor * SpiderOwner->GetWebLength()), false, NULL, ETeleportType::TeleportPhysics);
	}
}
