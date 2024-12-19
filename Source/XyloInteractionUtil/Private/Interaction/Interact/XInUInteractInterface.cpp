// Copyright XyloIsCoding 2024


#include "Interaction/Interact/XInUInteractInterface.h"

#include "XInUGameplayTags.h"


bool IXInUInteractInterface::CanInteract_Implementation(AActor* InteractableActor, FGameplayTag InteractionTag, FGameplayTag& OutStatusTag)
{
	OutStatusTag = TAG_XInU_InteractionStatusTag_Invalid;
	return false;
}

bool IXInUInteractInterface::TryInteract_Implementation(AActor* InteractableActor, FGameplayTag InteractionTag)
{
	return false;
}
