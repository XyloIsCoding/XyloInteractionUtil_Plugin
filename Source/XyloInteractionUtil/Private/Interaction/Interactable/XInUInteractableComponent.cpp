// Copyright XyloIsCoding 2024


#include "Interaction/Interactable/XInUInteractableComponent.h"

#include "Blueprint/UserWidget.h"
#include "Interaction/XInUInteractionInfo.h"
#include "Interaction/Interactable/Data/XInUInteractableData.h"
#include "Interaction/Interact/XInUInteractComponent.h"
#include "Interaction/Interact/XInUInteractInterface.h"
#include "Net/UnrealNetwork.h"

UXInUInteractableComponent::UXInUInteractableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	bAvailableForInteraction = true;
	UnselectedBehaviour = EXInUInteractableUnselectedBehaviour::EUB_SkipInteractions;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * UActorComponent Interface
 */

void UXInUInteractableComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UXInUInteractableComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UXInUInteractableComponent, bAvailableForInteraction);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Interactable
 */

void UXInUInteractableComponent::SetAvailableForInteraction(bool bAvailable)
{
	bAvailableForInteraction = bAvailable;
	OnRep_AvailableForInteraction();
}

void UXInUInteractableComponent::OnRep_AvailableForInteraction()
{
	AvailableForInteractionDelegate.Broadcast(GetOwner(), bAvailableForInteraction);
}

FGameplayTag UXInUInteractableComponent::GetInteractionChannelTag() const
{
	if (InteractableData)
	{
		return InteractableData->InteractionChannelTag;
	}
	return FGameplayTag();
}

void UXInUInteractableComponent::OnEnterInteractRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult,
	bool bInstantInteraction, FGameplayTag InstantInteractionTag)
{
	if (IXInUInteractInterface* InteractInterface = Cast<IXInUInteractInterface>(OtherActor))
	{
		if (UXInUInteractComponent* InteractComponent = InteractInterface->Execute_GetInteractComponent(OtherActor))
		{
			InteractComponent->AddInteractableInRange(GetOwner());
		}
	}
}

void UXInUInteractableComponent::OnExitInteractRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (IXInUInteractInterface* InteractInterface = Cast<IXInUInteractInterface>(OtherActor))
	{
		if (UXInUInteractComponent* InteractComponent = InteractInterface->Execute_GetInteractComponent(OtherActor))
		{
			InteractComponent->RemoveInteractableInRange(GetOwner());
		}
	}
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* InteractionInfoDelegates */

void UXInUInteractableComponent::ResetInteractionEntries(const FGameplayTag InteractionChannel, AActor* Interactable)
{
	Super::ResetInteractionEntries(InteractionChannel, Interactable);
	ResetInteractionTimerData();
}

void UXInUInteractableComponent::UpdateInteractionEntries(const FXInUInteractionInfo& InteractionInfo)
{
	Super::UpdateInteractionEntries(InteractionInfo);
	if (!InteractionInfo.bSelected)
	{
		ResetInteractionTimerData();
	}
}

/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Interaction Timer */

float UXInUInteractableComponent::GetInteractionMaxTime(const FGameplayTag InteractionChannel)
{
	return InteractionTimer.Duration;
}

float UXInUInteractableComponent::GetInteractionTimeElapsed(const FGameplayTag InteractionChannel)
{
	if (InteractionTimer.StartTime >= 0.f)
	{
		return GetWorld()->GetTimeSeconds() - InteractionTimer.StartTime;
	}
	return -1.f;
}

float UXInUInteractableComponent::GetInteractionTimeLeft(const FGameplayTag InteractionChannel)
{
	if (InteractionTimer.StartTime >= 0.f && InteractionTimer.Duration >= 0.f)
	{
		return FMath::Max((InteractionTimer.StartTime + InteractionTimer.Duration) - GetWorld()->GetTimeSeconds(), 0.f);
	}
	return -1.f;
}

void UXInUInteractableComponent::UpdateInteractionTimerData(const FXInUInteractionTimerData& NewInteractionTimer)
{
	InteractionTimer.InteractionTag = NewInteractionTimer.InteractionTag;
	InteractionTimer.StartTime = NewInteractionTimer.StartTime;
	InteractionTimer.Duration = NewInteractionTimer.Duration;
}

void UXInUInteractableComponent::ResetInteractionTimerData()
{
	InteractionTimer.StartTime = -1.f;
	InteractionTimer.Duration = -1.f;
}

/*--------------------------------------------------------------------------------------------------------------------*/

