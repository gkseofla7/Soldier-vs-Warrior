// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include"../PuzzlePlatforms.h"
#include "Blueprint/UserWidget.h"
#include "PlayerInfoWidget.generated.h"

/**
 * 
 */
UCLASS()
class PUZZLEPLATFORMS_API UPlayerInfoWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual bool Initialize();

public:
	UFUNCTION(BlueprintImplementableEvent)
	void CustomInitialize(int Num);
private:
public:

	UPROPERTY(meta = (BindWidget))
		class UProgressBar* HpBar;
	UPROPERTY(meta = (BindWidget))
		class UProgressBar* MpBar;
	UPROPERTY(meta = (BindWidget))
		class UTextBlock* PlayerName;

	UPROPERTY(meta = (BindWidget))
		class UTextBlock* HpNum;
	UPROPERTY(meta = (BindWidget))
		class UTextBlock* MpNum;
	UPROPERTY(meta = (BindWidget),BlueprintReadWrite)
		class UMapWidget* MapWidget;
	UPROPERTY(meta = (BindWidget),BlueprintReadWrite)
		class UMinimapWidget* Minimap_Widget;
public:
	void BindCharacterStat(class UMyCharacterStatComponent* NewCharacterStat);
	void BindCharacterName(FText NewName);
	void ToggleMap();

private:
	TWeakObjectPtr<class UMyCharacterStatComponent> CurrentCharacterStat;
};