// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/XInUInteractComponent.h"

#include <string>

#include "GameFramework/HUD.h"
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
		UpdateSelectedInteractable_Local();
		
		if (bDebugArrayCount)
		{
			ENetRole LocalRole = GetPawn<APawn>()->GetLocalRole();
			FString Role;
			switch (LocalRole)
			{
			case ENetRole::ROLE_Authority:
				Role = FString("Authority");
				break;
			case ENetRole::ROLE_AutonomousProxy:
				Role = FString("Autonomous Proxy");
				break;
			case ENetRole::ROLE_SimulatedProxy:
				Role = FString("Simulated Proxy");
				break;
			case ENetRole::ROLE_None:
				Role = FString("None");
				break;
			}
			FString LocalRoleString = FString::Printf(TEXT("Local Role: %s"), *Role) + "> " + FString::Printf(TEXT("InRange: %d"), InteractablesInRange.Num()) + " | "  + FString::Printf(TEXT("Disabled: %d"), DisabledInteractablesInRange.Num());
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 0.f, LocalRole == ROLE_Authority ? FColor::Green : FColor::Blue, LocalRoleString);
		}
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

void UXInUInteractComponent::UpdateSelectedInteractable_Local()
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
				UpdateInteractableStatus_Local(SelectedInteractable);
				SelectedInteractable = NewSelectedActor;
				UpdateInteractableStatus_Local(NewSelectedActor, true);
			}
		}
	}
}

void UXInUInteractComponent::UpdateInteractableStatus_Local(AActor* Interactable, bool bSelected)
{
	if (IXInUInteractableInterface* InteractableInterface = Cast<IXInUInteractableInterface>(Interactable))
	{
		if (UXInUInteractableComponent* InteractableComponent = InteractableInterface->GetInteractableComponent())
		{
			// reset
			InteractableComponent->ResetInteractionWidget();
			
			// if is in range and has interactable data
			if (InteractablesInRange.Contains(Interactable) && InteractableComponent->InteractableData)
			{
				// if selected, or unselected behaviour is to show interaction data
				if (bSelected || InteractableComponent->UnselectedBehaviour == XInUIUB_ShowInteractions)
				{
					// Get all interaction tags from actor's InteractableData, and generate the status for each to update interaction widget
					const FGameplayTagContainer InteractionTags = InteractableComponent->InteractableData->GetInteractionTags();
					for (const FGameplayTag InteractionTag : InteractionTags)
					{
						FGameplayTag InteractionStatus;
						if (Interactable && GetInteractInterface()->CanInteract(Interactable, InteractionTag, InteractionStatus))
						{
							InteractableComponent->AddEntryToInteractionWidget(InteractionTag, InteractionStatus);
						}
					}
					InteractableComponent->ShowInteractionWidget(true);
				}
				// if not selected and show default
				else if (InteractableComponent->UnselectedBehaviour == XInUIUB_ShowDefault)
				{
					InteractableComponent->AddDefaultEntryToInteractionWidget();
					InteractableComponent->ShowInteractionWidget(true);
				}
				// if not selected and show none
				else if (InteractableComponent->UnselectedBehaviour == XInUIUB_ShowNone)
				{
					InteractableComponent->ShowInteractionWidget(false);
				}
			}
			// not in range or has no interactable data
			else
			{
				InteractableComponent->ShowInteractionWidget(false);
			}
		}
	}
}

void UXInUInteractComponent::OnInteractableAvailabilityChanged(AActor* Interactable, bool bAvailable)
{
	// if set to available remove from disabled array and add to in range array
	if (bAvailable)
	{
		if (DisabledInteractablesInRange.Remove(Interactable) > 0)
		{
			InteractablesInRange.Add(Interactable);
		}
	}
	// if set in unavailable remove from in range array and add to disabled array
	else
	{
		if (InteractablesInRange.Remove(Interactable) > 0)
		{
			DisabledInteractablesInRange.Add(Interactable);
		}
	}
}

void UXInUInteractComponent::AddInteractableInRange(AActor* NewInteractable)
{
	InteractablesInRange.Remove(nullptr);
	
	if (IXInUInteractableInterface* InteractableInterface = Cast<IXInUInteractableInterface>(NewInteractable))
	{
		// Bind delegate for Available state
		if (UXInUInteractableComponent* InteractableComponent = InteractableInterface->GetInteractableComponent())
		{
			InteractableComponent->AvailableForInteractionDelegate.AddDynamic(this, &ThisClass::OnInteractableAvailabilityChanged);

			// if available for interaction, add to in range array and update local status for this actor
			if (InteractableComponent->GetAvailableForInteraction())
			{
				InteractablesInRange.Add(NewInteractable);

				// update status for this interactable
				if (GetPawn<APawn>()->IsLocallyControlled())
				{
					UpdateInteractableStatus_Local(NewInteractable);
				}
			}
			// if not available for interaction, add to disabled array
			else
			{
				DisabledInteractablesInRange.Add(NewInteractable);
			}
		}
	}
}

void UXInUInteractComponent::RemoveInteractableInRange(AActor* NewInteractable)
{
	const bool bWasAvailable = InteractablesInRange.Remove(NewInteractable) > 0;
	if (bWasAvailable || DisabledInteractablesInRange.Remove(NewInteractable) > 0)
	{
		// remove binding of delegate for Available state
		if (IXInUInteractableInterface* InteractableInterface = Cast<IXInUInteractableInterface>(NewInteractable))
		{
			if (UXInUInteractableComponent* InteractableComponent = InteractableInterface->GetInteractableComponent())
			{
				InteractableComponent->AvailableForInteractionDelegate.RemoveAll(this);
			}
		}
	}

	// update status for this interactable
	if (bWasAvailable && GetPawn<APawn>()->IsLocallyControlled())
	{
		UpdateInteractableStatus_Local(NewInteractable);
	}
}

void UXInUInteractComponent::Interact(FGameplayTag InteractionTag)
{
	ExecuteInteraction(SelectedInteractable, InteractionTag);
	
	if (!HasAuthority())
	{
		ServerInteract(SelectedInteractable, InteractionTag);
	}
}

void UXInUInteractComponent::ServerInteract_Implementation(AActor* Interactable, FGameplayTag InteractionTag)
{
	ExecuteInteraction(Interactable, InteractionTag);
}

void UXInUInteractComponent::ExecuteInteraction(AActor* Interactable, FGameplayTag InteractionTag)
{
	// if no longer in range, set to null
	if (!InteractablesInRange.Contains(Interactable)) Interactable = nullptr;
	// if it cannot be interacted with, set to null
	if (IXInUInteractableInterface* InteractableInterface = Cast<IXInUInteractableInterface>(Interactable))
	{
		if (UXInUInteractableComponent* InteractableComponent = InteractableInterface->GetInteractableComponent())
		{
			if (!InteractableComponent->GetAvailableForInteraction()) Interactable = nullptr;
		}
	}
	// NOTE: im setting to null instead of just returning, cause the result might differ from client to server.
	// this way, for example, if client predicts an equip weapon function, the server can override equipping nullptr.
	
	if (GetInteractInterface())
	{
		FGameplayTag InteractionStatus;
		if (GetInteractInterface()->CanInteract(Interactable, InteractionTag, InteractionStatus))
		{
			GetInteractInterface()->TryInteract(Interactable, InteractionTag);
		}
	}
}
