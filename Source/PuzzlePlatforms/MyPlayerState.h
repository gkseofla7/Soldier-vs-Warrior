// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PuzzlePlatforms.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystem/UI/PlayerStateSpellbookInterface.h"
#include "MyPlayerState.generated.h"

/**
 * 
 */
DECLARE_MULTICAST_DELEGATE(FOnSkillPointChangedDelegate);
UCLASS()
class PUZZLEPLATFORMS_API AMyPlayerState : public APlayerState, public IPlayerStateSpellbookInterface
{
	GENERATED_BODY()

public:
	AMyPlayerState();

	int32 GetGameScore() const;
	void BeginPlay() override;


	UFUNCTION(Server, Reliable, WithValidation)
		void Server_SetPlayerName(const FText& NewPlayerName);
	UFUNCTION(Server, Reliable, WithValidation)
		void Server_SpellsUpgrade(int index);
	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void NetMulticast_SpellsUpgrade(int index);
	UFUNCTION(Server, Reliable, WithValidation)
		void Server_SetSkillPoints(int NewSkillPoint);
	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void NetMulticast_SetSkillPoints(int NewSkillPoint);
	virtual int GetSkillPoints() override;
	//void SetMaxLevel();


public:
	class UMyCharacterStatComponent* CharacterStat;
	UPROPERTY(Transient)
		int32 GameScore;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		TArray<int> SpellsUpgrade;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		TArray<int> SpellsMaxUpgrade;
	int SkillPoints = 5;


	FOnSkillPointChangedDelegate OnSkillPointChangedDelegate;
	//class UMyCharacterStatComponent* CharacterStat;
	
	//FText PlayerName;

};
