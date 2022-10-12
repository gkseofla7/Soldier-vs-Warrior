// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CharacterMotionReplicator.h"
#include "WarriorMotionReplicator.generated.h"

//public UActorComponent, public IMotionReplicatorInterface
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PUZZLEPLATFORMS_API UWarriorMotionReplicator : public UCharacterMotionReplicator 
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWarriorMotionReplicator();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void InitializeComponent() override;
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
	UFUNCTION(BlueprintCallable, Category = "Disable")
		void DisableActor(bool toHide);

	//UFUNCTION(Server, Reliable, WithValidation)
	//	void Server_SendRide(AActor* _Car, APawn* _Rider) override;
	UFUNCTION(Server, Reliable, WithValidation)
		void Server_SendAttack() override;
	UFUNCTION(Server, Reliable, WithValidation)
		void Server_SendClimbUp();
	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void NetMulticast_SendClimbUp();
	UFUNCTION(Server, Reliable, WithValidation)
		void Server_SendDash();
	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void NetMulticast_SendDash();

	UFUNCTION()
		void OnRep_Attack();
	UPROPERTY(ReplicatedUsing = OnRep_Attack)
		bool AttackToggle = false;

	void PlaySwordAttackMontage();

private:
	UPROPERTY()
		class UPlayerAnimInstance* MyAnim;


	//����

public:
	UPROPERTY(replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = Attack, Meta = (AllowPrivateAccess = true))
		int32 CurrentCombo;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = Attack, Meta = (AllowPrivateAccess = true))
		int32 MaxCombo;





	UFUNCTION()
		void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
};
