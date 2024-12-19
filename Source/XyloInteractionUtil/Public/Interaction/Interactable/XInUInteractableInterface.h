// Copyright XyloIsCoding 2024

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "XInUInteractableInterface.generated.h"

struct FGameplayTag;
class UXInUInteractableComponent;

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UXInUInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class XYLOINTERACTIONUTIL_API IXInUInteractableInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
	UXInUInteractableComponent* GetInteractableComponent() const;
	virtual UXInUInteractableComponent* GetInteractableComponent_Implementation() const = 0;
};
