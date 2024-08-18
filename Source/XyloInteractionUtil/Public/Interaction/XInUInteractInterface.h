// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "XInUInteractInterface.generated.h"

class UXInUInteractComponent;

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UXInUInteractInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class XYLOINTERACTIONUTIL_API IXInUInteractInterface
{
	GENERATED_BODY()

public:
	virtual UXInUInteractComponent* GetInteractComponent() = 0;

	/** Get view location and direction for the locally controlled character */
	virtual bool GetLocalPlayerView(FVector& LocalPlayerViewLocation, FVector& LocalPlayerViewDirection) = 0;
	
	virtual bool CanInteract(AActor* InteractableActor, FGameplayTag InteractionTag, FGameplayTag& OutStatusTag);
	virtual bool TryInteract(AActor* InteractableActor, FGameplayTag InteractionTag, FGameplayTag& OutStatusTag);
};
