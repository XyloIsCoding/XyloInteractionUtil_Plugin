// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/XInUInteractableComponent.h"

#include "Blueprint/UserWidget.h"
#include "Interaction/XInUInteractableData.h"
#include "Interaction/XInUInteractComponent.h"
#include "Interaction/XInUInteractInterface.h"
#include "Net/UnrealNetwork.h"

UXInUInteractableComponent::UXInUInteractableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	bAvailableForInteraction = true;
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
	AvailableForInteractionDelegate.Broadcast(GetOwner(), bAvailableForInteraction);
}

void UXInUInteractableComponent::OnRep_AvailableForInteraction()
{
	AvailableForInteractionDelegate.Broadcast(GetOwner(), bAvailableForInteraction);
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

void UXInUInteractableComponent::ShowInteractionWidget(bool bShow)
{
	ShowInteractionWidgetDelegate.Broadcast(bShow);
}

void UXInUInteractableComponent::ResetInteractionWidget()
{
	ResetInteractionWidgetDelegate.Broadcast();
}

void UXInUInteractableComponent::AddDefaultEntryToInteractionWidget()
{
	if (InteractableData)
	{
		if (const TSubclassOf<UUserWidget> EntryWidgetClass = InteractableData->DefaultInteractionWidgetClass)
		{
			AddEntryToInteractionWidgetDelegate.Broadcast(EntryWidgetClass);
		}
	}
}

void UXInUInteractableComponent::AddEntryToInteractionWidget(FGameplayTag InteractionTag, FGameplayTag StatusTag)
{
	if (InteractableData)
	{
		if (const TSubclassOf<UUserWidget> EntryWidgetClass = InteractableData->GetInteractionWidgetClass(InteractionTag, StatusTag))
		{
			AddEntryToInteractionWidgetDelegate.Broadcast(EntryWidgetClass);
		}
	}
}
