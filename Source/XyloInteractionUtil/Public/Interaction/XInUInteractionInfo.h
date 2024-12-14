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
	AActor* Interactable;
	UPROPERTY(BlueprintReadWrite)
	FGameplayTag InteractionChannel;
	UPROPERTY(BlueprintReadWrite)
	bool bSelected;
	UPROPERTY(BlueprintReadWrite)
	TArray<FXInUInteractionType> Interactions;
};