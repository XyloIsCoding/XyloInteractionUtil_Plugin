// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactable/XInUInteractableData.h"

FXInUInteractionSettings* UXInUInteractableData::GetInteractionSettings(const FGameplayTag& Action)
{
	return InteractionSettings.Find(Action);
}
