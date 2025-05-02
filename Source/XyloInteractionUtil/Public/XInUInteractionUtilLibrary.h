// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "XInUInteractionUtilLibrary.generated.h"

struct FXInUInteractionState;
struct FXInUInteractionInfo;
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
	UFUNCTION(BlueprintCallable)
	static UXInUInteractableComponent* GetInteractableComponent(AActor* InteractableActor);
	UFUNCTION(BlueprintCallable)
	static bool GetInteractionAimTransform(AActor* InteractActor, FTransform& AimTransform);
	UFUNCTION(BlueprintCallable)
	static float GetDefaultInteractionDuration(AActor* InteractableActor, const FGameplayTag& Action);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static FXInUInteractionState GetInteractionStateFromInfo(const FXInUInteractionInfo& InteractionInfo, const FGameplayTag& Action);
};
