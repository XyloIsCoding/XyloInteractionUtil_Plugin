// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "XInUInteractableData.generated.h"


USTRUCT(BlueprintType)
struct FXInUInteractionSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float InteractionDuration;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bClientOnlyInteractionDuration;
};


/**
 * 
 */
UCLASS()
class XYLOINTERACTIONUTIL_API UXInUInteractableData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag InteractionChannel;
private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FGameplayTag, FXInUInteractionSettings> InteractionSettings;

public:
	FXInUInteractionSettings* GetInteractionSettings(const FGameplayTag& Action);
	
};
