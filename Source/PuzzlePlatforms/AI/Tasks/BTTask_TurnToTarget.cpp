// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_TurnToTarget.h"

#include "../NPC_Master.h"
#include "../NPCAIController.h"
#include "../../Character_Master.h"

#include "BehaviorTree/BlackboardComponent.h"
UBTTask_TurnToTarget::UBTTask_TurnToTarget()
{
	NodeName = TEXT("Turn");
}

EBTNodeResult::Type UBTTask_TurnToTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMomory)
{
	EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMomory);

	auto Monster = Cast<ANPC_Master>(OwnerComp.GetAIOwner()->GetPawn());
	if (Monster == nullptr)
		return EBTNodeResult::Failed;

	auto Target = Cast<ACharacter_Master>(
		OwnerComp.GetBlackboardComponent()->GetValueAsObject(ANPCAIController::TargetKey));
	if (nullptr == Target)
		return EBTNodeResult::Failed;


	FVector LookVector = Target->GetActorLocation() - Monster->GetActorLocation();

	LookVector.Z = 0.0f;
	FRotator TargetRot = FRotationMatrix::MakeFromX(LookVector).Rotator();
	if (Monster->HasAuthority())
	{//movement 알아서 해주나?
		Monster->SetActorRotation(FMath::RInterpTo(Monster->GetActorRotation(), TargetRot, GetWorld()->GetDeltaSeconds(), 4.0f));
	}
	return EBTNodeResult::Succeeded;
}