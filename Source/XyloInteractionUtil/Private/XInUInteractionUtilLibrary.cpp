// Fill out your copyright notice in the Description page of Project Settings.


#include "XInUInteractionUtilLibrary.h"

#include "Interact/XInUInteractInterface.h"
#include "Interactable/XInUInteractableInterface.h"

UXInUInteractableComponent* UXInUInteractionUtilLibrary::GetInteractableComponent(AActor* InteractableActor)
{
	IXInUInteractableInterface* InteractableInterface = Cast<IXInUInteractableInterface>(InteractableActor);
	if (!InteractableInterface) return nullptr;
	
	return InteractableInterface->GetInteractableComponent();
}

bool UXInUInteractionUtilLibrary::GetInteractionAimTransform(AActor* InteractActor, FTransform& AimTransform)
{
	IXInUInteractInterface* InteractInterface = Cast<IXInUInteractInterface>(InteractActor);
	if (!InteractInterface) return nullptr;
	
	return InteractInterface->GetInteractionAimTransform(AimTransform);
}
