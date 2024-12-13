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
	
	if (GetOwner<APawn>()->IsLocallyControlled())
	{
		UpdateSelectedInteractable_Local();
		
		if (bDebugArrayCount)
		{
			ENetRole LocalRole = GetOwner<APawn>()->GetLocalRole();
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
	if (!SelectedInteractable.Contains(InteractionChannel))
	{
		SelectedInteractable.Emplace(InteractionChannel);
	}
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

int32 UXInUInteractComponent::AddInteractableToInRangeMap(const FGameplayTag InteractionChannel, AActor* Interactable)
{
	if (!InteractablesInRange.Contains(InteractionChannel))
	{
		InteractablesInRange.Emplace(InteractionChannel, FXInUInteractablesContainer());
	}
	return InteractablesInRange[InteractionChannel].Interactables.AddUnique(Interactable);
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
	IXInUInteractInterface* InteractInterface = GetInteractInterface();
	if (!InteractInterface) return;
	
	FVector ViewLocation;
	FVector ViewDirection;
	if (!InteractInterface->Execute_GetLocalPlayerView(GetOwner(), ViewLocation, ViewDirection)) return;

	// for each InteractionChannel find the interactable in range that is closer to the center of the screen
	TArray<FGameplayTag> InteractionChannels;
	InteractablesInRange.GetKeys(InteractionChannels);
	for (FGameplayTag InteractionChannel : InteractionChannels)
	{
		float MaxCrossProduct = 0;
		AActor* NewSelectedActor = nullptr;
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

		// if we found a new closer interactable, update the status of both old and new
		if (NewSelectedActor != GetSelectedInteractable(InteractionChannel))
		{
			UpdateInteractableStatus_Local(GetSelectedInteractable(InteractionChannel), false);
			SetSelectedInteractable(InteractionChannel, NewSelectedActor);
			UpdateInteractableStatus_Local(NewSelectedActor, true);
		}
	}
}

void UXInUInteractComponent::UpdateInteractableStatus_Local(AActor* Interactable, const bool bSelected)
{
	if (!Interactable) return;

	IXInUInteractableInterface* InteractableInterface = Cast<IXInUInteractableInterface>(Interactable);
	if (!InteractableInterface) return;
	
	UXInUInteractableComponent* InteractableComponent = InteractableInterface->Execute_GetInteractableComponent(Interactable);
	if (!InteractableComponent) return;

	const FGameplayTag InteractionChannel = InteractableComponent->GetInteractionChannelTag();
	if (!InteractionChannel.IsValid()) return;
	
	// reset
	InteractableComponent->ResetInteractionEntries(InteractionChannel);
	ResetInteractionEntries(InteractionChannel);

	// not in range or has no interactable data
	const UXInUInteractableData* InteractableData = InteractableComponent->GetInteractableData();
	if (!Interactable || !IsInteractableInRange(InteractionChannel, Interactable))
	{
		InteractableComponent->ShowInteractionWidget(false);
		return;
	}
	
	if (!bSelected && InteractableComponent->GetUnselectedBehaviour() == EXInUInteractableUnselectedBehaviour::XInUIUB_ShowNone)
	{
		// if not selected and show none
		InteractableComponent->ShowInteractionWidget(false);
	}
	else // selected or unselected and (show default, or show interactions)
	{
		FXInUInteractionInfo InteractionInfo;
		InteractionInfo.Interactable = Interactable;
		InteractionInfo.InteractionChannel = InteractionChannel;

		if (bSelected || InteractableComponent->GetUnselectedBehaviour() == EXInUInteractableUnselectedBehaviour::XInUIUB_ShowInteractions)
		{
			// if selected, or unselected behaviour is to show interaction data
			// Get all interaction tags from actor's InteractableData, and generate the status for each to update interaction widget
			const FGameplayTagContainer InteractionTags = InteractableComponent->GetInteractableData()->GetInteractionTags();
			for (const FGameplayTag InteractionTag : InteractionTags)
			{
				FGameplayTag InteractionStatus;
				GetInteractInterface()->Execute_CanInteract(GetOwner(), Interactable, InteractionTag, InteractionStatus);
				if (InteractionStatus.IsValid())
				{
					FXInUInteractionType Interaction;
					Interaction.InteractionStatus = InteractionStatus;
					Interaction.InteractionTag = InteractionTag;
					InteractionInfo.Interactions.Add(Interaction);
				}
			}
		}
		// else we show default, which means that we do not pass any interactions in InteractionInfo

		InteractableComponent->AddInteractionEntry(InteractionInfo);
		InteractableComponent->ShowInteractionWidget(true);
		AddInteractionEntry(InteractionInfo);
	}
}

void UXInUInteractComponent::OnInteractableAvailabilityChanged(AActor* Interactable, const bool bAvailable)
{
	if (!Interactable) return;

	IXInUInteractableInterface* InteractableInterface = Cast<IXInUInteractableInterface>(Interactable);
	if (!InteractableInterface) return;
	
	const UXInUInteractableComponent* InteractableComponent = InteractableInterface->Execute_GetInteractableComponent(Interactable);
	if (!InteractableComponent) return;

	const FGameplayTag InteractionChannel = InteractableComponent->GetInteractionChannelTag();
	if (!InteractionChannel.IsValid()) return;
	
	// if set to available remove from disabled array and add to in range array
	if (bAvailable)
	{
		if (DisabledInteractablesInRange.Remove(Interactable) > 0)
		{
			AddInteractableToInRangeMap(InteractionChannel, Interactable);
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
	if (GetOwner<APawn>() && GetOwner<APawn>()->IsLocallyControlled())
	{
		UpdateInteractableStatus_Local(Interactable, Interactable == GetSelectedInteractable(InteractionChannel));
	}
}

void UXInUInteractComponent::AddInteractableInRange(AActor* NewInteractable)
{
	if (!NewInteractable) return;

	IXInUInteractableInterface* InteractableInterface = Cast<IXInUInteractableInterface>(NewInteractable);
	if (!InteractableInterface) return;
	
	UXInUInteractableComponent* InteractableComponent = InteractableInterface->Execute_GetInteractableComponent(NewInteractable);
	if (!InteractableComponent) return;

	const FGameplayTag InteractionChannel = InteractableComponent->GetInteractionChannelTag();
	if (!InteractionChannel.IsValid()) return;

	InteractableComponent->AvailableForInteractionDelegate.AddUniqueDynamic(this, &ThisClass::OnInteractableAvailabilityChanged);
	
	// if available for interaction, add to in range array and update local status for this actor
	if (InteractableComponent->GetAvailableForInteraction())
	{
		AddInteractableToInRangeMap(InteractionChannel, NewInteractable);

		// update status for this interactable
		if (GetOwner<APawn>() && GetOwner<APawn>()->IsLocallyControlled())
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
	
	UXInUInteractableComponent* InteractableComponent = InteractableInterface->Execute_GetInteractableComponent(NewInteractable);
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
		if (GetOwner<APawn>() && GetOwner<APawn>()->IsLocallyControlled())
		{
			UpdateInteractableStatus_Local(NewInteractable, false);
		}
	}
}

void UXInUInteractComponent::InputInteract(const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag)
{
	Interact(GetSelectedInteractable(InteractionChannel), InteractionChannel, InteractionChannel);
	
	if (GetOwner() && !GetOwner()->HasAuthority())
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
	
	if (IXInUInteractInterface* InteractInterface = GetInteractInterface())
	{
		FGameplayTag InteractionStatus;
		if (InteractInterface->Execute_CanInteract(GetOwner(), Interactable, InteractionTag, InteractionStatus))
		{
			InteractInterface->Execute_TryInteract(GetOwner(), Interactable, InteractionTag);
		}
	}
}

