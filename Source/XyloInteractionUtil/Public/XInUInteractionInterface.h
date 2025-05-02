// Copyright 2025, Davide Bosco. All Rights Reserved.

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
	/** @return true if OutState is filled */
	virtual bool CanInteract(AActor* OtherActor, const FGameplayTag& Channel, const FGameplayTag& Action, FXInUInteractionState& OutState) = 0;
	/** @return true if interacted */
	virtual bool TryInteract(AActor* OtherActor, const FGameplayTag& Channel, const FGameplayTag& Action) = 0;
	
};
