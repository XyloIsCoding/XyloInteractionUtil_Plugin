// Copyright 2025, Davide Bosco. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "XInUInteractionTypes.h"
#include "Components/ActorComponent.h"
#include "XInUInteractorComponent.generated.h"

struct FXInUInteractionInfo;
struct FXInUInteractionState;
class UXInUInteractorComponent;
struct FXInUInteractionKey;
struct FGameplayTag;

USTRUCT()
struct FXInUInteractableList
{
	GENERATED_BODY()

	struct FInteractables
	{
		TArray<TWeakObjectPtr<AActor>> Interactables;
	};

public:
	bool RegisterInteractable(AActor* Interactable);
	bool UnRegisterInteractable(AActor* Interactable);
	void GetChannels(TArray<FGameplayTag>& OutChannels);
	bool GetAvailable(const FGameplayTag& Channel, TArray<AActor*>& OutInteractables);
	bool IsAvailable(const FGameplayTag& Channel, AActor* Interactable) const;
private:
	TMap<FGameplayTag, FInteractables> Available;
	TMap<FGameplayTag, FInteractables> UnAvailable;
};

UENUM()
enum class EXInUInteractionTimerStatus : uint8
{
	ETS_None,
	ETS_Inactive,
	ETS_ClientSide,
	ETS_ServerAuth
};

USTRUCT()
struct FXInUSelectedInteractable
{
	GENERATED_BODY()

	struct FSelected
	{
		TWeakObjectPtr<AActor> Interactable;
		FTimerHandle InteractionTimerHandle;
		// Should only be trusted if InteractionTimerHandle is active
		bool bClientOnlyTimer;
	};
	
public:
	AActor* GetSelected(const FGameplayTag& Channel);
	bool IsSelected(const FGameplayTag& Channel, AActor* Interactable);
	void UpdateSelection(const FGameplayTag& Channel, AActor* Interactable);
	void InvalidateSelection(const FGameplayTag& Channel);
	bool StartInteractionTimer(const FGameplayTag& Channel, const FTimerDelegate& TimerDelegate, float Duration, bool bClientOnly);
	bool StopInteractionTimer(const FGameplayTag& Channel);
	float GetInteractionProgress(const FGameplayTag& Channel) const;
	EXInUInteractionTimerStatus GetInteractionTimerStatus(const FGameplayTag& Channel);
private:
	TMap<FGameplayTag, FSelected> Selected;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*--------------------------------------------------------------------------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Manages interactions with UXInUInteractableComponent.
 * <p> The owner of this component must implement IXInUInteractorInterface </p>
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class XYLOINTERACTIONUTIL_API UXInUInteractorComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UXInUInteractorComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * UActorComponent Interface
	 */
	
protected:
	virtual void BeginPlay() override;
	virtual void OnRegister() override;
public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	bool IsOwnerLocallyControlled() const;
	bool HasAuthority() const;
	
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * UXInUInteractorComponent Interface
	 */

public:
	void ResetInteractionState(AActor* Interactable, const FGameplayTag& Channel);
	void UpdateInteractionState(const FXInUInteractionInfo& InteractionInfo);
	FXInUInteractionInfoResetSignature InteractionInfoResetDelegate;
	FXInUInteractionInfoSignature InteractionInfoDelegate;
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	float GetInteractionProgress(const FGameplayTag& Channel) const;

public:
	/** Register an interactable actor to this component
	 * @remark NOT replicated. Has to be called from both client and server */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void RegisterInteractable(AActor* Interactable);
	/** Unregister an interactable actor to this component 
	 * @remark NOT replicated. Has to be called from both client and server */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void UnRegisterInteractable(AActor* Interactable);
protected:
	void UpdateInteractableAvailability(AActor* Interactable, bool bAvailable);
private:
	FXInUInteractableList InteractablesInRage;

protected:
	void Local_UpdateSelection();
	void Local_UpdateSelectionForChannel(const FGameplayTag& Channel);
	void Local_UpdateSelectionInternal(const FGameplayTag& Channel, const FVector& AimLocation, const FVector& AimDirection);
	virtual void Local_UpdateInteractableStatus(const FGameplayTag& Channel, AActor* Interactable, bool bSelected);
private:
	/** @remark Only filled if locally controlled. On non locally controlled authority, values are only updated
	 *			when starting an interaction */
	FXInUSelectedInteractable SelectedInteractables;

/*--------------------------------------------------------------------------------------------------------------------*/
	/* Interaction Management */

public:
	/** Start an interaction.
	 * @remark should be called from locally controlled actors */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	virtual void InputStartInteraction(const FGameplayTag Channel, const FGameplayTag Action);
	/** Stop an interaction. should be called from locally controlled actors.
	 * @remark should be called from locally controlled actors */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	virtual void InputStopInteraction(const FGameplayTag Channel);
protected:
	UFUNCTION(Server, Reliable)
	virtual void ServerStartInteractionRPC(AActor* Interactable, const FGameplayTag& Channel, const FGameplayTag& Action);
	UFUNCTION(Server, Reliable)
	virtual void ServerStopInteractionRPC(AActor* Interactable, const FGameplayTag& Channel);
	
	virtual bool StartInteraction(AActor* Interactable, const FGameplayTag& Channel, const FGameplayTag& Action);
	virtual void StopInteraction(AActor* Interactable, const FGameplayTag& Channel);

	virtual bool DelayedInteractionStart(AActor* Interactable, const FGameplayTag& Channel, const FGameplayTag& Action, float Duration, const bool bClientOnly);
	virtual void InteractionTimerEnded(AActor* Interactable, const FGameplayTag Channel, const FGameplayTag Action, const bool bClientOnly);
	UFUNCTION(Server, Reliable)
	virtual void ServerInteractFromTimerRPC(AActor* Interactable, const FGameplayTag& Channel, const FGameplayTag& Action);
	virtual void InteractFromTimer(AActor* Interactable, const FGameplayTag& Channel, const FGameplayTag& Action);

	//~ Interaction Management
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
	/* Interaction Logic */

protected:
	/** @return true if OutState is filled */
	virtual bool CheckInteraction(AActor* Interactable, const FGameplayTag& Channel, const FGameplayTag& Action, FXInUInteractionState& OutState);
	virtual void Interact(AActor* Interactable, const FGameplayTag& Channel, const FGameplayTag& Action);

	//~ Interaction Logic
/*--------------------------------------------------------------------------------------------------------------------*/

	
};

