// Copyright 2025, Davide Bosco. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "XInUInteractorInterface.generated.h"

class UXInUInteractorComponent;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UXInUInteractorInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class XYLOINTERACTIONUTIL_API IXInUInteractorInterface
{
	GENERATED_BODY()

public:
	virtual UXInUInteractorComponent* GetInteractorComponent() const = 0;
	virtual bool GetInteractionAimTransform(FTransform& AimTransform) const = 0;
};
