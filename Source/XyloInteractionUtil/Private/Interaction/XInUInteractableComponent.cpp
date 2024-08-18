// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/XInUInteractableComponent.h"

#include "Blueprint/UserWidget.h"
#include "Interaction/XInUInteractableData.h"
#include "Interaction/XInUInteractableInterface.h"
#include "Interaction/XInUInteractComponent.h"
#include "Interaction/XInUInteractInterface.h"

UXInUInteractableComponent::UXInUInteractableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * UActorComponent Interface
 */

void UXInUInteractableComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Interactable
 */

void UXInUInteractableComponent::OnEnterInteractRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult,
	bool bInstantInteraction, FGameplayTag InstantInteractionTag)
{
	if (IXInUInteractInterface* InteractInterface = Cast<IXInUInteractInterface>(OtherActor))
	{
		if (UXInUInteractComponent* InteractComponent = InteractInterface->GetInteractComponent())
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
		if (UXInUInteractComponent* InteractComponent = InteractInterface->GetInteractComponent())
		{
			InteractComponent->RemoveInteractableInRange(GetOwner());
		}
	}
}

void UXInUInteractableComponent::ResetInteractionWidget()
{
	ResetInteractionWidgetDelegate.Broadcast();
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
