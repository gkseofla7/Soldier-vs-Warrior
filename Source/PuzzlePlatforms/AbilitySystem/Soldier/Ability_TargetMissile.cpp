// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability_TargetMissile.h"
#include "../../Soldier.h"
#include "TargetMissileMovementComponent.h"
#include "TargetMissileReplicateComponent.h"
#include "../../Weapons/Weapon_Master.h"
#include "../ActorAbilities.h"

AAbility_TargetMissile::AAbility_TargetMissile()
	:Super()
{
	MeshOffsetRoot = CreateDefaultSubobject<USceneComponent>(TEXT("MeshOffsetRoot"));
	MeshOffsetRoot->SetupAttachment(RootComponent);
	MissileMovementComponent = CreateDefaultSubobject<UTargetMissileMovementComponent>(TEXT("MissileMovementComponent"));
	MissileReplicateComponent = CreateDefaultSubobject<UTargetMissileReplicateComponent>(TEXT("MissileReplicateComponent"));
	AbilityRoot->OnComponentBeginOverlap.AddDynamic(this, &AAbility_TargetMissile::OnOverlapBegin);
	MissileMeshComponent = CreateDefaultSubobject< UStaticMeshComponent>(TEXT("MissileComponent"));
	MissileMeshComponent->SetupAttachment(MeshOffsetRoot);
	MissileMeshComponent->SetVisibility(false);
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ParticleAsset(TEXT("/Game/Etcs/p_Turret_Explosion"));
	if (ParticleAsset.Succeeded())
	{
		ParticleTemplate = ParticleAsset.Object;
	}
}
void AAbility_TargetMissile::BeginPlay()
{
	Super::BeginPlay();

	if (PlayerRef->IsLocallyControlled())//애초에 서버로 알려줘야되니,,ㅋ
	{

		SoldierRef = Cast<ASoldier>(PlayerRef);
		SoldierRef->ShowTarget = true;
		SoldierRef->GetMesh()->bPauseAnims = true;
		PlayerRef->OnSkillReleased.AddUObject(this, &AAbility_TargetMissile::ActivateEffect);//등록하는애들만
	}



}

void AAbility_TargetMissile::ActivateEffect_Implementation()
{

	PlayerRef->OnSkillReleased.Clear();//Release하는 순간 초기화

	SoldierRef->SetUsingSkill(false);
	SoldierRef->GetMesh()->bPauseAnims = false;
	SoldierRef->ShowTarget = false;
	if (SoldierRef->CurrentTarget == nullptr)
		return;

	Server_SetTransform(SoldierRef->RocketHolderComponent->GetSocketTransform("Mouth"));
	Server_SetVisibility();
	Server_DetachAbilityFromPlayer();//모두 일단 띄어냄
	Server_SetTarget(SoldierRef->CurrentTarget);
	Server_SetActive();
}

void AAbility_TargetMissile::Server_SetVisibility_Implementation()
{
	NetMulticast_SetVisibility();
}

void AAbility_TargetMissile::NetMulticast_SetVisibility_Implementation()
{
	MissileMeshComponent->SetVisibility(true);
}


bool AAbility_TargetMissile::Server_SetVisibility_Validate()
{
	return true;
}

bool AAbility_TargetMissile::NetMulticast_SetVisibility_Validate()
{
	return true;
}

void AAbility_TargetMissile::OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

	if (OtherActor == PlayerRef)
		return;
	//UE_LOG(LogTemp, Warning, TEXT("Hit %s, %s, %s %s"), *OtherActor->GetName(), *OverlappedComp->GetName(), *OtherActor->GetName(), *OtherComp->GetName());
	if (HasAuthority() == true)
	{
		NetMulticast_Spark(GetActorLocation());
		//auto Player = Cast<ACharacter_Master>(OtherActor);
		//if (Player != nullptr)
		//{
		//	UGameplayStatics::ApplyDamage(Player, DamageAmount, PlayerRef->GetController(), PlayerRef, UDamageType::StaticClass());

		//}
		Destroy();
	}
}


void AAbility_TargetMissile::NetMulticast_Spark_Implementation(FVector Location)
{
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ParticleTemplate, Location, FRotator(0, 0, 0));
}

bool AAbility_TargetMissile::NetMulticast_Spark_Validate(FVector Location)
{
	return true;
}

void AAbility_TargetMissile::Server_SetActive_Implementation()
{
	NetMulticast_SetActive();
}

bool AAbility_TargetMissile::Server_SetActive_Validate()
{
	return true;
}

void AAbility_TargetMissile::NetMulticast_SetActive_Implementation()
{
	bActive = true;
}

bool AAbility_TargetMissile::NetMulticast_SetActive_Validate()
{
	return true;
}

void AAbility_TargetMissile::Server_SetTarget_Implementation(AActor* NewTarget)
{
	MissileMovementComponent->Target = NewTarget;
}

bool AAbility_TargetMissile::Server_SetTarget_Validate(AActor* NewTarget)
{
	return true;
}

void AAbility_TargetMissile::SetAbilityLevel()
{
	//쓸때마다 불러옴
	auto Spells = PlayerRef->ActorAbilitiesComponent->PlayerSpells;
	//UE_LOG(LogTemp, Warning, TEXT("Spellbook Num : %d "), Spells.Num());
	for (int i = 0; i < Spells.Num(); i++)
	{
		if (Spells[i]->IsChildOf(AAbility_TargetMissile::StaticClass()) == true)
		{
			AbilityLevel = Cast< AMyPlayerState>(PlayerRef->GetPlayerState())->SpellsUpgrade[i];
		}
	}
}