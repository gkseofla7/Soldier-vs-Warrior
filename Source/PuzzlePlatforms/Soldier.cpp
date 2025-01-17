// Fill out your copyright notice in the Description page of Project Settings.


#include "Soldier.h"
#include "Weapons/Weapon_Master.h"
#include "Weapons/FPSHudWidget.h"
#include "Missile//Missile.h"
#include "MyPlayerController.h"
#include "PuzzlePlatformsGameMode.h"
#include "UI/FPSTargetWidget.h"
#include "PlayersComponent/ControlRotationReplicator.h"
#include "Missile/TargetMarker.h"
#include "Missile/TargetableComponent.h"



#include "PlayersComponent/SoldierMotionReplicator.h"
#include "AnimInstance/SoldierAnimInstance.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/DecalComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Kismet/GameplayStaticsTypes.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/GameStateBase.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"

#include "Net/UnrealNetwork.h"

ASoldier::ASoldier()
{
	bReplicates = true;
#pragma region AssetSettings
	//Asset Settings
	static ConstructorHelpers::FClassFinder<AMissile> MissileBPClass((TEXT("/Game/RocketPath/BP_Missile")));
	if (MissileBPClass.Succeeded())	MissileClass = MissileBPClass.Class;

	static ConstructorHelpers::FClassFinder<ATargetMarker> TargetMarkerBPClass((TEXT("/Game/TargetMissiles/BP_TargetMarker")));
	if (TargetMarkerBPClass.Succeeded())	TargetMarkerClass = TargetMarkerBPClass.Class;
	//static ConstructorHelpers::FClassFinder<AMissile> GunBPClass((TEXT("/Game/Weapons/BP_Weapon_Master")));
	//if (GunBPClass.Succeeded())	GunClass = GunBPClass.Class;
	static ConstructorHelpers::FClassFinder<USoldierAnimInstance> SOLDIER_ANIM((TEXT("/Game/Animation/BP_SoldierAnim")));
	if (SOLDIER_ANIM.Succeeded())	GetMesh()->SetAnimInstanceClass(SOLDIER_ANIM.Class);
	static ConstructorHelpers::FObjectFinder<UStaticMesh>SplineStaticMeshAsset(TEXT("/Game/StarterContent/Shapes/Shape_Cylinder"));
	SplineStaticMesh = SplineStaticMeshAsset.Object;
	static ConstructorHelpers::FObjectFinder<UMaterial>SplineStaticMaterialAsset(TEXT("/Game/RocketPath/M_Spline_White"));
	SplineStaticMaterial = SplineStaticMaterialAsset.Object;
	ConstructorHelpers::FClassFinder<AWeapon_Master> WeaponBPClass(TEXT("/Game/Weapons/BP_Weapon_Master"));
	if (!ensure(WeaponBPClass.Class != nullptr)) return;
	WeaponMasterClass = WeaponBPClass.Class;
#pragma endregion AssetSettings
	//ComponentSetting
#pragma region ComponentSetting
	ReplicateComponent = CreateDefaultSubobject<USoldierMotionReplicator>(TEXT("SoldierMotionReplicator"));
	RocketHolderComponent = CreateDefaultSubobject< UStaticMeshComponent>(TEXT("RocketHolderComponent"));
	RocketHolderComponent->SetupAttachment(GetMesh(), "clavicle_rSocket");
	MissileComponent = CreateDefaultSubobject< UStaticMeshComponent>(TEXT("MissileComponent"));
	MissileComponent->SetupAttachment(RocketHolderComponent, "Missile");
	GridSphere = CreateDefaultSubobject< UStaticMeshComponent>(TEXT("GridSphere"));
	GridSphere->SetupAttachment(RootComponent);
	DecalMissileComponent = CreateDefaultSubobject< UDecalComponent>(TEXT("DecalMissileComponent"));
	DecalMissileComponent->SetupAttachment(GridSphere);	
	DecalMissileComponent->SetVisibility(false);
	SplinePathComponent = CreateDefaultSubobject< USplineComponent>(TEXT("SplinePathComponent"));
	SplinePathComponent->SetupAttachment(RocketHolderComponent);
	ADSCam_ = CreateDefaultSubobject<UCameraComponent>(TEXT("ADSCam"));
	ADSCam_->SetupAttachment(GetMesh());
	ControlRotationReplicator = CreateDefaultSubobject< UControlRotationReplicator>(TEXT("ControlRotationReplicator"));

#pragma endregion ComponentSetting
	//변수 초기화
	GeneralCameraPosition = FVector(0, 90, 90);
	MissileCameraPosition = FVector(0, 90, 200);
	CameraBoom->SetRelativeLocation(GeneralCameraPosition);
	IsItemEquipped = false;
	static ConstructorHelpers::FClassFinder<UUserWidget> FPSHudBPClass(TEXT("/Game/Weapons/UI/BP_FPS_Hud"));
	if (FPSHudBPClass.Succeeded())
	{
		FPSHudClass = FPSHudBPClass.Class;

	}
	TeamNum = 1;


}

//void ASoldier::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
//{
//	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
//	//DOREPLIFETIME(ASoldier, ControlRotation);
//
//}

void ASoldier::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	MyAnim = Cast<USoldierAnimInstance>(GetMesh()->GetAnimInstance());
	ABCHECK(nullptr != MyAnim);



}

void ASoldier::BeginPlay()
{
	Super::BeginPlay();
	MyAnim = Cast<UAnimInstance_Master>(GetMesh()->GetAnimInstance());
	FActorSpawnParameters SpawnParams;
	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	
}

void ASoldier::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// Set up gameplay key bindings
	PlayerInputComponent->BindAxis("MoveForward", this, &ACharacter_Master::MoveForward);
	PlayerInputComponent->BindAction("WeaponPrimary", IE_Released, this, &ASoldier::WeaponPrimaryReleased);
	PlayerInputComponent->BindAction("WeaponSecondary", IE_Pressed, this, &ASoldier::WeaponSecondaryPressed);
	PlayerInputComponent->BindAction("WeaponSecondary", IE_Released, this, &ASoldier::WeaponSecondaryReleased);
	PlayerInputComponent->BindAction("WeaponReload", IE_Pressed, this, &ASoldier::WeaponReload);
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ASoldier::InteractPressed);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ASoldier::Sprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ASoldier::UnSprint);
}

void ASoldier::SimulateControllerRotation(FControlRotation NewControlRotation)
{
	//아 애초에 없구나.. 컨트롤러가
	AddControllerPitchInput(NewControlRotation.Pitch);
	AddControllerYawInput(NewControlRotation.Yaw);

	
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

	if (CrosshairWidget != nullptr)
	{
		if (IsShooting == true)
		{
			CurrentSpread = UKismetMathLibrary::FClamp(CurrentSpread + 5.f, 5.0, 40.f);
			CrosshairWidget->crosshair_spread = CurrentSpread;
		}
		if (IsShooting == false)
		{
			CurrentSpread = UKismetMathLibrary::FClamp(CurrentSpread - 5.f, 5.0, 40.f);
			CrosshairWidget->crosshair_spread = CurrentSpread;
		}
	}
	//if (IsLocallyControlled() && IsPlayerControlled())
	//{
	//	DrawDebugBox(GetWorld(), GetActorLocation(), FVector(100, 100, 100), FColor::Purple, true, -1, 0, 10);
	//}


	if (EquippedItem!=nullptr )//&&IsLocallyControlled() && IsPlayerControlled()
	{
		if (GetLocalRole() == ROLE_AutonomousProxy)//일단 서버 아니고 자기꺼 있을때 자기꺼 움직이고 서버한테 정보보냄
		{
			//UE_LOG(LogTemp, Warning, TEXT("Rotation %f"), GetController()->GetControlRotation().Pitch);
			LastControlRotation = CreateControlRotation(DeltaTime);
			SimulateRotationAnimation(GetControlRotation());
		}
		//We are the server and in control of the pawn
		if (GetLocalRole() == ROLE_Authority && IsLocallyControlled() && IsPlayerControlled())//서버고 자기꺼일때 
		{
			LastControlRotation = CreateControlRotation(DeltaTime);
			SimulateRotationAnimation(GetControlRotation());
		}

	}
	if (IsAiming)
	{
		AimDownSights();
	}
	else
		UnAim();
	WearItem();
#pragma region SettingMissile
	if (ShowPath == true)
	{
		if (MissileTargetArmLength != CameraBoom->TargetArmLength)
			AimMissile();
		ClearPointsArray();

		auto RocketMouthTransform = RocketHolderComponent->GetSocketTransform("Mouth");
		auto ForwardVector = RocketMouthTransform.GetRotation().GetForwardVector();
		MissileVelocity = (ForwardVector + Direction * ForwardVector) * RocketSpeed;
	

		TArray< TEnumAsByte< EObjectTypeQuery > > ObjectTypes;
		ObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery1);
		FPredictProjectilePathParams PredictParam;
		//FPredictProjectilePathResult PredictResult;
		TArray<AActor*> Ignores;
		Ignores.Add(this);
		{
			//PredictParam.StartLocation = RocketMouthTransform.GetLocation();
			//PredictParam.LaunchVelocity = MissileVelocity;
			//PredictParam.ProjectileRadius = 20.;
			//PredictParam.bTraceWithCollision = true;
			//PredictParam.ObjectTypes = ObjectTypes;
			//PredictParam.SimFrequency = 15;
			////PredictParam.DrawDebugType = EDrawDebugTrace::ForDuration;
			////PredictParam.DrawDebugTime = .1;
			//PredictParam.MaxSimTime = 10;
			//PredictParam.ActorsToIgnore = Ignores;
			//PredictParam.OverrideGravityZ = 0.;
		}
		//결국 시작값, 이동등을 정해서 위치 경로 예측
		FHitResult OutHit;
		TArray< FVector > OutPathPositions;
		FVector OutLastTraceDestination;
		UGameplayStatics::PredictProjectilePath(GetWorld(), OutHit, OutPathPositions, OutLastTraceDestination, RocketMouthTransform.GetLocation(), MissileVelocity,
			true, 20., ObjectTypes, false, Ignores, EDrawDebugTrace::None, 0, 15, 10, 0);
			
		//UGameplayStatics::PredictProjectilePath(GetWorld(), PredictParam, PredictResult);
	
		for (int i = 0; i < OutPathPositions.Num(); i++)
		{
			SplinePathComponent->AddSplinePointAtIndex(OutPathPositions[i], i, ESplineCoordinateSpace::World);//예상경로등록
		}
		SplinePathComponent->SetSplinePointType(OutPathPositions.Num() - 1, ESplinePointType::CurveClamped);//예상경로 마지막 부분에 마지막부분이라고 알려줌

		for (int i = 0; i < SplinePathComponent->GetNumberOfSplinePoints() - 2; i++)
		{

			AddSplineMeshComponent(OutPathPositions[i], SplinePathComponent->GetTangentAtSplinePoint(i, ESplineCoordinateSpace::World),
				OutPathPositions[i + 1], SplinePathComponent->GetTangentAtSplinePoint(i + 1, ESplineCoordinateSpace::World));
		}


		auto NextLocation = UKismetMathLibrary::VInterpTo(OutLastTraceDestination, GridSphere->GetComponentLocation(), DeltaTime, 10);
		GridSphere->SetWorldLocation(NextLocation);
	}
	else if (GeneralTargetArmLength != CameraBoom->TargetArmLength)
	{
		UnAimMissile();
	}



#pragma endregion SettingMissile

	if (IsLocallyControlled()&& ShowTarget == true)
	{
		
		auto Target = FindBestTarget();
		SetCurrentTarget(Target);
	}
	else if (IsLocallyControlled() && ShowTarget == false)
	{
		EndTarget();
	}

}

void ASoldier::SimulateRotationAnimation(FRotator NewRotator)
{

	float A = 360.0 - NewRotator.Pitch;

	float B = NewRotator.Pitch;
	float tmp = 0;
	if (B >= 180)
	{
		tmp = A / 3;
	}
	else
	{
		tmp = B * (-1) / 3;
	}

	ControlRotation = FRotator(0, 0, tmp);;
	//Cast< USoldierMotionReplicator>(ReplicateComponent)->Server_SetControllRotation(ControlRotation);
}

struct FControlRotation ASoldier::CreateControlRotation(float DeltaTime)
{

	FControlRotation Rotation;
	Rotation.DeltaTime = DeltaTime;
	Rotation.Pitch = Pitch;
	Rotation.Yaw = Yaw;
	Rotation.Time = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
	return Rotation;
}

void ASoldier::AddControllerPitchInput(float Val)
{
	if (bIsDashing == false)
	{
		//FRotator bf = GetController()->GetControlRotation();
		//UE_LOG(LogTemp, Warning, TEXT("Before Rotation %f"),bf.Pitch);
		//UE_LOG(LogTemp, Warning, TEXT("Add : %f"), Val);
		Super::AddControllerPitchInput(Val);
		Pitch = Val;
		//UE_LOG(LogTemp, Warning, TEXT("After Rotation %f"), GetController()->GetControlRotation().Pitch);
		if (ShowPath == true)
		{
			float NewDirection = 0;
			NewDirection = UKismetMathLibrary::FInterpTo(Direction, Direction - Val, UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), 5);
			Direction = NewDirection;
		}
	}
}


void ASoldier::SetMuzzleRotation()
{
	if (EquippedItem == nullptr)
		return;
	
	UCameraComponent* CurrentCam = FollowCamera;
	if (IsAiming)
		CurrentCam = ADSCam_;

	const float WeaponRange = 20000.f;
	const FVector StartTrace = CurrentCam->GetComponentLocation();
	 FVector EndTrace = (CurrentCam->GetForwardVector() * WeaponRange) + StartTrace;
	FVector Start = EquippedItem->GetSkeletalMesh()->GetSocketLocation("Muzzle");
	//FVector Target = AimObejctFPP->GetComponentLocation();
	FHitResult Hit;
	FCollisionQueryParams QueryParams = FCollisionQueryParams(SCENE_QUERY_STAT(WeaponTrace), false, this);
	if (GetWorld()->LineTraceSingleByChannel(Hit, StartTrace, EndTrace, ECC_Visibility, QueryParams))
	{
		EndTrace = Hit.ImpactPoint;
		
	}

	FRotator temp = UKismetMathLibrary::FindLookAtRotation(Start, EndTrace);
	FVector AimDir = (EndTrace - Start).GetSafeNormal();
	const int32 RandomSeed = FMath::Rand();
	FRandomStream WeaponRandomStream(RandomSeed);
	float WeaponSpread = 2.0f;
	if (IsAiming == true)
		WeaponSpread = .5f;
	const float TotalSpread = WeaponSpread + CurrentFiringSpread;
	//if (MyPawn && MyPawn->IsTargeting()) //조종하고있으면
	//{
	//	FinalSpread *= InstantConfig.TargetingSpreadMod;
	//}
	const float ConeHalfAngle = FMath::DegreesToRadians(TotalSpread * 0.5f);
	const FVector ShootDir = WeaponRandomStream.VRandCone(AimDir, ConeHalfAngle, ConeHalfAngle);

	auto MyReplicateComponent = Cast< USoldierMotionReplicator>(ReplicateComponent);

	if (IsAiming == true)
	{
		CurrentFiringSpread = FMath::Min(2.f, TotalSpread);
	}
	else
	{
		CurrentFiringSpread = FMath::Min(4.f, TotalSpread);
	}
	MyReplicateComponent->Server_SetMuzzleRotation(UKismetMathLibrary::FindLookAtRotation(FVector(0, 0, 0), ShootDir));

}

FRotator ASoldier::GetMuzzleRotation()
{
	if (EquippedItem == nullptr)
		return FRotator();

	UCameraComponent* CurrentCam = FollowCamera;
	if (IsAiming)
		CurrentCam = ADSCam_;

	const float WeaponRange = 20000.f;
	const FVector StartTrace = CurrentCam->GetComponentLocation();
	FVector EndTrace = (CurrentCam->GetForwardVector() * WeaponRange) + StartTrace;
	FVector Start = EquippedItem->GetSkeletalMesh()->GetSocketLocation("Muzzle");
	//FVector Target = AimObejctFPP->GetComponentLocation();
	FHitResult Hit;
	FCollisionQueryParams QueryParams = FCollisionQueryParams(SCENE_QUERY_STAT(WeaponTrace), false, this);
	if (GetWorld()->LineTraceSingleByChannel(Hit, StartTrace, EndTrace, ECC_Visibility, QueryParams))
	{
		EndTrace = Hit.ImpactPoint;

	}

	FRotator temp = UKismetMathLibrary::FindLookAtRotation(Start, EndTrace);
	FVector AimDir = (EndTrace - Start).GetSafeNormal();
	const int32 RandomSeed = FMath::Rand();
	FRandomStream WeaponRandomStream(RandomSeed);
	float WeaponSpread = 2.0f;
	if (IsAiming == true)
		WeaponSpread = .5f;
	const float TotalSpread = WeaponSpread + CurrentFiringSpread;
	//if (MyPawn && MyPawn->IsTargeting()) //조종하고있으면
	//{
	//	FinalSpread *= InstantConfig.TargetingSpreadMod;
	//}
	const float ConeHalfAngle = FMath::DegreesToRadians(TotalSpread * 0.5f);
	const FVector ShootDir = WeaponRandomStream.VRandCone(AimDir, ConeHalfAngle, ConeHalfAngle);

	auto MyReplicateComponent = Cast< USoldierMotionReplicator>(ReplicateComponent);

	if (IsAiming == true)
	{
		CurrentFiringSpread = FMath::Min(2.f, TotalSpread);
	}
	else
	{
		CurrentFiringSpread = FMath::Min(4.f, TotalSpread);
	}
	FRotator Output = UKismetMathLibrary::FindLookAtRotation(FVector(0, 0, 0), ShootDir);
	//UE_LOG(LogTemp, Warning, TEXT("Set Muzzle Rotation : %f, %f, %f"),Output.Pitch, Output.Yaw, Output.Roll);
	return Output;
}

void ASoldier::SteamPack()
{
	ReplicateComponent->Server_SetMaxWalkSpeed(SteamPackWalkSpeed);
	if (EquippedItem == nullptr)
		return;

	//Movement->MaxAcceleration = GeneralAcceleration;
	EquippedItem->SteamPack = true;

}

void ASoldier::UnSteamPack()
{
	ReplicateComponent->Server_SetMaxWalkSpeed(GeneralWalkSpeed);
	if (EquippedItem != nullptr)
		EquippedItem->SteamPack = false;

}

void ASoldier::Attack()
{
	if (EquippedItem == nullptr || CanAim == false)
	{
		return;
	}
	if (bIsSprinting == true)
	{
		return;
	}
	IsShooting = true;
	SetMuzzleRotation();
	//먼저 클라이언트에서 쏠 수 있게
	StartFire();
	//if (ReplicateComponent != nullptr)//여기서 바로 말고..
	//	ReplicateComponent->Server_SendAttack();
}

void ASoldier::WeaponPrimaryReleased()
{
	StopFire();
}

void ASoldier::StartFire()
{
	EquippedItem->StartFire();//클라에서 신호
}

void ASoldier::StopFire()
{
	if (EquippedItem == nullptr)
		return;
	IsShooting = false;
	EquippedItem->StopFire();
	//Cast<USoldierMotionReplicator>(ReplicateComponent)->Server_SendAttackStop();

}



void ASoldier::AimDownSights()
{
	FollowCamera->Deactivate();
	ADSCam_->Activate();
	FVector VCurrent = ADSCam_->GetComponentLocation();
	FVector VTarget = EquippedItem->GetCamera()->GetComponentLocation();

	FRotator RCurrent = ADSCam_->GetComponentRotation();
	FRotator RTarget = EquippedItem->GetCamera()->GetComponentRotation();

	float Deltatime = UGameplayStatics::GetWorldDeltaSeconds(GetWorld());
	FVector NextLoc = FMath::VInterpTo(VCurrent, VTarget, Deltatime, 15);
	FRotator NexRot = FMath::RInterpTo(RCurrent, RTarget, Deltatime, 15);
	CrosshairWidget->SetVisibility(ESlateVisibility::Hidden);
	ADSCam_->SetWorldLocationAndRotation(NextLoc, NexRot);

}

void ASoldier::UnAim()
{
	FVector VCurrent = ADSCam_->GetComponentLocation();
	FVector VTarget = FollowCamera->GetComponentLocation();

	FRotator RCurrent = ADSCam_->GetComponentRotation();
	FRotator RTarget = FollowCamera->GetComponentRotation();

	float Deltatime = UGameplayStatics::GetWorldDeltaSeconds(GetWorld());
	FVector NextLoc = FMath::VInterpTo(VCurrent, VTarget, Deltatime, 15);
	FRotator NexRot = FMath::RInterpTo(RCurrent, RTarget, Deltatime, 15);

	ADSCam_->SetWorldLocationAndRotation(NextLoc, NexRot);
	if(CrosshairWidget!=nullptr)
		CrosshairWidget->SetVisibility(ESlateVisibility::Visible);
	if (FollowCamera->GetComponentLocation().Equals(ADSCam_->GetComponentLocation(), 0.01))
	{
		ADSCam_->Deactivate();
		FollowCamera->Activate();
	}
}

void ASoldier::AimMissile()
{

	FVector VCurrent = CameraBoom->GetRelativeLocation();
	FVector VTarget = MissileCameraPosition;

	float FCurrent = CameraBoom->TargetArmLength;
	float FTarget = MissileTargetArmLength;

	float Deltatime = UGameplayStatics::GetWorldDeltaSeconds(GetWorld());
	FVector NextLoc = FMath::VInterpTo(VCurrent, VTarget, Deltatime, 15);
	CameraBoom->SetRelativeLocation(NextLoc);

	float NextLenth = FMath::FInterpTo(FCurrent, FTarget, Deltatime, 15);
	CameraBoom->TargetArmLength = NextLenth;

}

void ASoldier::UnAimMissile()
{
	FVector VCurrent = CameraBoom->GetRelativeLocation();
	FVector VTarget = GeneralCameraPosition;

	float FCurrent = CameraBoom->TargetArmLength;
	float FTarget = GeneralTargetArmLength;

	float Deltatime = UGameplayStatics::GetWorldDeltaSeconds(GetWorld());
	FVector NextLoc = FMath::VInterpTo(VCurrent, VTarget, Deltatime, 15);
	CameraBoom->SetRelativeLocation(NextLoc);

	float NextLenth = FMath::FInterpTo(FCurrent, FTarget, Deltatime, 15);
	CameraBoom->TargetArmLength = NextLenth;

}





void ASoldier::WeaponReload()
{
	if (EquippedItem == nullptr)
		return;
	bool MyIsFiring = Cast<USoldierMotionReplicator>(ReplicateComponent)->IsFiring;

	if (EquippedItem->CanReload == true && MyIsFiring == false && IsReloading == false && IsItemEquipped == true)
	{
		CanAim = false;
		IsReloading = true;
		Cast<USoldierMotionReplicator>(ReplicateComponent)->NetMulticast_SetIsReloading(true);
		Cast<USoldierMotionReplicator>(ReplicateComponent)->Server_WeaponReload();
		FTimerHandle WaitHandle;
		
		float WaitTime = EquippedItem->ReloadDelay; //시간을 설정하고
		GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]()
			{
				IsReloading = false;
				Cast<USoldierMotionReplicator>(ReplicateComponent)->Server_WeaponReloadEnd();
				CanAim = true;
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
			weapon->OwnerCharacter = this;
			weapon->OwnerController = Cast< AMyPlayerController>(Controller);
			EquippedItem = weapon;//요건 전체
			PrimaryWeapon = weapon;//전체
			EquippedItem->Player = this;//전체
			EquippedItem->GetSkeletalMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			EquippedItem->AttachToPlayer(this ,"hand_rSocket");//전체
			IsItemEquipped = true;//전체
			bUseControllerRotationYaw= true;
			//자연스럽게 원하는 방향으로 회전
			FTimerHandle WaitHandle;
			EquippedItem->Reload();
			float WaitTime = 1.73; //시간을 설정하고
			GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]()
				{
					CanAim = true;
				}), WaitTime, false); //반복도 여기서 추가 변수를 선언해 설정가능
			CanAim = true;//나중에 애니메이션 노티파이로변환 1.73
			GetCharacterMovement()->bOrientRotationToMovement = false;
			// 자동적으로 캐릭터의 이동방향을 움직이는 방향에 맞춰주며 회전보간을 해줌
			
			WeaponSlotUse = EWeaponSlot::TE_PrimaryWeapon;
			if (IsLocallyControlled()&&FPSHudClass != nullptr)
			{
				SetFPSHudWidget();

			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Wiered Things happen"))
		auto weapon = Cast<AWeapon_Master>(Item);
		if (weapon != nullptr)
		{
			SecondaryWeapon = weapon;
			SecondaryWeapon->Player = this;
			SecondaryWeapon->GetSkeletalMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			SecondaryWeapon->AttachToPlayer(this, "RifleHolster");
			
		}
	}
}




void ASoldier::InteractPressed()
{

	if (IsItemEquipped == false&& DoPickupLinetrace == true)
	{
		Cast<USoldierMotionReplicator>(ReplicateComponent)->Server_SendGetItem(PickupItem);
		//SetFPSHudWidget();
	}


}



void ASoldier::WearItem()//먹을게 있는지 찾아봄
{
	if (DoPickupLinetrace == true)
	{
		FVector StartLocation = FollowCamera->GetComponentLocation();
		FVector TargetLocation = FollowCamera->GetComponentLocation() + FollowCamera->GetForwardVector() * 600.;
		TArray<TEnumAsByte<EObjectTypeQuery> > ObjectTypes;
		ObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery9);
		TArray<AActor*> ActorsToIgnore;
		TArray<FHitResult> OutHits;
		//, bool bIgnoreSelf, FLinearColor TraceColor, FLinearColor TraceHitColor, float DrawTime
		UKismetSystemLibrary::LineTraceMultiForObjects(GetWorld(), StartLocation, TargetLocation, ObjectTypes, true, ActorsToIgnore, EDrawDebugTrace::ForOneFrame, OutHits,true);
		if (OutHits.Num() > 0)
		{
			auto tmp = Cast<AWeapon_Master>(OutHits[0].GetActor());
			if (tmp != nullptr)
			{
				PickupItem = tmp;
			}
			else
			{
				PickupItem = nullptr;
			}
		}

	}
	else
		PickupItem = nullptr;
}

void ASoldier::Die()
{
	auto Anim = Cast<USoldierAnimInstance>(MyAnim);
	if (Anim != nullptr)
	{
		Anim->PlayDeathMontage();
		PlayersDied();
		//FTimerHandle TimerHandler;
		//GetWorld()->GetTimerManager().SetTimer(TimerHandler, this, &ASoldier::PlayersDied, 5, false);


	}
}

void ASoldier::PlayersDied()
{

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
	//GetMesh()->SetSimulatePhysics(true);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->DisableMovement();
	FTimerHandle VisibiltyTimerHandler;
	GetWorld()->GetTimerManager().SetTimer(VisibiltyTimerHandler, this, &ACharacter_Master::UnvisiblePlayer, 2.5f, false);

	if (HasAuthority())
	{

		FTimerHandle DestroyTimerHandler;
		GetWorld()->GetTimerManager().SetTimer(DestroyTimerHandler, this, &ACharacter_Master::DestroyPlayer, 6, false);
	}
	if (IsLocallyControlled())
	{
		HudWidget->RemoveFromViewport();//포인터는 알아서 지워지나..?
		FTimerHandle TimerHandler;
		GetWorld()->GetTimerManager().SetTimer(TimerHandler, this, &ASoldier::RespawnCharacter, 5, false);
	}
}

void ASoldier::RespawnCharacter()//Run on Owning Client
{

	Cast<USoldierMotionReplicator>(ReplicateComponent)->Server_RespawnPawn(Cast<APlayerController>(GetController()));
	UnPossessed();
}




AActor* ASoldier::FindBestTarget()
{
	TArray<TEnumAsByte<EObjectTypeQuery> > ObjectTypes;//
	UClass* ActorClassFilter = AActor::StaticClass();
	TArray<AActor*> ActorsToIgnore;// TArray<AActor*>& OutActors)
	ObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery7);//아마 이게 Character
	ObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery3);//아마 이게 Pawn
	ActorsToIgnore.Add(this);
	TArray<AActor*> OutActors;
	UKismetSystemLibrary::SphereOverlapActors(GetWorld(), GetActorLocation(), TargetingRange, ObjectTypes, ActorClassFilter, ActorsToIgnore, OutActors);
	AActor* BestTarget =nullptr;
	float CurrentValue = 0;
	float BestValue = 1000 ;
	for (auto L_CurrentTarget : OutActors)
	{
		auto Comp = L_CurrentTarget->GetComponentByClass(UTargetableComponent::StaticClass());
		if (Comp != nullptr)
		{
			auto ForwardVector = GetActorForwardVector();
			auto DirectionToTarget = UKismetMathLibrary::GetDirectionUnitVector(GetActorLocation(), L_CurrentTarget->GetActorLocation());
			CurrentValue = FVector::DotProduct(ForwardVector, DirectionToTarget);
			if (BestValue > 100 || CurrentValue > BestValue)
			{
				BestTarget = L_CurrentTarget;
				BestValue = CurrentValue;
			}

		}
			
	}
	if (BestValue <100 && UKismetMathLibrary::DegAcos(BestValue) <= TargetingConeAngle)
	{
		return BestTarget;
	}
	else
	{
		return nullptr;
	}
}

void ASoldier::SetCurrentTarget(AActor* Target)
{
	if (CurrentTarget != Target)
	{
		EndTarget();
		CurrentTarget = Target;
		BeginTarget();

	}
}

void ASoldier::EndTarget()
{
	if (TargetMarker != nullptr)
	{
		TargetMarker->Destroy();
		TargetMarker = nullptr;
	}
}
void ASoldier::BeginTarget()
{
	if (CurrentTarget != nullptr)
	{
		CurrentTarget->GetActorLocation();
		FActorSpawnParameters ActorSpawnParameters;
		FTransform ActorTransform;
		ActorTransform.SetLocation(CurrentTarget->GetActorLocation());
		TargetMarker = GetWorld()->SpawnActor<ATargetMarker>(TargetMarkerClass.Get(), ActorTransform);
		TargetMarker->AttachToActor(CurrentTarget, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
}



void ASoldier::WeaponSecondaryPressed()
{
	if (CanAim == false)
		return;
	if (EquippedItem != nullptr)
		Cast<USoldierMotionReplicator>(ReplicateComponent)->Server_SetIsAiming(true);

}
void ASoldier::WeaponSecondaryReleased()
{
	if (EquippedItem != nullptr)
	{
		Cast<USoldierMotionReplicator>(ReplicateComponent)->Server_SetIsAiming(false);
	}

}

void ASoldier::SetIsAiming(bool NewIsAiming)
{
	auto Anim = Cast<USoldierAnimInstance>(MyAnim);

	ABCHECK(Anim != nullptr);
	Anim->IsAiming = NewIsAiming;
	if (IsLocallyControlled())
	{
		IsAiming = NewIsAiming;

	}


}

//void ASoldier::Multicast_SetGun_Implementation(AWeapon_Master* weapon)
//{
//	EquippedItem = weapon;//요건 전체
//	PrimaryWeapon = weapon;//전체
//	EquippedItem->Player = this;//전체
//	EquippedItem->GetSkeletalMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
//	EquippedItem->AttachToPlayer(this, "GripPoint");//전체
//	IsItemEquipped = true;//전체
//	bUseControllerRotationYaw = true;
//	//자연스럽게 원하는 방향으로 회전
//	GetCharacterMovement()->bOrientRotationToMovement = false;
//	// 자동적으로 캐릭터의 이동방향을 움직이는 방향에 맞춰주며 회전보간을 해줌
//
//	WeaponSlotUse = EWeaponSlot::TE_PrimaryWeapon;
//}
//bool ASoldier::Multicast_SetGun_Validate(AWeapon_Master* NewItem)
//{
//	return true;
//}