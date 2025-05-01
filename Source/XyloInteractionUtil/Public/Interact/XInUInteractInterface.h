// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "XInUInteractInterface.generated.h"

class UXInUInteractComponent;
// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UXInUInteractInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class XYLOINTERACTIONUTIL_API IXInUInteractInterface
{
	GENERATED_BODY()

public:
	virtual UXInUInteractComponent* GetInteractComponent() const = 0;
	virtual bool GetInteractionAimTransform(FTransform& AimTransform) const = 0;
};
