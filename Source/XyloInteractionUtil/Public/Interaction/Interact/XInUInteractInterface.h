// Copyright XyloIsCoding 2024

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
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
	UXInUInteractComponent* GetInteractComponent() const;
	virtual UXInUInteractComponent* GetInteractComponent_Implementation() const = 0;

	/** Get view location and direction for the locally controlled character */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
	bool GetLocalPlayerView(FVector& LocalPlayerViewLocation, FVector& LocalPlayerViewDirection);
	virtual bool GetLocalPlayerView_Implementation(FVector& LocalPlayerViewLocation, FVector& LocalPlayerViewDirection) = 0;

	/** Checks if the interaction is possible. Called on both client and server */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
	bool CanInteract(AActor* InteractableActor, FGameplayTag InteractionTag, FGameplayTag& OutStatusTag);
	virtual bool CanInteract_Implementation(AActor* InteractableActor, FGameplayTag InteractionTag, FGameplayTag& OutStatusTag);
	/** Tries to perform the interaction. Called on both client and server */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
	bool TryInteract(AActor* InteractableActor, FGameplayTag InteractionTag);
	virtual bool TryInteract_Implementation(AActor* InteractableActor, FGameplayTag InteractionTag);
};
