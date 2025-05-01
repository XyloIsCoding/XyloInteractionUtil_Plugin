// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "XInUInteractionUtilLibrary.generated.h"

struct FGameplayTag;
class UXInUInteractableComponent;

/**
 * 
 */
UCLASS()
class XYLOINTERACTIONUTIL_API UXInUInteractionUtilLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static UXInUInteractableComponent* GetInteractableComponent(AActor* InteractableActor);
	static bool GetInteractionAimTransform(AActor* InteractActor, FTransform& AimTransform);
	static float GetDefaultInteractionDuration(AActor* InteractableActor, const FGameplayTag& Action);
};
