// Fill out your copyright notice in the Description page of Project Settings.


#include "IggyScorchAnimInstance.h"
#include "NPC_Boss.h"

UIggyScorchAnimInstance::UIggyScorchAnimInstance()
	: Super()
{
	static ConstructorHelpers::FObjectFinder<UAnimMontage> AttackMontageAsset(TEXT(
		"/Game/Animation/IggyScorch/Montagne/IggyScorchAttackMontage"
	));
	if (AttackMontageAsset.Succeeded())
	{
		AttackMontage = AttackMontageAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> FireBlastMontageAsset(TEXT(
		"/Game/Animation/IggyScorch/Montagne/IggyScorch_FireMontage"
	));
	if (FireBlastMontageAsset.Succeeded())
	{
		FireBlastMontage = FireBlastMontageAsset.Object;
	}

	StartRotator = FRotator(0, 0, 0);
	EndRotator = FRotator(0, 0, -30);

}

void UIggyScorchAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if(bMateor == true)
	{
		CurrentTimeForMeteor += DeltaSeconds;

		float LerpRatio = CurrentTimeForMeteor / TotalTimeForMeteor;

		if (LerpRatio > 1)
			LerpRatio = 1;
		if (bMateorStart == true)
			AimRotation = FQuat::Slerp(StartRotator.Quaternion(), EndRotator.Quaternion(), LerpRatio).Rotator();
		else
			AimRotation = FQuat::Slerp(EndRotator.Quaternion(), StartRotator.Quaternion(), LerpRatio).Rotator();
	
	}
}

void UIggyScorchAnimInstance::PlayAttackMontage()
{
	Montage_Play(AttackMontage, 1.0);
	FullBody = false;
}

void UIggyScorchAnimInstance::PlayFireBlastMontage()
{

	Montage_Play(FireBlastMontage, 1.0);
	FullBody = false;
}

void UIggyScorchAnimInstance::PlayMateorMontage()
{
	UE_LOG(LogTemp, Warning, TEXT("PlayMateor"));
	PlayFireBlastMontage();
	CurrentTimeForMeteor = 0;
	bMateor = true;
	bMateorStart = true;
}

void UIggyScorchAnimInstance::AnimNotify_Shot()
{
	auto BossRef = Cast<ANPC_Boss>(GetOwningActor());
	ABCHECK(BossRef != nullptr);
	if (BossRef->HasAuthority())
	{
		OnShotDelegate.Broadcast();
	}
	
}


void UIggyScorchAnimInstance::AnimNotify_FireStart()
{
	auto BossRef = Cast<ANPC_Boss>(GetOwningActor());
	ABCHECK(BossRef != nullptr);
	if (BossRef->HasAuthority())
	{
		BossRef->ActivateParticle(true);
	}
}

void UIggyScorchAnimInstance::AnimNotify_FireEnd()
{
	auto BossRef = Cast<ANPC_Boss>(GetOwningActor());
	CurrentTimeForMeteor = 0;
	bMateorStart = false;
	ABCHECK(BossRef != nullptr);
	if (BossRef->HasAuthority())
	{
		BossRef->ActivateParticle(false);
	}
}