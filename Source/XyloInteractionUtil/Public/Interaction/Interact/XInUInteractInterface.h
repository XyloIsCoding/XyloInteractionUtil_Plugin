// Copyright XyloIsCoding 2024

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "XInUInteractInterface.generated.h"

class UXInUInteractComponent;

// This class does not need to be modified.
UINTERFACE()
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
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
	UXInUInteractComponent* GetInteractComponent();

	/** Get view location and direction for the locally controlled character */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
	bool GetLocalPlayerView(FVector& LocalPlayerViewLocation, FVector& LocalPlayerViewDirection);

	/** Checks if the interaction is possible. Called on both client and server */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
	bool CanInteract(AActor* InteractableActor, FGameplayTag InteractionTag, FGameplayTag& OutStatusTag);
	/** Tries to perform the interaction. Called on both client and server */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
	bool TryInteract(AActor* InteractableActor, FGameplayTag InteractionTag);
};
