// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "XInUInteractionTypes.generated.h"


USTRUCT(BlueprintType)
struct FXInUInteractionTimerData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FGameplayTag Action;
	UPROPERTY(BlueprintReadWrite)
	float StartTime;
	UPROPERTY(BlueprintReadWrite)
	float Duration;

	void Reset();
};

USTRUCT(BlueprintType)
struct FXInUInteractionState
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	FGameplayTag InteractionStatus;
};

USTRUCT(BlueprintType)
struct FXInUInteractionInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	AActor* Interactable = nullptr;
	UPROPERTY(BlueprintReadWrite)
	FGameplayTag Channel;
	UPROPERTY(BlueprintReadWrite)
	bool bSelected = false;
private:
	UPROPERTY(BlueprintReadWrite)
	TMap<FGameplayTag, FXInUInteractionState> InteractionsState;

public:
	FXInUInteractionState* GetInteractionState(const FGameplayTag& Action);
	void AddInteractionState(const FGameplayTag& Action, const FXInUInteractionState& State);
};
