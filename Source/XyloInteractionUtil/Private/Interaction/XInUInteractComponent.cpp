// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/XInUInteractComponent.h"

#include "Interaction/XInUInteractableComponent.h"
#include "Interaction/XInUInteractableData.h"
#include "Interaction/XInUInteractableInterface.h"
#include "Interaction/XInUInteractInterface.h"

UXInUInteractComponent::UXInUInteractComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * UActorComponent Interface
 */

void UXInUInteractComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UXInUInteractComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GetPawn<APawn>()->IsLocallyControlled())
	{
		UpdateSelectedInteractable_Client();
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Interact
 */

IXInUInteractInterface* UXInUInteractComponent::GetInteractInterface()
{
	if (!InteractInterface)
	{
		InteractInterface = GetOwner<IXInUInteractInterface>();
	}
	return InteractInterface;
}

void UXInUInteractComponent::UpdateSelectedInteractable_Client()
{
	if (GetInteractInterface())
	{
		FVector ViewLocation;
		FVector ViewDirection;
		if (GetInteractInterface()->GetLocalPlayerView(ViewLocation, ViewDirection))
		{
			float MaxCrossProduct = 0;
			AActor* NewSelectedActor = nullptr;
			for (AActor* Interactable : InteractablesInRange)
			{
				if (Interactable)
				{
					const float NewCrossProduct = (Interactable->GetActorLocation() - ViewLocation).GetSafeNormal() | ViewDirection;
					if (NewCrossProduct > MaxCrossProduct)
					{
						MaxCrossProduct = NewCrossProduct;
						NewSelectedActor = Interactable;
					}
				}
			}

			if (NewSelectedActor != SelectedInteractable)
			{
				UpdateInteractableStatus_Client(SelectedInteractable);
				UpdateInteractableStatus_Client(NewSelectedActor);
				SelectedInteractable = NewSelectedActor;
			}
		}
	}
}

void UXInUInteractComponent::UpdateInteractableStatus_Client(AActor* Interactable)
{
	if (IXInUInteractableInterface* InteractableInterface = Cast<IXInUInteractableInterface>(Interactable))
	{
		if (UXInUInteractableComponent* InteractableComponent = InteractableInterface->GetInteractableComponent())
		{
			// reset
			InteractableComponent->ResetInteractionWidget();
			
			// Get all interaction tags from actor's InteractableData, and generate the status for each to update interaction widget
			if (InteractablesInRange.Contains(Interactable) && InteractableComponent->InteractableData)
			{
				const FGameplayTagContainer InteractionTags = InteractableComponent->InteractableData->GetInteractionTags();
				for (const FGameplayTag InteractionTag : InteractionTags)
				{
					FGameplayTag InteractionStatus;
					if (Interactable && GetInteractInterface()->CanInteract(Interactable, InteractionTag, InteractionStatus))
					{
						InteractableComponent->AddEntryToInteractionWidget(InteractionTag, InteractionStatus);
					}
				}
			}
		}
	}
}

void UXInUInteractComponent::AddInteractableInRange(AActor* NewInteractable)
{
	if (Cast<IXInUInteractableInterface>(NewInteractable))
	{
		const TArray<AActor*> OldInteractablesInRange = InteractablesInRange;
		InteractablesInRange.Add(NewInteractable);

		if (GetPawn<APawn>()->IsLocallyControlled())
		{
			UpdateInteractableStatus_Client(NewInteractable);
		}
	}
}

void UXInUInteractComponent::RemoveInteractableInRange(AActor* NewInteractable)
{
	if (GetPawn<APawn>()->IsLocallyControlled())
	{
		UpdateInteractableStatus_Client(NewInteractable);
	}
	
	InteractablesInRange.Remove(NewInteractable);
}

void UXInUInteractComponent::Interact(FGameplayTag InteractionTag)
{
	if (GetInteractInterface())
	{
		FGameplayTag InteractionStatus;
		const bool bCanInteract = SelectedInteractable && GetInteractInterface()->CanInteract(SelectedInteractable, InteractionTag, InteractionStatus);
		
		// TODO: interaction message based on InteractionStatus

		if (bCanInteract)
		{
			ServerInteract(SelectedInteractable, InteractionTag);	
		}
	}
}

void UXInUInteractComponent::ServerInteract_Implementation(AActor* Interactable, FGameplayTag InteractionTag)
{
	if (Interactable && InteractablesInRange.Contains(Interactable))
	{
		if (GetInteractInterface())
		{
			FGameplayTag InteractionStatus;
			GetInteractInterface()->TryInteract(Interactable, InteractionTag, InteractionStatus);
		}
	}
}
