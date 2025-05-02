// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "XInUInteractionInterface.generated.h"

struct FGameplayTag;
struct FXInUInteractionState;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UXInUInteractionInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class XYLOINTERACTIONUTIL_API IXInUInteractionInterface
{
	GENERATED_BODY()

public:
	virtual bool CanInteract(AActor* OtherActor, const FGameplayTag& Channel, const FGameplayTag& Action, FXInUInteractionState& OutState) = 0;
	virtual bool TryInteract(AActor* OtherActor, const FGameplayTag& Channel, const FGameplayTag& Action) = 0;
	
};
