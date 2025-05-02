// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XInUInteractionTypes.h"
#include "Components/ActorComponent.h"
#include "XInUInteractableComponent.generated.h"

class UXInUInteractableData;
struct FGameplayTag;

DECLARE_MULTICAST_DELEGATE_TwoParams(FXInUAvailabilityChangedSignature, AActor*, bool)

/**
 *
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
	UPROPERTY(ReplicatedUsing = OnRep_AvailableForInteraction)
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
	UPROPERTY(EditAnywhere)
	TObjectPtr<UXInUInteractableData> InteractableData;

public:
	void UpdateInteractionTimerData(const FXInUInteractionTimerData& NewTimerData);
	void ResetInteractionTimerData();
private:
	FXInUInteractionTimerData InteractionTimerData;
};
