// Copyright XyloIsCoding 2024

#pragma once

#include "CoreMinimal.h"
#include "Interaction/XInUBaseInteractionComponent.h"
#include "XInUInteractComponent.generated.h"


struct FXInUInteractionTimerHandle;
class UXInUInteractableComponent;
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
	 * UXInUBaseInteractionComponent Interface
	 */

/*--------------------------------------------------------------------------------------------------------------------*/
	/* Interaction Timer */
	
public:
	/** as of now can only be used on local client. use overload to get on server */
	virtual float GetDefaultInteractionDurationByTag(const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag) override;
	virtual float GetInteractionDurationByTag(const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag) override;
	virtual float GetInteractionTimeElapsedByTag(const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag) override;
	virtual float GetInteractionTimeLeftByTag(const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag) override;

	/** Overload that can be used on server too */
	virtual float GetDefaultInteractionDurationByTag(AActor* Interactable, const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag);

/*--------------------------------------------------------------------------------------------------------------------*/
	
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * Interact
	 */

protected:
	virtual IXInUInteractInterface* GetInteractInterface();
	virtual UXInUInteractableComponent* GetInteractableComponent(AActor* Interactable);
	virtual UXInUInteractableComponent* GetAvailableInteractableComponent(AActor* Interactable, const FGameplayTag InteractionChannel);

protected:
	virtual void SetSelectedInteractable(AActor* NewInteractable, const FGameplayTag InteractionChannel);
	virtual AActor* GetSelectedInteractable(const FGameplayTag InteractionChannel) const;
	virtual bool IsInteractableInRange(const AActor* Interactable, const FGameplayTag InteractionChannel) const;
	virtual int32 RemoveInteractableFromInRangeMap(AActor* Interactable, const FGameplayTag InteractionChannel);
	virtual int32 AddInteractableToInRangeMap(AActor* Interactable, const FGameplayTag InteractionChannel);
	virtual bool IsInteractableSelected(const AActor* Interactable, const FGameplayTag InteractionChannel) const;
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

public:
	/** Add an interactable actor to the in range list. Should be called both on client and server */
	virtual void AddInteractableInRange(AActor* NewInteractable);
	/** Remove an interactable actor from the in range list. Should be called both on client and server */
	virtual void RemoveInteractableInRange(AActor* NewInteractable);
protected:
	/** Bound to Interactable::AvailableForInteractionDelegate on both client and server */
	UFUNCTION()
	virtual void OnInteractableAvailabilityChanged(AActor* Interactable, const bool bAvailable);

/*--------------------------------------------------------------------------------------------------------------------*/
	/* Interaction */
	
public:
	/** Function to call to start an interaction. should be called from locally controlled actors.
	 * (Calls ExecuteInteraction, and if not authority calls ServerInteractRPC) */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	virtual void InputStartInteraction(const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag);
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	virtual void InputStopInteraction(const FGameplayTag InteractionChannel);
protected:
	UFUNCTION(Server, Reliable)
	virtual void ServerStartInteractionRPC(AActor* Interactable, const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag);
	UFUNCTION(Server, Reliable)
	virtual void ServerStopInteractionRPC(AActor* Interactable, const FGameplayTag InteractionChannel);
	
	virtual bool StartInteraction(AActor* Interactable, const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag);
	virtual bool StartInteractionWithDuration(AActor* Interactable, FGameplayTag InteractionChannel, FGameplayTag InteractionTag, const float InteractionTime);
	virtual void StopInteraction(AActor* Interactable, const FGameplayTag InteractionChannel);
	
	virtual void Interact(AActor* Interactable, const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag);
	virtual void InteractFromTimer(AActor* Interactable, const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag);
	UFUNCTION(Server, Reliable)
	virtual void ServerInteractFromTimerRPC(AActor* Interactable, const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag);
	
	
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
	/* Interaction Timer */
	
protected:
	/** @return if a timer is active for this channel */
	virtual bool HasActiveTimerForChannel(const FGameplayTag InteractionChannel);
	/** @return pointer to timer handle struct of the active timer for this channel */
	virtual FXInUInteractionTimerHandle* GetActiveTimerByChannel(const FGameplayTag InteractionChannel);
	/** @return true if there is a timer active AND it is client only */
	virtual bool HasClientOnlyTimer(const FGameplayTag InteractionChannel);
protected:
	virtual void StartInteractionTimer(AActor* Interactable, const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag, const float Timer, const
	                                   bool bClientOnly);
	virtual void StopInteractionTimer(const FGameplayTag InteractionChannel);
	virtual void InteractionTimerEnded(AActor* Interactable, const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag, const bool bClientOnly);
private:
	/** Maps the interaction channel tag to a timer handle (the timer delegate already has data about the
	 * interactable and interaction tag, so we just use a timer handle per channel) */
	TMap<FGameplayTag, FXInUInteractionTimerHandle> InteractionTimer;

/*--------------------------------------------------------------------------------------------------------------------*/
	
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * Debug
	 */

private:
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDebugArrayCount = false;
	
};
