// Copyright XyloIsCoding 2024


#include "Interaction/Interactable/Data/XInUInteractableData.h"

const TArray<FXInUInteractionData>& UXInUInteractableData::GetPossibleInteractions()
{
	return PossibleInteractions;
}

bool UXInUInteractableData::GetInteractionData(const FGameplayTag InteractionTag, FXInUInteractionData& OutInteractionData)
{
	for (const FXInUInteractionData& InteractionData : PossibleInteractions)
	{
		if (InteractionData.InteractionTag.MatchesTagExact(InteractionTag))
		{
			OutInteractionData = InteractionData;
			return true;
		}
	}
	return false;
}



