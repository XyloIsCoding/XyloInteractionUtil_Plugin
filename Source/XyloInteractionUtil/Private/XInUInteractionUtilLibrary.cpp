// Fill out your copyright notice in the Description page of Project Settings.


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

FXInUInteractionState UXInUInteractionUtilLibrary::GetInteractionStateFromInfo(const FXInUInteractionInfo& InteractionInfo, const FGameplayTag& Action)
{
	return *InteractionInfo.GetInteractionState(Action);
}
