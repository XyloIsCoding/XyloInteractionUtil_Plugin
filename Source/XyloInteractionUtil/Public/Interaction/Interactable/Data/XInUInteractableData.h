// Copyright XyloIsCoding 2024

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "XInUInteractableData.generated.h"


/**
 * 
 */
UCLASS()
class XYLOINTERACTIONUTIL_API UXInUInteractableData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag InteractionChannelTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer PossibleInteractions;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	FGameplayTagContainer GetInteractionTags();
};
