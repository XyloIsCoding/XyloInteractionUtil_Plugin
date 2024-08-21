// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/XInUInteractInterface.h"

bool IXInUInteractInterface::CanInteract(AActor* InteractableActor, FGameplayTag InteractionTag, FGameplayTag& OutStatusTag)
{
	return false;
}

bool IXInUInteractInterface::TryInteract(AActor* InteractableActor, FGameplayTag InteractionTag)
{
	return false;
}
