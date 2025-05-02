// Fill out your copyright notice in the Description page of Project Settings.

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
		bool bClientOnlyTimer;
	};
	
public:
	AActor* GetSelected(const FGameplayTag& Channel);
	bool IsSelected(const FGameplayTag& Channel, AActor* Interactable);
	void UpdateSelection(const FGameplayTag& Channel, AActor* Interactable);
	void InvalidateSelection(const FGameplayTag& Channel, AActor* Interactable);
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
 *
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
public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

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
	void RegisterInteractable(AActor* Interactable);
	void UnRegisterInteractable(AActor* Interactable);
	void UpdateInteractableAvailability(AActor* Interactable, bool bAvailable);
private:
	FXInUInteractableList InteractablesInRage;

protected:
	void UpdateSelection();
	void UpdateSelectionForChannel(const FGameplayTag& Channel);
	void UpdateSelectionInternal(const FGameplayTag& Channel, const FVector& AimLocation, const FVector& AimDirection);
	virtual void UpdateInteractableStatus(const FGameplayTag& Channel, AActor* Interactable, bool bSelected);
private:
	FXInUSelectedInteractable SelectedInteractables;

/*--------------------------------------------------------------------------------------------------------------------*/
	/* Interaction Management */

public:
	/** Function to call to start an interaction. should be called from locally controlled actors.
	 * (Calls ExecuteInteraction, and if not authority calls ServerInteractRPC) */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	virtual void InputStartInteraction(const FGameplayTag& Channel, const FGameplayTag& Action);
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	virtual void InputStopInteraction(const FGameplayTag& Channel);
protected:
	UFUNCTION(Server, Reliable)
	virtual void ServerStartInteractionRPC(AActor* Interactable, const FGameplayTag& Channel, const FGameplayTag& Action);
	UFUNCTION(Server, Reliable)
	virtual void ServerStopInteractionRPC(AActor* Interactable, const FGameplayTag& Channel);
	
	virtual bool StartInteraction(AActor* Interactable, const FGameplayTag& Channel, const FGameplayTag& Action);
	virtual void StopInteraction(AActor* Interactable, const FGameplayTag& Channel);

	virtual bool StartInteractionWithDuration(AActor* Interactable, const FGameplayTag& Channel, const FGameplayTag& Action, float Duration, const bool bClientOnly);
	virtual void InteractionTimerEnded(AActor* Interactable, const FGameplayTag Channel, const FGameplayTag Action, const bool bClientOnly);
	virtual void InteractFromTimer(AActor* Interactable, const FGameplayTag& Channel, const FGameplayTag& Action);
	UFUNCTION(Server, Reliable)
	virtual void ServerInteractFromTimerRPC(AActor* Interactable, const FGameplayTag& Channel, const FGameplayTag& Action);

	//~ Interaction Management
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
	/* Interaction Logic */

protected:
	virtual bool CheckInteraction(AActor* Interactable, const FGameplayTag& Channel, const FGameplayTag& Action, FXInUInteractionState OutState);
	virtual void Interact(AActor* Interactable, const FGameplayTag& Channel, const FGameplayTag& Action);

	//~ Interaction Logic
/*--------------------------------------------------------------------------------------------------------------------*/

	
};

