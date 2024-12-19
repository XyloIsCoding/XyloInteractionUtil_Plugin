// Copyright XyloIsCoding 2024


#include "Interaction/Interactable/XInUInteractableComponent.h"

#include "Blueprint/UserWidget.h"
#include "Interaction/XInUInteractionInfo.h"
#include "Interaction/Interactable/Data/XInUInteractableData.h"
#include "Interaction/Interact/XInUInteractComponent.h"
#include "Interaction/Interact/XInUInteractInterface.h"
#include "Interaction/Interactable/XInUInteractableInterface.h"
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

void UXInUInteractableComponent::OnRegister()
{
	Super::OnRegister();
	
	if (!GetOwner<IXInUInteractableInterface>())
	{
		UE_LOG(LogTemp, Error, TEXT("[UXInUInteractableComponent::OnRegister] The owning class of this component must implement IXInUInteractableInterface"));
	}
}

void UXInUInteractableComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UXInUInteractableComponent, bAvailableForInteraction);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * UXInUBaseInteractionComponent Interface
 */

/*--------------------------------------------------------------------------------------------------------------------*/
/* InteractionInfoDelegates */

void UXInUInteractableComponent::ResetInteractionEntries(AActor* Interactable, const FGameplayTag InteractionChannel)
{
	Super::ResetInteractionEntries(Interactable, InteractionChannel);
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

float UXInUInteractableComponent::GetDefaultInteractionDurationByTag(const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag)
{
	if (InteractableData)
	{
		FXInUInteractionData InteractionData;
		if (InteractableData->GetInteractionData(InteractionTag, InteractionData))
		{
			return InteractionData.InteractionTime;
		}
	}
	return -1.f;
}

float UXInUInteractableComponent::GetInteractionDurationByTag(const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag)
{
	if (!InteractionTimer.InteractionTag.MatchesTagExact(InteractionTag)) return -1.f;

	return InteractionTimer.Duration;
}

float UXInUInteractableComponent::GetInteractionTimeElapsedByTag(const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag)
{
	if (!InteractionTimer.InteractionTag.MatchesTagExact(InteractionTag)) return -1.f;
	
	if (InteractionTimer.StartTime >= 0.f)
	{
		return GetWorld()->GetTimeSeconds() - InteractionTimer.StartTime;
	}
	return -1.f;
}

float UXInUInteractableComponent::GetInteractionTimeLeftByTag(const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag)
{
	if (!InteractionTimer.InteractionTag.MatchesTagExact(InteractionTag)) return -1.f;
	
	if (InteractionTimer.StartTime >= 0.f && InteractionTimer.Duration >= 0.f)
	{
		return FMath::Max((InteractionTimer.StartTime + InteractionTimer.Duration) - GetWorld()->GetTimeSeconds(), 0.f);
	}
	return -1.f;
}

/*--------------------------------------------------------------------------------------------------------------------*/

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
/* Interaction Timer */

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

