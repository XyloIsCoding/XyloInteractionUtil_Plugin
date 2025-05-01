// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "XInUInteractionTypes.generated.h"


USTRUCT()
struct FXInUInteractionKey
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FGameplayTag Channel;
	UPROPERTY(BlueprintReadWrite)
	FGameplayTag Action;
};
