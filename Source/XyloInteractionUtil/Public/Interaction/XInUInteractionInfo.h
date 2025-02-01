#pragma once

#include "GameplayTagContainer.h"
#include "XInUInteractionInfo.generated.h"


USTRUCT(BlueprintType)
struct FXInUInteractionType
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FGameplayTag InteractionTag;
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
	FGameplayTag InteractionChannel;
	UPROPERTY(BlueprintReadWrite)
	bool bSelected = false;
	UPROPERTY(BlueprintReadWrite)
	TArray<FXInUInteractionType> Interactions;
};

USTRUCT(BlueprintType)
struct FXInUInteractionTimerData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FGameplayTag InteractionTag;
	UPROPERTY(BlueprintReadWrite)
	float StartTime = -1.f;
	UPROPERTY(BlueprintReadWrite)
	float Duration = -1.f;
};

USTRUCT(BlueprintType)
struct FXInUInteractionTimerHandle
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FGameplayTag InteractionTag;
	UPROPERTY(BlueprintReadWrite)
	FTimerHandle TimerHandle;
	UPROPERTY(BlueprintReadWrite)
	bool bClientOnly = false;
};
