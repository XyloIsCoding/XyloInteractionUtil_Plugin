// Copyright XyloIsCoding 2024


#include "Interaction/Interactable/XInUInteractableComponent.h"

#include "Blueprint/UserWidget.h"
#include "Interaction/Interactable/Data/XInUInteractableData.h"
#include "Interaction/Interact/XInUInteractComponent.h"
#include "Interaction/Interact/XInUInteractInterface.h"
#include "Net/UnrealNetwork.h"

UXInUInteractableComponent::UXInUInteractableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	bAvailableForInteraction = true;
	UnselectedBehaviour = EXInUInteractableUnselectedBehaviour::XInUIUB_ShowDefault;
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
