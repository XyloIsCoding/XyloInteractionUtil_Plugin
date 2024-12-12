// Copyright XyloIsCoding 2024


#include "Interaction/Interact/XInUInteractComponent.h"

#include "Interaction/Interactable/XInUInteractableComponent.h"
#include "Interaction/Interactable/Data/XInUInteractableData.h"
#include "Interaction/Interactable/XInUInteractableInterface.h"
#include "Interaction/Interact/XInUInteractInterface.h"

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
	return GetOwner<IXInUInteractInterface>();
}

void UXInUInteractComponent::SetSelectedInteractable(const FGameplayTag InteractionChannel, AActor* NewInteractable)
{
	SelectedInteractable[InteractionChannel] = NewInteractable;
}

AActor* UXInUInteractComponent::GetSelectedInteractable(const FGameplayTag InteractionChannel)
{
	if (AActor** ResultPointer = SelectedInteractable.Find(InteractionChannel))
	{
		return *ResultPointer;
	}
	return nullptr;
}

bool UXInUInteractComponent::IsInteractableInRange(const FGameplayTag InteractionChannel, const AActor* Interactable) const
{
	return InteractablesInRange.Contains(InteractionChannel) && InteractablesInRange[InteractionChannel].Interactables.Contains(Interactable);
}

int32 UXInUInteractComponent::RemoveInteractableFromInRangeMap(const FGameplayTag InteractionChannel, AActor* Interactable) 
{
	if (!InteractablesInRange.Contains(InteractionChannel)) return 0;
	return InteractablesInRange[InteractionChannel].Interactables.Remove(Interactable);
}

void UXInUInteractComponent::RefreshAllInteractableStatus_Local()
{
	TArray<FGameplayTag> InteractionChannels;
	InteractablesInRange.GetKeys(InteractionChannels);
	for (FGameplayTag InteractionChannel : InteractionChannels)
	{
		for (AActor* Actor : InteractablesInRange[InteractionChannel].Interactables)
		{
			UpdateInteractableStatus_Local(Actor, Actor == GetSelectedInteractable(InteractionChannel));
		}
	}
}

void UXInUInteractComponent::UpdateSelectedInteractable_Local()
{
	if (GetInteractInterface())
	{
		FVector ViewLocation;
		FVector ViewDirection;
		if (GetInteractInterface()->Execute_GetLocalPlayerView(GetOwner(), ViewLocation, ViewDirection))
		{
			float MaxCrossProduct = 0;
			AActor* NewSelectedActor = nullptr;
			
			TArray<FGameplayTag> InteractionChannels;
			InteractablesInRange.GetKeys(InteractionChannels);
			for (FGameplayTag InteractionChannel : InteractionChannels)
			{
				for (AActor* Interactable : InteractablesInRange[InteractionChannel].Interactables)
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

				if (NewSelectedActor != GetSelectedInteractable(InteractionChannel))
				{
					UpdateInteractableStatus_Local(GetSelectedInteractable(InteractionChannel), false);
					SetSelectedInteractable(InteractionChannel, NewSelectedActor);
					UpdateInteractableStatus_Local(NewSelectedActor, true);
				}
			}
		}
	}
}

void UXInUInteractComponent::UpdateInteractableStatus_Local(AActor* Interactable, const bool bSelected)
{
	if (!Interactable) return;

	IXInUInteractableInterface* InteractableInterface = Cast<IXInUInteractableInterface>(Interactable);
	if (!InteractableInterface) return;
	
	UXInUInteractableComponent* InteractableComponent = InteractableInterface->GetInteractableComponent();
	if (!InteractableComponent) return;

	const FGameplayTag InteractionChannel = InteractableComponent->GetInteractionChannelTag();
	if (!InteractionChannel.IsValid()) return;
	
	// reset
	InteractableComponent->ResetInteractionWidget();
	
	// if is in range and has interactable data
	if (IsInteractableInRange(InteractionChannel, Interactable) && InteractableComponent->GetInteractableData())
	{
		// if selected, or unselected behaviour is to show interaction data
		if (bSelected || InteractableComponent->GetUnselectedBehaviour() == EXInUInteractableUnselectedBehaviour::XInUIUB_ShowInteractions)
		{
			// Get all interaction tags from actor's InteractableData, and generate the status for each to update interaction widget
			const FGameplayTagContainer InteractionTags = InteractableComponent->GetInteractableData()->GetInteractionTags();
			for (const FGameplayTag InteractionTag : InteractionTags)
			{
				FGameplayTag InteractionStatus;
				GetInteractInterface()->Execute_CanInteract(GetOwner(), Interactable, InteractionTag, InteractionStatus);
				if (InteractionStatus.IsValid())
				{
					InteractableComponent->AddEntryToInteractionWidget(InteractionTag, InteractionStatus);
				}
			}
			InteractableComponent->ShowInteractionWidget(true);
		}
		// if not selected and show default
		else if (InteractableComponent->GetUnselectedBehaviour() == EXInUInteractableUnselectedBehaviour::XInUIUB_ShowDefault)
		{
			InteractableComponent->AddDefaultEntryToInteractionWidget();
			InteractableComponent->ShowInteractionWidget(true);
		}
		// if not selected and show none
		else if (InteractableComponent->GetUnselectedBehaviour() == EXInUInteractableUnselectedBehaviour::XInUIUB_ShowNone)
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

void UXInUInteractComponent::OnInteractableAvailabilityChanged(AActor* Interactable, bool bAvailable)
{
	if (!Interactable) return;

	IXInUInteractableInterface* InteractableInterface = Cast<IXInUInteractableInterface>(Interactable);
	if (!InteractableInterface) return;
	
	const UXInUInteractableComponent* InteractableComponent = InteractableInterface->GetInteractableComponent();
	if (!InteractableComponent) return;

	const FGameplayTag InteractionChannel = InteractableComponent->GetInteractionChannelTag();
	if (!InteractionChannel.IsValid()) return;
	
	// if set to available remove from disabled array and add to in range array
	if (bAvailable)
	{
		if (DisabledInteractablesInRange.Remove(Interactable) > 0)
		{
			InteractablesInRange[InteractionChannel].Interactables.AddUnique(Interactable);
		}
	}
	// if set in unavailable remove from in range array and add to disabled array
	else
	{
		if (RemoveInteractableFromInRangeMap(InteractionChannel, Interactable) > 0)
		{
			DisabledInteractablesInRange.AddUnique(Interactable);
		}
	}

	// update status for this interactable
	if (GetPawn<APawn>() && GetPawn<APawn>()->IsLocallyControlled())
	{
		UpdateInteractableStatus_Local(Interactable, Interactable == GetSelectedInteractable(InteractionChannel));
	}
}

void UXInUInteractComponent::AddInteractableInRange(AActor* NewInteractable)
{
	if (!NewInteractable) return;

	IXInUInteractableInterface* InteractableInterface = Cast<IXInUInteractableInterface>(NewInteractable);
	if (!InteractableInterface) return;
	
	const UXInUInteractableComponent* InteractableComponent = InteractableInterface->GetInteractableComponent();
	if (!InteractableComponent) return;

	const FGameplayTag InteractionChannel = InteractableComponent->GetInteractionChannelTag();
	if (!InteractionChannel.IsValid()) return;

	InteractableComponent->AvailableForInteractionDelegate.AddUniqueDynamic(this, &ThisClass::OnInteractableAvailabilityChanged);
	
	// if available for interaction, add to in range array and update local status for this actor
	if (InteractableComponent->GetAvailableForInteraction())
	{
		InteractablesInRange[InteractionChannel].Interactables.AddUnique(NewInteractable);

		// update status for this interactable
		if (GetPawn<APawn>() && GetPawn<APawn>()->IsLocallyControlled())
		{
			UpdateInteractableStatus_Local(NewInteractable, false);
		}
	}
	// if not available for interaction, add to disabled array
	else
	{
		DisabledInteractablesInRange.AddUnique(NewInteractable);
	}
}

void UXInUInteractComponent::RemoveInteractableInRange(AActor* NewInteractable)
{
	if (!NewInteractable) return;

	IXInUInteractableInterface* InteractableInterface = Cast<IXInUInteractableInterface>(NewInteractable);
	if (!InteractableInterface) return;
	
	UXInUInteractableComponent* InteractableComponent = InteractableInterface->GetInteractableComponent();
	if (!InteractableComponent) return;

	const FGameplayTag InteractionChannel = InteractableComponent->GetInteractionChannelTag();
	if (!InteractionChannel.IsValid()) return;
	
	const bool bWasAvailable = RemoveInteractableFromInRangeMap(InteractionChannel, NewInteractable) > 0;
	if (bWasAvailable || DisabledInteractablesInRange.Remove(NewInteractable) > 0)
	{
		// remove binding of delegate for Available state
		InteractableComponent->AvailableForInteractionDelegate.RemoveAll(this);
	}
	
	if (bWasAvailable)
	{
		// update status for this interactable
		if (GetPawn<APawn>() && GetPawn<APawn>()->IsLocallyControlled())
		{
			UpdateInteractableStatus_Local(NewInteractable, false);
		}
	}
}

void UXInUInteractComponent::InputInteract(const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag)
{
	Interact(GetSelectedInteractable(InteractionChannel), InteractionChannel, InteractionChannel);
	
	if (!HasAuthority())
	{
		ServerInteractRPC(GetSelectedInteractable(InteractionChannel), InteractionChannel, InteractionChannel);
	}
}

void UXInUInteractComponent::ServerInteractRPC_Implementation(AActor* Interactable, const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag)
{
	Interact(Interactable, InteractionChannel, InteractionTag);
}

void UXInUInteractComponent::Interact(AActor* Interactable, const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag)
{
	// if no longer in range, set to null
	if (!IsInteractableInRange(InteractionChannel, Interactable)) Interactable = nullptr;
	// if it cannot be interacted with, set to null
	if (IXInUInteractableInterface* InteractableInterface = Cast<IXInUInteractableInterface>(Interactable))
	{
		if (UXInUInteractableComponent* InteractableComponent = InteractableInterface->Execute_GetInteractableComponent(Interactable))
		{
			if (!InteractableComponent->GetAvailableForInteraction()) Interactable = nullptr;
		}
	}
	// NOTE: im setting to null instead of just returning, cause the result might differ from client to server.
	// this way, for example, if client predicts an equip weapon function, the server can override equipping nullptr.
	
	if (GetInteractInterface())
	{
		FGameplayTag InteractionStatus;
		if (GetInteractInterface()->Execute_CanInteract(GetOwner(), Interactable, InteractionTag, InteractionStatus))
		{
			GetInteractInterface()->Execute_TryInteract(GetOwner(), Interactable, InteractionTag);
		}
	}
}

