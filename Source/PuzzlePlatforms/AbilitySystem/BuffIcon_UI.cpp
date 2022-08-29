// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffIcon_UI.h"

#include "Ability_Buff.h"
#include "BuffPanel_UI.h"

#include "Components/WrapBox.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Kismet/KismetMathLibrary.h"



void UBuffIcon_UI::CustomInitialize(class AAbility_Buff* NewBuffAbility)
{
	BuffAbility = NewBuffAbility;
	BuffImage->SetBrushFromTexture(BuffAbility->AbilityDetails.Icon);
	BuffLifeSpan = BuffAbility->BuffLifeSpan;
	BuffAbility->OnEndBuffDelegate.AddUObject(this, &UBuffIcon_UI::DeleteFromParent);

	FTimerHandle TimerHandler;
	GetWorld()->GetTimerManager().SetTimer(TimerHandler, this, &UBuffIcon_UI::UpdateProgressBar, .5, true);
}

void UBuffIcon_UI::UpdateProgressBar()
{
	if (BuffAbility != nullptr)
	{
		float percent = UKismetMathLibrary::NormalizeToRange(BuffAbility->GetLifeSpan(), 0, BuffLifeSpan);
		percent = 1 - percent;
		BuffTimer->SetPercent(percent);
	}


}

void UBuffIcon_UI::DeleteFromParent()
{
	//Cast< UBuffPanel_UI>(GetParent())->BuffPanel
	RemoveFromParent();
	UE_LOG(LogTemp, Warning, TEXT("aa"));

}