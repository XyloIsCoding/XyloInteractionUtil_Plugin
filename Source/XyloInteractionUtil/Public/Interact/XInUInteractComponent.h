// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "XInUInteractComponent.generated.h"

class UXInUInteractComponent;
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
private:
	TMap<FGameplayTag, FInteractables> Available;
	TMap<FGameplayTag, FInteractables> UnAvailable;
};

USTRUCT()
struct FXInUSelectedInteractable
{
	GENERATED_BODY()

	struct FSelected
	{
		TWeakObjectPtr<AActor> Interactable;
		FTimerHandle InteractionTimerHandle;
		bool IsClientOnlyTimer;
	};
	
public:
	void UpdateSelection(const FGameplayTag& Channel, AActor* Interactable);
	AActor* GetSelected(const FGameplayTag& Channel);
	bool StopInteractionTimer(const FGameplayTag& Channel);
	bool IsInteractionTimerActive(const FGameplayTag& Channel);
	bool IsInteractionTimerClientSide(const FGameplayTag& Channel); // TODO: does not really have meaning without a HasTimer check
	bool IsInteractionTimerActiveClientSide(const FGameplayTag& Channel); // TODO: finish
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
class XYLOINTERACTIONUTIL_API UXInUInteractComponent : public UActorComponent
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
	 * UXInUInteractComponent Interface
	 */

public:
	void RegisterInteractable(AActor* Interactable);
	void UnRegisterInteractable(AActor* Interactable);
	void UpdateInteractableAvailability(AActor* Interactable, bool bAvailable);
private:
	FXInUInteractableList InteractablesInRage;

protected:
	void UpdateSelection();
	virtual void UpdateInteractableStatus(const FGameplayTag& Channel, AActor* Interactable, bool bSelected);
private:
	FXInUSelectedInteractable SelectedInteractables;

/*--------------------------------------------------------------------------------------------------------------------*/
	/* Interaction */

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
	virtual bool StartInteractionWithDuration(AActor* Interactable, const FGameplayTag& Channel, const FGameplayTag& Action, const float InteractionTime);
	virtual void StopInteraction(AActor* Interactable, const FGameplayTag& Channel);
	
	virtual void Interact(AActor* Interactable, const FGameplayTag& Channel, const FGameplayTag& Action);
	virtual void InteractFromTimer(AActor* Interactable, const FGameplayTag& Channel, const FGameplayTag& Action);
	UFUNCTION(Server, Reliable)
	virtual void ServerInteractFromTimerRPC(AActor* Interactable, const FGameplayTag& Channel, const FGameplayTag& Action);

	//~ Interaction
/*--------------------------------------------------------------------------------------------------------------------*/
	
	
};

