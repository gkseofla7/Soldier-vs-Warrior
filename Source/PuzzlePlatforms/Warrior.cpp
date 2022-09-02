// Fill out your copyright notice in the Description page of Project Settings.
#include "Warrior.h"
#include "AnimInstance/PlayerAnimInstance.h"
#include "PlayersComponent/PlayersMotionReplicator.h"
#include "Weapons/Sword_Master.h"

#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blueprint/UserWidget.h"

AWarrior::AWarrior()
{
	bReplicates = true;
	DaerimMotionReplicator = CreateDefaultSubobject<UPlayersMotionReplicator>(TEXT("MOTIOREPLICATOR"));

	static ConstructorHelpers::FClassFinder<ASword_Master> FinderSword(TEXT("/Game/Weapons/BP_Sword_Master"));
	if (FinderSword.Succeeded())
	{
		SwordClass = FinderSword.Class;
		
		//EquippedItem->Get
	}

	static ConstructorHelpers::FClassFinder<UAnimInstance> WARRIO_ANIM((TEXT("/Game/Animation/BP_WarriorAnimInstance")));
	if (WARRIO_ANIM.Succeeded())
	{
		
		GetMesh()->SetAnimInstanceClass(WARRIO_ANIM.Class);

	}

	static ConstructorHelpers::FClassFinder<UUserWidget> CrosshairHudBPClass(TEXT("/Game/Weapons/UI/BP_Crosshair_Hud"));
	if (CrosshairHudBPClass.Succeeded())
	{
		CrosshairHudClass = CrosshairHudBPClass.Class;

	}
	
}
void AWarrior::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	auto Anim = Cast<UPlayerAnimInstance>(GetMesh()->GetAnimInstance());
	ABCHECK(nullptr != Anim);




	Anim->OnAttackHitCheck.AddUObject(this, &AWarrior::AttackCheck);

}
void AWarrior::BeginPlay()
{
	Super::BeginPlay();
	MyAnim = Cast<UAnimInstance_Master>(GetMesh()->GetAnimInstance());

	EquippedItem = GetWorld()->SpawnActor<ASword_Master>(SwordClass);// GetMesh()->GetSocketTransform("SwordSocket")
	EquippedItem->GetSkeletalMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EquippedItem->AttachToPlayer(this, "SwordSocket");
	auto Anim = Cast<UPlayerAnimInstance>(GetMesh()->GetAnimInstance());
	
	Anim->OnHangMovePlace.AddLambda([this]()->void {
		HangMontageNotify();
		});

	if (CrosshairHudClass != nullptr)
	{

		HudWidget = CreateWidget<UUserWidget>(GetWorld(), CrosshairHudClass);
		HudWidget->AddToViewport();

	}
}
void AWarrior::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ForwardTrace();
	HeightTrace();
}

void AWarrior::AttackCheck()
{
	UE_LOG(LogTemp, Warning, TEXT("AttackCheck"));
	if (HasAuthority())
	{
		FHitResult HitResult;
		FCollisionQueryParams Params(NAME_None, false, this);

		float AttackRange = 200.f;
		float AttackRadius = 50.f;


		bool bResult = GetWorld()->SweepSingleByChannel(
			OUT HitResult,
			GetActorLocation(),
			GetActorLocation() + GetActorForwardVector() * AttackRange,
			FQuat::Identity,
			ECollisionChannel::ECC_GameTraceChannel2,
			FCollisionShape::MakeSphere(AttackRadius),
			Params);
		FVector Vec = GetActorForwardVector() * AttackRange;
		FVector Center = GetActorLocation() + Vec * 0.5f;
		float HalfHeight = AttackRange * 0.5f + AttackRadius;
		FQuat Rotation = FRotationMatrix::MakeFromZ(Vec).ToQuat();
		FColor DrawColor;
		if (bResult)
			DrawColor = FColor::Green;
		else
			DrawColor = FColor::Red;


		DrawDebugCapsule(GetWorld(), Center, HalfHeight, AttackRadius,
			Rotation, DrawColor, false, 5.f);
		if (bResult && HitResult.Actor.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("Hit Actor Name : %s"), *HitResult.Actor->GetName());
			FDamageEvent DamageEvent;
			HitResult.Actor->TakeDamage(50.0f, DamageEvent, GetController(), this);

		}
	}
}

void AWarrior::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	//PlayerInputComponent->BindAction("Climb", IE_Pressed, this, &AWarrior::ClimbTheWall);
	//PlayerInputComponent->BindAction("Climb", IE_Released, this, &AWarrior::ClimbTheWall);
	PlayerInputComponent->BindAction("ClimbUp", IE_Pressed, this, &AWarrior::ClimbUp);
	PlayerInputComponent->BindAction("DropDown", IE_Pressed, this, &AWarrior::DropDown);

}


void AWarrior::MoveForward(float Value)
{


	MoveUpDown = Value;
	if ((Value != 0.0f) && IsClimbing == false)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
	{
		//if (Value == 0)
		//	return;

		//if (IsClimbing == false)
		//{
		//	// find out which way is forward
		//	const FRotator Rotation = Controller->GetControlRotation();
		//	const FRotator YawRotation(0, Rotation.Yaw, 0);
		//	// get forward vector
		//	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		//	AddMovementInput(Direction, Value);

		//}
		//else
		//{
		//	//if (IsOnEdge == false)
		//	//{
		//	//	UE_LOG(LogTemp, Warning, TEXT("Wtf"));
		//	//	MoveUpDown = 0;
		//	//}
		//	//else
		//	AddMovementInput(GetActorUpVector(), Value);
		//}
	}
}




void AWarrior::MoveRight(float Value)
{

	MoveRightLeft = Value;
	{
	//if ((Controller != nullptr) && (Value != 0.0f))
	//{
	//	if (IsClimbing == false)
	//	{
	//		// find out which way is right
	//		const FRotator Rotation = Controller->GetControlRotation();
	//		const FRotator YawRotation(0, Rotation.Yaw, 0);

	//		// get right vector 
	//		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	//		// add movement in that direction
	//		AddMovementInput(Direction, Value);
	//	}
	//	else
	//	{
	//		AddMovementInput(GetActorRightVector(), Value);
	//	}
	//}
	}


		if ((Controller != nullptr) && (Value != 0.0f)&&IsClimbing==false)
		{

			AddMovementInput(GetActorRightVector(), Value);
		}
}


//void AWarrior::ClimbTheWall()
//{
//	if(ClimbTheWallOn == false)
//		ClimbTheWallOn = true;
//	else
//	{
//		ClimbTheWallOn = false;
//	}
//	if (IsClimbing)
//	{
//		FVector tmp = GetActorForwardVector() * -500;
//		LaunchCharacter(FVector(tmp.X, tmp.Y, 0), false, false);
//		JumpFromWall();
//	}
//
//}

//void AWarrior::Climb()
//{
//	bool CanClimb = ForwardTrace();
//	if (CanClimb&&IsClimbing ==false)
//	{
//		
//		float YawValue = UKismetMathLibrary::MakeRotFromX(WallNormal).Yaw + 180;
//		SetActorRotation(FRotator(0, YawValue, 0));
//		IsClimbing = true;
//		GetCharacterMovement()->Velocity =(FVector(0, 0, 0));
//		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
//	}
//}

bool AWarrior::ForwardTrace()
{
	auto StartLoc = GetActorLocation();
	auto EndLoc = StartLoc + GetActorForwardVector() * 150;//Hit, From, To, ECC_Visibility, QueryParams)
	TArray<AActor*> Ignore;
	Ignore.Add(this);
	FHitResult OutHit;
	bool Output = UKismetSystemLibrary::SphereTraceSingle(GetWorld(),StartLoc,EndLoc,10.,ETraceTypeQuery::TraceTypeQuery1,false, Ignore,EDrawDebugTrace::ForOneFrame,OutHit,true);
	WallNormal = OutHit.Normal;
	WallLocation = OutHit.Location;
	//UE_LOG(LogTemp, Warning, TEXT("WallLocation %f, %f %f"), WallLocation.X, WallLocation.Y, WallLocation.Z);
	return Output;

}
	

void AWarrior::HeightTrace()
{
	auto StartLoc = GetActorLocation() + FVector(0,0,500.) +  GetActorForwardVector()*75;
	auto EndLoc = StartLoc - FVector(0, 0, 500.);
	TArray<AActor*> Ignore;
	Ignore.Add(this);
	FHitResult OutHit;
	bool ClimbUp = UKismetSystemLibrary::SphereTraceSingle(GetWorld(), StartLoc, EndLoc, 10., ETraceTypeQuery::TraceTypeQuery1, false, Ignore, EDrawDebugTrace::ForOneFrame, OutHit, true);
	if (ClimbUp == true)
	{
		HeightLocation = OutHit.Location;
		float tmp = GetMesh()->GetSocketLocation("pelvisSocket").Z - HeightLocation.Z;
		if (tmp >= -50 && tmp <= 0)
		{
			if (IsClimbing == false)
			{
				GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
				GetCharacterMovement()->StopMovementImmediately();
				Hang();
			}
		}
	}

}

void AWarrior::Hang()
{
	auto Anim = Cast<UPlayerAnimInstance>(GetMesh()->GetAnimInstance());
	PlayHangToCrouchMontage();
	Anim->Montage_Pause();
	IsClimbing = true;
	FVector tmpLoc = WallNormal * 20 + WallLocation;
	FVector TargetRelativeLocation = FVector(tmpLoc.X, tmpLoc.Y, HeightLocation.Z - 90);
	FRotator TargetRelativeRotation = UKismetMathLibrary::MakeRotFromX((WallNormal * -1));
	FLatentActionInfo Latentinfo;
	Latentinfo.CallbackTarget = this;
	Latentinfo.ExecutionFunction = "****OnMoveComponentToEnd";
	Latentinfo.Linkage = 0;
	UKismetSystemLibrary::MoveComponentTo(GetCapsuleComponent(), TargetRelativeLocation, TargetRelativeRotation, true, true, .2, false,EMoveComponentAction::Move, Latentinfo);

}

void AWarrior::PlayHangToCrouchMontage()
{
	auto Anim = Cast<UPlayerAnimInstance>(GetMesh()->GetAnimInstance());
	if (Anim == nullptr)
	{
		return;
	}

	Cast<UPlayerAnimInstance>(Anim)->PlayHangToCrouchMontage();
}

void AWarrior::ClimbUp()
{
	Cast<UPlayersMotionReplicator>(DaerimMotionReplicator)->Server_SendClimbUp();
	//if (IsClimbing == true)
	//{
	//	PlayHangToCrouchMontage();
	//}
}
void AWarrior::HangMontageNotify()
{

	FVector TargetRelativeLocation = GetCapsuleComponent()->GetComponentLocation() + 50 * GetActorForwardVector();
	FRotator TargetRelativeRotation = UKismetMathLibrary::MakeRotFromX((GetActorForwardVector()));
	FLatentActionInfo Latentinfo;
	Latentinfo.CallbackTarget = this;
	Latentinfo.ExecutionFunction = "****OnMoveComponentToEnd";
	Latentinfo.Linkage = 0;
	UKismetSystemLibrary::MoveComponentTo(GetCapsuleComponent(), TargetRelativeLocation, TargetRelativeRotation, true, true, .2, false, EMoveComponentAction::Move, Latentinfo);
	IsClimbing = false;
	//UE_LOG(LogTemp, Warning, TEXT("IsClimbing!"))
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);

}
void AWarrior::DropDown()
{
	if (IsClimbing == true)
	{
		auto Anim = Cast<UPlayerAnimInstance>(GetMesh()->GetAnimInstance());
		IsClimbing = false;
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
		StopAnimMontage(Anim->GetHangToCrouchMontage());
	}
}

