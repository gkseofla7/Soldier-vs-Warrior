// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "CharacterMotionReplicator.h"
#include "../Weapons/Object_Master.h"

#include "SoldierMotionReplicator.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PUZZLEPLATFORMS_API USoldierMotionReplicator : public UCharacterMotionReplicator
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USoldierMotionReplicator();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_SetControllRotation(FRotator NewControlRotattor);
	UFUNCTION(Server, Reliable, WithValidation)
		void Server_SetMuzzleRotation(FRotator NewRotator);
	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void NetMulticast_SetMuzzleRotation(FRotator NewRotator);
	UFUNCTION(Server, Reliable, WithValidation)
		void Server_WeaponReload();
	UFUNCTION(Server, Reliable, WithValidation)
		void Server_WeaponReloadEnd();
	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void NetMulticast_SetIsReloading(bool bIsReloading);
	UFUNCTION(Server, Reliable, WithValidation)
		void Server_SendAttack() override;
	UFUNCTION(Server, Reliable, WithValidation)
		void Server_SendAttackStop();
	UFUNCTION(Server, Reliable, WithValidation)
		void Server_SendGetItem(class AObject_Master* NewWeapon);
	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void Multicast_SendGetItem(class AObject_Master* NewWeapon);
	UFUNCTION(Server, Reliable, WithValidation)
		void Server_RespawnPawn(APlayerController* NewController);
	UFUNCTION(Server, Reliable, WithValidation)
		void Server_SetIsAiming(bool NewIsAiming);
	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void NetMulticast_SetIsAiming(bool NewIsAiming);

	UFUNCTION(BlueprintCallable, Category = "Disable")
		void DisableActor(bool toHide);

	//UFUNCTION()
	//void OnRep_Attack();

private:
	UPROPERTY()
		class USoldierAnimInstance* MyAnim;

public:
	//UPROPERTY(ReplicatedUsing = OnRep_Attack)
	bool IsFiring = false;
	//UPROPERTY(replicated, BlueprintReadWrite, EditAnywhere)
	//	AObject_Master* PickupItem;

};
