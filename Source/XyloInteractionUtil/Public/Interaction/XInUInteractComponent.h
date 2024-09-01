// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "XInUInteractComponent.generated.h"


class IXInUInteractInterface;
struct FGameplayTag;

UCLASS(BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class XYLOINTERACTIONUTIL_API UXInUInteractComponent : public UPawnComponent
{
	GENERATED_BODY()

public:	
	UXInUInteractComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * UActorComponent Interface
	 */
	
protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * Interact
	 */

private:
	IXInUInteractInterface* InteractInterface;
protected:
	IXInUInteractInterface* GetInteractInterface();
	
private:
	/** List on interactable actors in range, is updated by AddInteractableInRange and RemoveInteractableInRange */
	UPROPERTY()
	TArray<AActor*> InteractablesInRange;
	/** List on interactable actors in range which are not available for interaction, is updated by
	 * AddInteractableInRange, RemoveInteractableInRange and by bound delegate */
	UPROPERTY()
	TArray<AActor*> DisabledInteractablesInRange;
	/** Only set locally from UpdateSelectedInteractable_Local */
	UPROPERTY()
	AActor* SelectedInteractable;

protected:
	virtual void UpdateSelectedInteractable_Local();
	virtual void UpdateInteractableStatus_Local(AActor* Interactable, bool bSelected = false);

protected:
	UFUNCTION()
	virtual void OnInteractableAvailabilityChanged(AActor* Interactable, bool bAvailable);
public:
	/** Add an interactable actor to the in range list. Should be called both on client and server */
	virtual void AddInteractableInRange(AActor* NewInteractable);
	/** Remove an interactable actor from the in range list. Should be called both on client and server */
	virtual void RemoveInteractableInRange(AActor* NewInteractable);

	/** Function to call to start an interaction. should be called from locally controlled actors */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	virtual void Interact(FGameplayTag InteractionTag);
	UFUNCTION(Server, Reliable)
	virtual void ServerInteract(AActor* Interactable, FGameplayTag InteractionTag);
	virtual void ExecuteInteraction(AActor* Interactable, FGameplayTag InteractionTag);


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * Debug
	 */

private:
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDebugArrayCount = false;
	
};
