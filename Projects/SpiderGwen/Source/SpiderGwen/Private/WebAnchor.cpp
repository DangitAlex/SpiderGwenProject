// Fill out your copyright notice in the Description page of Project Settings.

#include "WebAnchor.h"
#include "SpiderGwenCharacter.h"

// Sets default values
AWebAnchor::AWebAnchor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AnchorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AnchorMesh"));
	WebMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WebMesh"));
}

// Called when the game starts or when spawned
void AWebAnchor::BeginPlay()
{
	Super::BeginPlay();

	if (Instigator)
	{
		ASpiderGwenCharacter * OwningSpider = Cast<ASpiderGwenCharacter>(Instigator);

		if (!OwningSpider)
			return;

		WebLength = (GetActorLocation() - OwningSpider->GetActorLocation()).Size();
	}
}

// Called every frame
void AWebAnchor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
