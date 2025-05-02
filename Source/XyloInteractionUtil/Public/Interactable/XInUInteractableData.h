// Copyright 2025, Davide Bosco. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "XInUInteractableData.generated.h"


enum class EXInUInteractableUnselectedBehaviour : uint8;

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
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EXInUInteractableUnselectedBehaviour UnselectedBehaviour;
private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess))
	TMap<FGameplayTag, FXInUInteractionSettings> InteractionSettings;

public:
	FXInUInteractionSettings* GetInteractionSettings(const FGameplayTag& Action);
	void GetSupportedActions(FGameplayTagContainer& Actions) const;
};
