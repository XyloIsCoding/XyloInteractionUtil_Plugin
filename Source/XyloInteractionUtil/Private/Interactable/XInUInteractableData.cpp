// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactable/XInUInteractableData.h"

FXInUInteractionSettings* UXInUInteractableData::GetInteractionSettings(const FGameplayTag& Action)
{
	return InteractionSettings.Find(Action);
}

void UXInUInteractableData::GetSupportedActions(FGameplayTagContainer& Actions) const
{
	TArray<FGameplayTag> ActionTags;
	InteractionSettings.GetKeys(ActionTags);
	for (const FGameplayTag& Tag : ActionTags)
	{
		Actions.AddTag(Tag);
	}
}
