// Copyright 2025, Davide Bosco. All Rights Reserved.


#include "XInUInteractionUtilLibrary.h"


#include "Interactable/XInUInteractableComponent.h"
#include "Interactable/XInUInteractableInterface.h"
#include "Interactor/XInUInteractorInterface.h"

UXInUInteractableComponent* UXInUInteractionUtilLibrary::GetInteractableComponent(AActor* InteractableActor)
{
	IXInUInteractableInterface* InteractableInterface = Cast<IXInUInteractableInterface>(InteractableActor);
	if (!InteractableInterface) return nullptr;
	
	return InteractableInterface->GetInteractableComponent();
}

UXInUInteractorComponent* UXInUInteractionUtilLibrary::GetInteractorComponent(AActor* InteractorActor)
{
	IXInUInteractorInterface* InteractorInterface = Cast<IXInUInteractorInterface>(InteractorActor);
	if (!InteractorInterface) return nullptr;
	
	return InteractorInterface->GetInteractorComponent();
}

bool UXInUInteractionUtilLibrary::GetInteractionAimTransform(AActor* InteractActor, FTransform& AimTransform)
{
	IXInUInteractorInterface* InteractorInterface = Cast<IXInUInteractorInterface>(InteractActor);
	if (!InteractorInterface) return false;
	
	return InteractorInterface->GetInteractionAimTransform(AimTransform);
}

float UXInUInteractionUtilLibrary::GetDefaultInteractionDuration(AActor* InteractableActor, const FGameplayTag& Action)
{
	UXInUInteractableComponent* InteractableComponent = GetInteractableComponent(InteractableActor);
	if (InteractableComponent) return -1.f;

	return InteractableComponent->GetInteractionDuration(Action);
}

bool UXInUInteractionUtilLibrary::GetInteractionStateFromInfo(const FXInUInteractionInfo& InteractionInfo, const FGameplayTag Action, FXInUInteractionState& OutInteractionState)
{
	const FXInUInteractionState* InteractionStatePtr = InteractionInfo.GetInteractionStateConst(Action);
	if (InteractionStatePtr)
	{
		OutInteractionState = *InteractionStatePtr;
	}
	return InteractionStatePtr != nullptr;
}
