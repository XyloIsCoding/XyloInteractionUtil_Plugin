// Copyright XyloIsCoding 2024


#include "Interaction/XInUInteractableData.h"

FGameplayTagContainer UXInUInteractableData::GetInteractionTags()
{
	FGameplayTagContainer InteractionTags;
	for (FXInUInteractionDataRow DataRow : InteractionRows)
	{
		InteractionTags.AddTag(DataRow.InteractionTag);
	}
	return InteractionTags;
}

TSubclassOf<UUserWidget> UXInUInteractableData::GetInteractionWidgetClass(const FGameplayTag InteractionTag, const FGameplayTag InteractionStatusTag)
{
	// look for the interaction data for the interaction tag
	for (FXInUInteractionDataRow InteractionDataRow : InteractionRows)
	{
		if (InteractionDataRow.InteractionTag.MatchesTagExact(InteractionTag))
		{
			// look for the status data for the status tag
			for (FXInUInteractionStatusDataRow StatusDataRow : InteractionDataRow.StatusData)
			{
				if (StatusDataRow.InteractionStatusTag.MatchesTagExact(InteractionStatusTag))
				{
					// return the widget
					return StatusDataRow.InteractionWidgetClass;
				}
			}
			// no status data found for the status tag.
			// break loop, because there is no need to keep going
			break;
		}
	}
	return nullptr;
}


