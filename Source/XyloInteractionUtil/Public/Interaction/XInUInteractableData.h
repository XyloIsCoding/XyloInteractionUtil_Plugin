// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "XInUInteractableData.generated.h"


struct FXInUInteractionInfo;
enum class EXInUInteractableStatus : uint8;

USTRUCT(BlueprintType)
struct FXInUInteractionStatusDataRow
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag InteractionStatusTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UUserWidget> InteractionWidgetClass;
};

USTRUCT(BlueprintType)
struct FXInUInteractionDataRow
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag InteractionTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FXInUInteractionStatusDataRow> StatusData;
};


/**
 * 
 */
UCLASS()
class XYLOINTERACTIONUTIL_API UXInUInteractableData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UUserWidget> DefaultInteractionWidgetClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FXInUInteractionDataRow> InteractionRows;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	FGameplayTagContainer GetInteractionTags();
	
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	TSubclassOf<UUserWidget> GetInteractionWidgetClass(const FGameplayTag InteractionTag, const FGameplayTag InteractionStatusTag);
	
};
