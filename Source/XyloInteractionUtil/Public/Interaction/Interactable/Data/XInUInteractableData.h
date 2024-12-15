// Copyright XyloIsCoding 2024

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "XInUInteractableData.generated.h"

USTRUCT(BlueprintType)
struct FXInUInteractionData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FGameplayTag InteractionTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float InteractionTime;
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
	FGameplayTag InteractionChannelTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FXInUInteractionData> PossibleInteractions;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	const TArray<FXInUInteractionData>& GetPossibleInteractions();

	bool GetInteractionData(const FGameplayTag InteractionTag, FXInUInteractionData& OutInteractionData);
};
