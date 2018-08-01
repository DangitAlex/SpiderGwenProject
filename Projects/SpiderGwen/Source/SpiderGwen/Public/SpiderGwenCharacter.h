// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "WebAnchor.h"
#include "SpiderGwenCharacterMovement.h"
#include "SpiderGwenCharacter.generated.h"

UCLASS(config=Game)
class ASpiderGwenCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FollowCamera;

public:
	/** Default UObject constructor. */
	ASpiderGwenCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(BlueprintReadOnly, Transient)
		USpiderGwenCharacterMovement * SpiderMovement;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
		float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
		float BaseLookUpRate;

	virtual void BeginPlay() override;

	void JumpPressed();
	void JumpReleased();
	void OnJump();

	void RunPressed();
	void RunReleased();

	virtual void Tick(float DeltaSeconds) override;

	/** Resets HMD orientation in VR. */
	//void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	virtual void AddMovementInput(FVector WorldDirection, float ScaleValue = 1.0f, bool bForce = false) override;

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	//Jumping Vars

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Jumping")
		bool bIsChargingJump;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Jumping")
		float lastJumpChargeStartTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Jumping")
		float fullJumpChargeTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Jumping")
		float jumpIntensity_current;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Jumping")
		float jumpIntensity_chargeThreshold;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Jumping")
		float jumpIntensity_max;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Jumping")
		float jumpIntensity_speed;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Jumping")
		float jumpVelocity_ScaleMax;

	//Running Vars
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Running")
		bool bRunPressed;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Running")
		bool bIsRunning;

	// AS: Web Swinging
	UPROPERTY(BlueprintReadOnly, Category = "Web Swinging")
		bool bWebSwingPressed;

	UPROPERTY(EditAnywhere, Category = "Web Swinging")
		float WebSwing_TraceLength;

	UPROPERTY(BlueprintReadOnly, Transient, Category = "Web Swinging")
		float WebSwing_TraceSpread_Current;

	UPROPERTY(EditAnywhere, Category = "Web Swinging")
		float WebSwing_TraceSpread_MAX;

	UPROPERTY(EditAnywhere, Category = "Web Swinging")
		float WebSwing_TraceSpread_Speed;

	UPROPERTY(EditAnywhere, Category = "Web Swinging")
		float WebSwing_TraceSpreadAxisAngleOffset;

	UPROPERTY(EditAnywhere, Category = "Web Swinging")
		float WebSwing_MinRequiredValidLength;

	UPROPERTY(EditAnywhere, Category = "Web Swinging")
		float WebSwing_TraceCount;

	//UPROPERTY(BlueprintReadOnly, Transient, Category = "Web Swinging")
		//float WebSwing_WebLength;

	//UPROPERTY(BlueprintReadOnly, Transient, Category = "Web Swinging")
		//FVector WebSwing_AnchorLocation;

	UPROPERTY(BlueprintReadWrite, Transient, Category = "Web Swinging")
		AWebAnchor* WebAnchor_Primary;

	UPROPERTY(BlueprintReadWrite, Transient, Category = "Web Swinging")
		AWebAnchor* WebAnchor_Secondary;

	void WebSwingPressed();
	void WebSwingReleased();

	UFUNCTION(BlueprintPure, Category = "Web Swinging")
		bool CanWebSwing();

	UFUNCTION(BlueprintPure, Category = "Web Swinging")
		bool IsWebSwinging();

	UFUNCTION(BlueprintPure, Category = "Web Swinging")
		FVector GetWebAnchorLocation(EWebAnchorSide ForSide = EWebAnchorSide::SIDE_None) const;

	UFUNCTION(BlueprintPure, Category = "Web Swinging")
		float GetWebLength(EWebAnchorSide ForSide = EWebAnchorSide::SIDE_None) const;

	UFUNCTION(BlueprintPure, Category = "Web Swinging")
		bool CanTickWebSwing();

	void Tick_WebSwingTrace(float DeltaSeconds);
	void WebSwing_Start(FVector AnchorLoc);

	UFUNCTION(BlueprintCallable, Category = "Web Swinging")
		void WebSwing_Stop();

	UFUNCTION(BlueprintImplementableEvent, Category = "Web Swinging")
		void BP_OnWebSwingStart(FVector AnchorLoc);

	UFUNCTION(BlueprintImplementableEvent, Category = "Web Swinging")
		void BP_OnWebSwingStop();

	// AS: Web Dashing 
	UPROPERTY(EditAnywhere, Category = "Web Dash")
		float WebDash_TraceDistance;

	UPROPERTY(EditAnywhere, Category = "Web Dash")
		float WebDash_Impulse_Forward;

	UPROPERTY(EditAnywhere, Category = "Web Dash")
		float WebDash_Impulse_Up;

	UPROPERTY(EditAnywhere, Category = "Web Dash")
		float WebDash_MaxVelocity;

	void TryWebDash();

	//Camera Vars
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
		FVector cameraOffset_currTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
		FVector cameraOffset_zoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
		FVector cameraOffset_idle;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
		FVector cameraOffset_walking;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
		FVector cameraOffset_running;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
		float cameraOffset_swinging_xMin;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
		float cameraOffset_swinging_xMax;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
		FVector cameraOffset_swinging;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
		float cameraOffset_InterpSpeed;

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PrevCustomMode) override;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION(BlueprintImplementableEvent, Category = "Camera")
		FRotator BP_OverrideCameraBaseOrientation();

	virtual FVector OverrideCharacterVelocity(const FVector & InitialVelocity, const FVector & Gravity, const float & DeltaTime) override;
};

