// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "XInUBaseInteractionComponent.generated.h"


USTRUCT(BlueprintType)
struct FXInUInteractionType
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FGameplayTag InteractionTag;
	UPROPERTY(BlueprintReadWrite)
	FGameplayTag InteractionStatus;
};

USTRUCT(BlueprintType)
struct FXInUInteractionInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	AActor* Interactable;
	UPROPERTY(BlueprintReadWrite)
	FGameplayTag InteractionChannel;
	UPROPERTY(BlueprintReadWrite)
	TArray<FXInUInteractionType> Interactions;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FShowInteractionWidgetSignature, const bool, ShowInteractionWidget);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAddInteractionEntrySignature, const FXInUInteractionInfo&, InteractionInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FResetInteractionEntriesSignature, const FGameplayTag, InteractionChannel);

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
	/** Broadcasts ShowInteractionWidgetDelegate */
	virtual void ShowInteractionWidget(const bool bShow);
	/** Broadcasts ResetInteractionWidgetDelegate */
	virtual void ResetInteractionEntries(const FGameplayTag InteractionChannel);
	/** Broadcasts AddEntryToInteractionWidgetDelegate with interaction widget */
	virtual void AddInteractionEntry(const FXInUInteractionInfo& InteractionInfo);

public:
	/** Used to bind function to show/hide the interaction widget */
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FShowInteractionWidgetSignature ShowInteractionWidgetDelegate;
	/** Used to bind function to add interaction widget rows to the interaction widget */
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FAddInteractionEntrySignature AddInteractionEntryDelegate;
	/** Used to bind function to remove all widget rows from the interaction widget */
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FResetInteractionEntriesSignature ResetInteractionEntriesDelegate;

/*--------------------------------------------------------------------------------------------------------------------*/
	
};
