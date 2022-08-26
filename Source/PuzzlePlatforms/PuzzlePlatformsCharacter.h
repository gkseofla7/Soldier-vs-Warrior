// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include"PuzzlePlatforms.h"
#include "GameFramework/Character.h"
//#include "PlayersComponent/MotionReplicatorInterface.h"
#include "PlayersComponent/SoldierMotionReplicator.h"
#include "PlayersComponent/MotionInterfaceComponent.h"
#include "PuzzlePlatformsCharacter.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInterruptCastingDeleagate);

UCLASS(config=Game, BlueprintType)
class APuzzlePlatformsCharacter : public ACharacter
{
protected:
	GENERATED_BODY()
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USphereComponent* NearObjectCollisionDetector;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
public:
	APuzzlePlatformsCharacter();
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;
	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;
	FRotator GetMuzzleRotation();
protected:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	void OnResetVR();
	virtual void MoveForward(float Value);
	virtual void MoveRight(float Value);
	virtual void CJump(float Value);
	virtual void CStopJumping(float Value);
	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);
	void GetInTheCar();
	void SeeMouseCursur();
	virtual void Attack();


	//UFUNCTION()
	//	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);


protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator, AActor* DamageCauser) override;

public:

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = Stat)
		class UMyCharacterStatComponent* CharacterStat;
	//UPROPERTY(VisibleAnywhere, Category = Replicator)
	//	class UPlayersMotionReplicator* MotionReplicator;
	//UPROPERTY에서 돌아가려면
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Replicator)
	//TScriptInterface<IMotionReplicatorInterface> DaerimMotionReplicator;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Replicator)
		UMotionInterfaceComponent* DaerimMotionReplicator;
		//IMotionReplicatorInterface* MotionReplicator;
	bool MouseCursorToggle = false;
	UPROPERTY(BlueprintAssignable,BlueprintCallable)
	FInterruptCastingDeleagate InterruptCasting;
protected:
	UPROPERTY()
		UAnimInstance* MyAnim;
	


};

