// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "XInUBaseInteractionComponent.generated.h"


struct FXInUInteractionInfo;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUpdateInteractionEntriesSignature, const FXInUInteractionInfo&,  InteractionInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FResetInteractionEntriesSignature, const FGameplayTag, InteractionChannel, AActor*, Interactable);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class XYLOINTERACTIONUTIL_API UXInUBaseInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UXInUBaseInteractionComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * UActorComponent Interaction
	 */
	
protected:
	virtual void BeginPlay() override;
	
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * Base Interaction
	 */

/*--------------------------------------------------------------------------------------------------------------------*/
	/* InteractionWidget */
	
public:
	/** Broadcasts ResetInteractionWidgetDelegate */
	virtual void ResetInteractionEntries(const FGameplayTag InteractionChannel, AActor* Interactable);
	/** Broadcasts AddEntryToInteractionWidgetDelegate with interaction widget */
	virtual void UpdateInteractionEntries(const FXInUInteractionInfo& InteractionInfo);

public:
	/** Broadcast when interactions are updated for an in-range interactable */
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FUpdateInteractionEntriesSignature UpdateInteractionEntriesDelegate;
	/** Broadcast when an interactable gets out of range */
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FResetInteractionEntriesSignature ResetInteractionEntriesDelegate;

/*--------------------------------------------------------------------------------------------------------------------*/
	
};
