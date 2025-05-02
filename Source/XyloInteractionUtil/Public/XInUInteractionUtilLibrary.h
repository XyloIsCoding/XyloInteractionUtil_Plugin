// Copyright 2025, Davide Bosco. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "XInUInteractionUtilLibrary.generated.h"

class UXInUInteractorComponent;
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
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	static UXInUInteractableComponent* GetInteractableComponent(AActor* InteractableActor);
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	static UXInUInteractorComponent* GetInteractorComponent(AActor* InteractorActor);
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	static bool GetInteractionAimTransform(AActor* InteractActor, FTransform& AimTransform);
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	static float GetDefaultInteractionDuration(AActor* InteractableActor, const FGameplayTag& Action);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	static bool GetInteractionStateFromInfo(const FXInUInteractionInfo& InteractionInfo, const FGameplayTag Action, FXInUInteractionState& OutInteractionState);
};
