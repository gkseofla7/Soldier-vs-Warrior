// Fill out your copyright notice in the Description page of Project Settings.


#include "Soldier.h"
#include "Weapons/Weapon_Master.h"
#include "Weapons/FPSHudWidget.h"

#include "PlayersComponent/SoldierMotionReplicator.h"
#include "AnimInstance/SoldierAnimInstance.h"
#include "Kismet/KismetMathLibrary.h"

ASoldier::ASoldier()
{
	DaerimMotionReplicator = CreateDefaultSubobject<USoldierMotionReplicator>(TEXT("SoldierMotionReplicator"));


	static ConstructorHelpers::FClassFinder<USoldierAnimInstance> SOLDIER_ANIM((TEXT("/Game/Animation/BP_SoldierAnim")));
	if (SOLDIER_ANIM.Succeeded())
	{
		GetMesh()->SetAnimInstanceClass(SOLDIER_ANIM.Class);
	}

	ConstructorHelpers::FClassFinder<AWeapon_Master> WeaponBPClass(TEXT("/Game/Weapons/BP_Weapon_Master"));
	if (!ensure(WeaponBPClass.Class != nullptr)) return;
	WeaponMasterClass = WeaponBPClass.Class;

	FPPCam_ = CreateDefaultSubobject<UCameraComponent>(TEXT("FPPCam"));
	SpringArm_ = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	AimObejctFPP = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AimObejctFPP"));
	FPPCam_->SetupAttachment(GetMesh(),"head");
	SpringArm_->SetupAttachment(FPPCam_);
	AimObejctFPP->SetupAttachment(SpringArm_, USpringArmComponent::SocketName);
	ADSCam_ = CreateDefaultSubobject<UCameraComponent>(TEXT("ADSCam"));
	ADSCam_->SetupAttachment(GetMesh());
	IsItemEquipped = false;
}

void ASoldier::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// Set up gameplay key bindings

	PlayerInputComponent->BindAction("WeaponPrimary", IE_Pressed, this, &ASoldier::WeaponPrimaryPressed);
	PlayerInputComponent->BindAction("WeaponPrimary", IE_Released, this, &ASoldier::WeaponPrimaryReleased);

	PlayerInputComponent->BindAction("WeaponSecondary", IE_Pressed, this, &ASoldier::WeaponSecondaryPressed);
	PlayerInputComponent->BindAction("WeaponSecondary", IE_Released, this, &ASoldier::WeaponSecondaryReleased);

	PlayerInputComponent->BindAction("WeaponReload", IE_Pressed, this, &ASoldier::WeaponReload);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ASoldier::InteractPressed);
}

void ASoldier::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	MyAnim = Cast<USoldierAnimInstance>(GetMesh()->GetAnimInstance());
	ABCHECK(nullptr != MyAnim);


}

void ASoldier::BeginPlay()
{
	Super::BeginPlay();
	//{
	//	FTransform WeaponTransform;
	//	EquippedItem = GetWorld()->SpawnActor<AWeapon_Master>(WeaponMasterClass, WeaponTransform);
	//	EquippedItem->Soldier = this;
	//	FAttachmentTransformRules ItemTransformRules(EAttachmentRule::SnapToTarget, true);

	//	EquippedItem->AttachToComponent(GetMesh(), ItemTransformRules, "GripPoint");
	//	IsItemEquipped = true;
	//}

	//HudWidget = CreateWidget<UFPSHudWidget>(GetController(), FPSHudClass);
	//ABCHECK(FPSHudClass != nullptr);

	if (EquippedItem == nullptr)
	{
		IsItemEquipped = false;
	}

	
}
void ASoldier::SetFPSHudWidget()
{
	if (FPSHudClass != nullptr)
	{

		HudWidget = CreateWidget<UFPSHudWidget>(GetWorld(), FPSHudClass);
		HudWidget->AddToViewport();

	}
}

void ASoldier::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//SetMuzzleRotation();

	if (IsAiming)
	{
		AimDownSights();
	}
	else
		UnAim();
}



void ASoldier::SetMuzzleRotation()
{
	if (EquippedItem == nullptr)
		return;
	FVector Start = EquippedItem->GetSkeletalMesh()->GetSocketLocation("Muzzle");
	FVector Target = AimObejctFPP->GetComponentLocation();
	FRotator temp = UKismetMathLibrary::FindLookAtRotation(Start, Target);

	Server_SetMuzzleRotation(temp);
	Everyone_SetMuzzleRotation(temp);
}

void ASoldier::Everyone_SetMuzzleRotation_Implementation(FRotator NewRotator)
{

	EquippedItem->SetMuzzleRotation(NewRotator);
	
}
bool ASoldier::Everyone_SetMuzzleRotation_Validate(FRotator NewRotator)
{
	return true;
}

bool ASoldier::Server_SetMuzzleRotation_Validate(FRotator NewRotator)
{
	return true;
}

void ASoldier::Server_SetMuzzleRotation_Implementation(FRotator NewRotator)
{

	EquippedItem->SetMuzzleRotation(NewRotator);

}



void ASoldier::AimDownSights()
{
	FPPCam_->Deactivate();
	ADSCam_->Activate();
	FVector VCurrent = ADSCam_->GetComponentLocation();
	FVector VTarget = EquippedItem->GetCamera()->GetComponentLocation();

	FRotator RCurrent = ADSCam_->GetComponentRotation();
	FRotator RTarget = EquippedItem->GetCamera()->GetComponentRotation();

	float Deltatime = UGameplayStatics::GetWorldDeltaSeconds(GetWorld());
	FVector NextLoc = FMath::VInterpTo(VCurrent, VTarget, Deltatime, 15);
	FRotator NexRot = FMath::RInterpTo(RCurrent, RTarget, Deltatime, 15);

	ADSCam_->SetWorldLocationAndRotation(NextLoc, NexRot);

}

void ASoldier::UnAim()
{
	FVector VCurrent = ADSCam_->GetComponentLocation();
	FVector VTarget = FPPCam_->GetComponentLocation();

	FRotator RCurrent = ADSCam_->GetComponentRotation();
	FRotator RTarget = FPPCam_ ->GetComponentRotation();

	float Deltatime = UGameplayStatics::GetWorldDeltaSeconds(GetWorld());
	FVector NextLoc = FMath::VInterpTo(VCurrent, VTarget, Deltatime, 15);
	FRotator NexRot = FMath::RInterpTo(RCurrent, RTarget, Deltatime, 15);

	ADSCam_->SetWorldLocationAndRotation(NextLoc, NexRot);

	if (FPPCam_->GetComponentLocation().Equals(ADSCam_->GetComponentLocation(), 0.01))
	{
		ADSCam_->Deactivate();
		FPPCam_->Activate();
	}
}

void ASoldier::WeaponPrimaryPressed()
{

	//SetMuzzleRotation();
	//EquippedItem->StartFire();
	//Cast<USoldierMotionReplicator>(DaerimMotionReplicator)->IsFiring = true;
	
	//IsFiring = true;
	//Cast<USoldierMotionReplicator>(DaerimMotionReplicator)->Server_SendAttack();
}

void ASoldier::Attack()
{
	if (EquippedItem == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("EquippedItem null"));
		return;
	}
		SetMuzzleRotation();
	Super::Attack();

}

void ASoldier::WeaponPrimaryReleased()
{

	if (EquippedItem == nullptr)
		return;
	Cast<USoldierMotionReplicator>(DaerimMotionReplicator)->Server_SendAttackStop();

}

void ASoldier::WeaponSecondaryPressed()
{
	if (CanAim == false)
		return;
	if (EquippedItem != nullptr)
		IsAiming = true;

}
void ASoldier::WeaponSecondaryReleased()
{

	if (EquippedItem != nullptr)
		IsAiming = false;
}
void ASoldier::WeaponReload()
{
	bool IsFiring__ = Cast<USoldierMotionReplicator>(DaerimMotionReplicator)->IsFiring;
	if (EquippedItem->CanReload == true && IsFiring__ == false && IsReloading == false && IsItemEquipped == true)
	{
		IsReloading = true;
		EquippedItem->Reload();
		FTimerHandle WaitHandle;
		float WaitTime =EquippedItem->ReloadDelay; //시간을 설정하고
		GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]()
			{
				// 여기에 코드를 치면 된다.
				IsReloading = false;
			}), WaitTime, false); //반복도 여기서 추가 변수를 선언해 설정가능
	}
}

void ASoldier::EquipItem(AObject_Master* Item, bool EquipAndHold)
{
	if (EquipAndHold == false)
	{
		auto weapon = Cast<AWeapon_Master>(Item);
		if (weapon != nullptr)
		{
			EquippedItem = weapon;
			PrimaryWeapon = weapon;
			EquippedItem->Soldier = this;
			EquippedItem->GetSkeletalMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			EquippedItem->AttachToSoldier(this ,"GripPoint");
			IsItemEquipped = true;
			bUseControllerRotationYaw= true;
			//자연스럽게 원하는 방향으로 회전
			GetCharacterMovement()->bOrientRotationToMovement = false;
			// 자동적으로 캐릭터의 이동방향을 움직이는 방향에 맞춰주며 회전보간을 해줌
			SetFPSHudWidget();
			WeaponSlotUse = EWeaponSlot::TE_PrimaryWeapon;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Wiered Things happen"))
		auto weapon = Cast<AWeapon_Master>(Item);
		if (weapon != nullptr)
		{
			SecondaryWeapon = weapon;
			SecondaryWeapon->Soldier = this;
			SecondaryWeapon->GetSkeletalMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			SecondaryWeapon->AttachToSoldier(this, "RifleHolster");
			
		}
	}
}

void ASoldier::InteractPressed()
{

	Cast<USoldierMotionReplicator>(DaerimMotionReplicator)->Server_SendGetItem(PickupItem);
	//if (DoPickupLinetrace)
	//{
	//	if (PickupItem != nullptr)
	//	{

	//		EquipItem(PickupItem, IsItemEquipped);

	//		FTimerHandle WaitHandle;
	//		float WaitTime = 1.73; //시간을 설정하고
	//		GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]()
	//			{
	//				CanAim = true;
	//				// 여기에 코드를 치면 된다.
	//			}), WaitTime, false); //반복도 여기서 추가 변수를 선언해 설정가능
	//		
	//	}
	//}
}