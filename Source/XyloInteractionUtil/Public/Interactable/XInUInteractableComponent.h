// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XInUInteractionTypes.h"
#include "Components/ActorComponent.h"
#include "XInUInteractableComponent.generated.h"

class UXInUInteractableData;
struct FGameplayTag;

DECLARE_MULTICAST_DELEGATE_TwoParams(FAvailabilityChangedSignature, AActor*, bool)

/**
 *
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class XYLOINTERACTIONUTIL_API UXInUInteractableComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UXInUInteractableComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

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
	 * UXInUInteractableComponent Interface
	 */

public:
	FAvailabilityChangedSignature AvailabilityChangedDelegate;
	void SetAvailable(bool bAvailable);
	bool IsAvailable() const { return bAvailableForInteraction; }
private:
	bool bAvailableForInteraction = true;

public:
	bool GetInteractionChannel(FGameplayTag& OutChannel) const;
	
	float GetInteractionDuration(const FGameplayTag& Action) const;
	float IsInteractionDurationClientSideOnly(const FGameplayTag& Action) const;
private:
	UPROPERTY()
	TObjectPtr<UXInUInteractableData> InteractableData;

public:
	void UpdateInteractionTimerData(const FXInUInteractionTimerData& NewTimerData);
	void ResetInteractionTimerData();
private:
	FXInUInteractionTimerData InteractionTimerData;
};
