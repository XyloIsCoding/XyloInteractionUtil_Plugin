// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "XInUInteractableInterface.generated.h"

struct FGameplayTag;
class UXInUInteractableComponent;

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UXInUInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class XYLOINTERACTIONUTIL_API IXInUInteractableInterface
{
	GENERATED_BODY()

public:
	virtual UXInUInteractableComponent* GetInteractableComponent() = 0;
};
