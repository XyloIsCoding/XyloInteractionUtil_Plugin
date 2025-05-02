// Copyright 2025, Davide Bosco. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "XInUInteractionTypes.generated.h"

USTRUCT(BlueprintType)
struct FXInUInteractionTimerData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FGameplayTag Action;
	UPROPERTY(BlueprintReadWrite)
	float StartTime;
	UPROPERTY(BlueprintReadWrite)
	float Duration;

	void Reset();
};

/** Result of IXInUInteractionInterface::CanInteract
 * <p> bCanInteract needs to be set to true to allow an interaction to happen
 * <p> InteractionStatus is optional, and mostly used to add extra informations to FXInUInteractionInfo */
USTRUCT(BlueprintType)
struct FXInUInteractionState
{
	GENERATED_BODY()

	/** Decides if the interaction can happen or not */
	UPROPERTY(BlueprintReadWrite)
	bool bCanInteract = false;
	UPROPERTY(BlueprintReadWrite)
	FGameplayTag InteractionStatus;
};

USTRUCT(BlueprintType)
struct FXInUInteractionInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	AActor* Interactor = nullptr;
	UPROPERTY(BlueprintReadWrite)
	AActor* Interactable = nullptr;
	UPROPERTY(BlueprintReadWrite)
	FGameplayTag Channel;
	UPROPERTY(BlueprintReadWrite)
	bool bSelected = false;
private:
	UPROPERTY()
	TMap<FGameplayTag, FXInUInteractionState> InteractionsState;

public:
	FXInUInteractionState* GetInteractionState(const FGameplayTag& Action);
	const FXInUInteractionState* GetInteractionStateConst(const FGameplayTag& Action) const;
	void AddInteractionState(const FGameplayTag& Action, const FXInUInteractionState& State);
};

DECLARE_MULTICAST_DELEGATE_OneParam(FXInUInteractionInfoSignature, const FXInUInteractionInfo&)
DECLARE_MULTICAST_DELEGATE_TwoParams(FXInUInteractionInfoResetSignature, AActor*, FGameplayTag)

UENUM(BlueprintType)
enum class EXInUInteractableUnselectedBehaviour : uint8
{
	EUB_SkipInteractions UMETA(DisplayName = "Skip Interactions") /* Do not fill FXInUInteractionState in FXInUInteractionInfo if not selected */,
	EUB_ComputeInteractions UMETA(DisplayName = "Compute Interactions") /* Fill FXInUInteractionState in FXInUInteractionInfo even if not selected */,
};