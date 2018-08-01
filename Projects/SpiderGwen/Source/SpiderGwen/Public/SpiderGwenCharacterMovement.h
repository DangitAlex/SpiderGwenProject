#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SpiderGwenCharacterMovement.generated.h"

/**
 * 
 */
UCLASS()
class SPIDERGWEN_API USpiderGwenCharacterMovement : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	USpiderGwenCharacterMovement(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void InitializeComponent() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	virtual void PostLoad() override;

public:
	UFUNCTION(Category = "Hit Delegates")
		void OnOwningCharCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

	virtual void PhysFalling(float deltaTime, int32 Iterations) override;
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;

	void PhysWebSwinging(float DeltaTime, int32 Iterations);

	void SimulateWebTension();
};
