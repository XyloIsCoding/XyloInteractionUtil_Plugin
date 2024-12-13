// Copyright XyloIsCoding 2024

#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "Interaction/XInUBaseInteractionComponent.h"
#include "XInUInteractComponent.generated.h"


class IXInUInteractInterface;
struct FGameplayTag;

USTRUCT()
struct FXInUInteractablesContainer
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<AActor*> Interactables;
};

UCLASS(BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class XYLOINTERACTIONUTIL_API UXInUInteractComponent : public UXInUBaseInteractionComponent
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

protected:
	IXInUInteractInterface* GetInteractInterface();

protected:
	virtual void SetSelectedInteractable(const FGameplayTag InteractionChannel, AActor* NewInteractable);
	virtual AActor* GetSelectedInteractable(const FGameplayTag InteractionChannel);
	virtual bool IsInteractableInRange(const FGameplayTag InteractionChannel, const AActor* Interactable) const;
	virtual int32 RemoveInteractableFromInRangeMap(const FGameplayTag InteractionChannel, AActor* Interactable);
	virtual int32 AddInteractableToInRangeMap(const FGameplayTag InteractionChannel, AActor* Interactable);
private:
	/** List on interactable actors in range, is updated by AddInteractableInRange and RemoveInteractableInRange */
	UPROPERTY()
	TMap<FGameplayTag, FXInUInteractablesContainer> InteractablesInRange;
	/** List on interactable actors in range which are not available for interaction, is updated by
	 * AddInteractableInRange, RemoveInteractableInRange and by bound delegate */
	UPROPERTY()
	TArray<AActor*> DisabledInteractablesInRange;
	/** Only set locally from UpdateSelectedInteractable_Local */
	UPROPERTY()
	TMap<FGameplayTag, AActor*> SelectedInteractable;

public:
	virtual void RefreshAllInteractableStatus_Local();
protected:
	virtual void UpdateSelectedInteractable_Local();
	/** Should be always called for Interactables when adding or removing them from InteractablesInRange.
	 * Is also called by RefreshAllInteractableStatus_Local and UpdateSelectedInteractable_Local */
	virtual void UpdateInteractableStatus_Local(AActor* Interactable, const bool bSelected = false);

protected:
	/** Bound to Interactable::AvailableForInteractionDelegate on both client and server */
	UFUNCTION()
	virtual void OnInteractableAvailabilityChanged(AActor* Interactable, const bool bAvailable);
public:
	/** Add an interactable actor to the in range list. Should be called both on client and server */
	virtual void AddInteractableInRange(AActor* NewInteractable);
	/** Remove an interactable actor from the in range list. Should be called both on client and server */
	virtual void RemoveInteractableInRange(AActor* NewInteractable);

	/** Function to call to start an interaction. should be called from locally controlled actors.
	 * (Calls ExecuteInteraction, and if not authority calls ServerInteractRPC) */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	virtual void InputInteract(const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag);
	UFUNCTION(Server, Reliable)
	/** Calls ExecuteInteraction */
	virtual void ServerInteractRPC(AActor* Interactable, const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag);
	virtual void Interact(AActor* Interactable, const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag);


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * Debug
	 */

private:
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDebugArrayCount = false;
	
};