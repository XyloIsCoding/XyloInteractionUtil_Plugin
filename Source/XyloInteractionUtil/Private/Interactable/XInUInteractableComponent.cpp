// Copyright 2025, Davide Bosco. All Rights Reserved.


#include "Interactable/XInUInteractableComponent.h"

#include "GameplayTagContainer.h"
#include "Interactable/XInUInteractableData.h"
#include "Interactable/XInUInteractableInterface.h"
#include "Net/UnrealNetwork.h"


UXInUInteractableComponent::UXInUInteractableComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
}

void UXInUInteractableComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, bAvailableForInteraction);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * UActorComponent Interface
 */

void UXInUInteractableComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UXInUInteractableComponent::OnRegister()
{
	Super::OnRegister();

	if (GetOwner())
	{
		if (!GetOwner()->Implements<UXInUInteractableInterface>())
		{
			UE_LOG(LogTemp, Error, TEXT("[%s] UXInUInteractableComponent's owner must implement IXInUInteractableInterface"), *GetOwner()->GetName())
		}
		if (!InteractableData)
		{
			UE_LOG(LogTemp, Error, TEXT("[%s] UXInUInteractableComponent needs a valid InteractableData to be set"), *GetOwner()->GetName())
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * UXInUInteractableComponent Interface
 */
 
void UXInUInteractableComponent::ResetInteractionState(AActor* Interactor)
{
	InteractionResetDelegate.Broadcast(Interactor, InteractableData ? InteractableData->InteractionChannel : FGameplayTag());
}

void UXInUInteractableComponent::UpdateInteractionState(const FXInUInteractionInfo& InteractionInfo)
{
	InteractionInfoDelegate.Broadcast(InteractionInfo);
}

float UXInUInteractableComponent::GetInteractionProgress() const
{
	return (GetWorld()->GetTimeSeconds() - InteractionTimerData.StartTime) / InteractionTimerData.Duration;
}

void UXInUInteractableComponent::SetAvailable(bool bAvailable)
{
	bAvailableForInteraction = bAvailable;
	AvailabilitySet();
}

void UXInUInteractableComponent::OnRep_AvailableForInteraction()
{
	AvailabilitySet();
}

void UXInUInteractableComponent::AvailabilitySet()
{
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

bool UXInUInteractableComponent::IsInteractionDurationClientSideOnly(const FGameplayTag& Action) const
{
	if (!InteractableData) return false;
	FXInUInteractionSettings* InteractionSettings = InteractableData->GetInteractionSettings(Action);
	if (!InteractionSettings) return false;

	return InteractionSettings->bClientOnlyInteractionDuration;
}

bool UXInUInteractableComponent::GetSupportedActions(FGameplayTagContainer& Actions) const
{
	if (!InteractableData) return false;
	InteractableData->GetSupportedActions(Actions);
	return true;
}

EXInUInteractableUnselectedBehaviour UXInUInteractableComponent::GetUnselectedBehaviour() const
{
	if (!InteractableData) return EXInUInteractableUnselectedBehaviour::EUB_SkipInteractions;
	return InteractableData->UnselectedBehaviour;
}

void UXInUInteractableComponent::UpdateInteractionTimerData(const FXInUInteractionTimerData& NewTimerData)
{
	InteractionTimerData = NewTimerData;
}

void UXInUInteractableComponent::ResetInteractionTimerData()
{
	InteractionTimerData.Reset();
}
