// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactable/XInUInteractableComponent.h"

#include "GameplayTagContainer.h"
#include "Interactable/XInUInteractableData.h"


UXInUInteractableComponent::UXInUInteractableComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * UActorComponent Interface
 */

void UXInUInteractableComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UXInUInteractableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * UXInUInteractableComponent Interface
 */

void UXInUInteractableComponent::SetAvailable(bool bAvailable)
{
	bAvailableForInteraction = bAvailable;
	AvailabilityChangedDelegate.Broadcast(GetOwner(), bAvailableForInteraction);
}

bool UXInUInteractableComponent::GetInteractionChannel(FGameplayTag& OutChannel) const
{
	if (!InteractableData || !InteractableData->InteractionChannel.IsValid()) return false;
	OutChannel = InteractableData->InteractionChannel;
	return true;
}

float UXInUInteractableComponent::GetInteractionDuration(const FGameplayTag& Action) const
{
	if (!InteractableData) return -1.f;
	FXInUInteractionSettings* InteractionSettings = InteractableData->GetInteractionSettings(Action);
	if (!InteractionSettings) return -1.f;
	
	return InteractionSettings->InteractionDuration;
}

float UXInUInteractableComponent::IsInteractionDurationClientSideOnly(const FGameplayTag& Action) const
{
	if (!InteractableData) return false;
	FXInUInteractionSettings* InteractionSettings = InteractableData->GetInteractionSettings(Action);
	if (!InteractionSettings) return false;

	return InteractionSettings->bClientOnlyInteractionDuration;
}

void UXInUInteractableComponent::UpdateInteractionTimerData(const FXInUInteractionTimerData& NewTimerData)
{
	InteractionTimerData = NewTimerData;
}

void UXInUInteractableComponent::ResetInteractionTimerData()
{
	InteractionTimerData.Reset();
}
