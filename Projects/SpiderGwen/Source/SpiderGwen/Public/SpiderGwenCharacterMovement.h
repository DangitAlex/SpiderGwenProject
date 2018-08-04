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

	UPROPERTY(EditAnywhere, Category = "Web Swing")
		float WebSwingVelocity_MAX;

	UPROPERTY(EditAnywhere, Category = "Web Swing")
		float InputForceSwingFrictionSpeed;

protected:
	virtual void InitializeComponent() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	virtual void PostLoad() override;

public:
	UFUNCTION(Category = "Hit Delegates")
		void OnOwningCharCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION(BluepriuntPure, Category = "Velocity")
		float GetVelocityRatio() const;


	virtual void PhysFalling(float deltaTime, int32 Iterations) override;
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;

	void PhysWebSwinging(float DeltaTime, int32 Iterations);

	void SimulateWebTension();
};
