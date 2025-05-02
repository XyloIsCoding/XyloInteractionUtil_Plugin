// Copyright 2025, Davide Bosco. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "XInUInteractionTypes.h"
#include "Components/ActorComponent.h"
#include "XInUInteractableComponent.generated.h"

class UXInUInteractableData;
struct FGameplayTag;

DECLARE_MULTICAST_DELEGATE_TwoParams(FXInUAvailabilityChangedSignature, AActor*, bool)

/**
 * Makes the owner able to be interacted with, by UXInUInteractorComponent.
 * <p> The owner of this component must implement IXInUInteractableInterface </p>
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class XYLOINTERACTIONUTIL_API UXInUInteractableComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UXInUInteractableComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * UActorComponent Interface
	 */
	
protected:
	virtual void BeginPlay() override;
	virtual void OnRegister() override;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * UXInUInteractableComponent Interface
	 */

public:
	void ResetInteractionState(AActor* Interactor);
	void UpdateInteractionState(const FXInUInteractionInfo& InteractionInfo);
	FXInUInteractionInfoResetSignature InteractionResetDelegate;
	FXInUInteractionInfoSignature InteractionInfoDelegate;
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	float GetInteractionProgress() const;

public:
	FXInUAvailabilityChangedSignature AvailabilityChangedDelegate;
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetAvailable(bool bAvailable);
	bool IsAvailable() const { return bAvailableForInteraction; }
protected:
	UFUNCTION()
	void OnRep_AvailableForInteraction();
	void AvailabilitySet();
private:
	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_AvailableForInteraction, Category = "Interaction")
	bool bAvailableForInteraction = true;

public:
	bool GetInteractionChannel(FGameplayTag& OutChannel) const;
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	float GetInteractionDuration(const FGameplayTag& Action) const;
	float IsInteractionDurationClientSideOnly(const FGameplayTag& Action) const;
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	bool GetSupportedActions(FGameplayTagContainer& Actions) const;
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	EXInUInteractableUnselectedBehaviour GetUnselectedBehaviour() const;
private:
	/** Defines the properties of this interactable actor */
	UPROPERTY(EditAnywhere, Category = "Interaction")
	TObjectPtr<UXInUInteractableData> InteractableData;

public:
	void UpdateInteractionTimerData(const FXInUInteractionTimerData& NewTimerData);
	void ResetInteractionTimerData();
private:
	/** @remark Only filled locally for the interacting client instance */
	FXInUInteractionTimerData InteractionTimerData;
};
