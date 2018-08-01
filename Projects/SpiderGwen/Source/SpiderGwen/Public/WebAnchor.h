// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WebAnchor.generated.h"

UENUM(BlueprintType)
enum class EWebAnchorSide : uint8
{
	SIDE_None		UMETA(DisplayName = "Neither Web Anchor"),
	SIDE_Primary	UMETA(DisplayName = "Primary Web Anchor"),
	SIDE_Secondary	UMETA(DisplayName = "Secondary Web Anchor"),
	SIDE_Both		UMETA(DisplayName = "Both Web Anchors"),

	SIDE_MAX		UMETA(Hidden)
};

UCLASS()
class SPIDERGWEN_API AWebAnchor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWebAnchor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Web Anchor", meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent * AnchorMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Web Anchor", meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent * WebMesh;

	UPROPERTY(BlueprintReadWrite, Transient, Category = "Web Anchor")
		EWebAnchorSide WebAnchorSide;

	UPROPERTY(BlueprintReadWrite, Transient, Category = "Web Anchor")
		float WebLength;

	UPROPERTY(BlueprintReadOnly)
		AWebAnchor * ConnectedToAnchor;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
};
