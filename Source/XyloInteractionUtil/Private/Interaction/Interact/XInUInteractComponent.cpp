// Copyright XyloIsCoding 2024


#include "Interaction/Interact/XInUInteractComponent.h"

#include "Interaction/XInUInteractionInfo.h"
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
	
	// if has no interactable data or not in range, then reset and return
	const UXInUInteractableData* InteractableData = InteractableComponent->GetInteractableData();
	if (!InteractableData || !IsInteractableInRange(InteractionChannel, Interactable))
	{
		InteractableComponent->ResetInteractionEntries(InteractionChannel, Interactable);
		ResetInteractionEntries(InteractionChannel, Interactable);
		return;
	}

	FXInUInteractionInfo InteractionInfo;
	InteractionInfo.Interactable = Interactable;
	InteractionInfo.InteractionChannel = InteractionChannel;
	InteractionInfo.bSelected = bSelected;

	// if selected, or unselected behaviour is to compute interactions
	// Get all interaction tags from actor's InteractableData, and generate the status for each
	if (bSelected || InteractableComponent->GetUnselectedBehaviour() == EXInUInteractableUnselectedBehaviour::EUB_ComputeInteractions)
	{
		const TArray<FXInUInteractionData>& InteractionTags = InteractableComponent->GetInteractableData()->GetPossibleInteractions();
		for (const FXInUInteractionData InteractionData : InteractionTags)
		{
			FGameplayTag InteractionStatus;
			GetInteractInterface()->Execute_CanInteract(GetOwner(), Interactable, InteractionData.InteractionTag, InteractionStatus);
			if (InteractionStatus.IsValid())
			{
				FXInUInteractionType Interaction;
				Interaction.InteractionStatus = InteractionStatus;
				Interaction.InteractionTag = InteractionData.InteractionTag;
				InteractionInfo.Interactions.Add(Interaction);
			}
		}
	}

	InteractableComponent->UpdateInteractionEntries(InteractionInfo);
	UpdateInteractionEntries(InteractionInfo);
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

void UXInUInteractComponent::InputStartInteraction(const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag)
{
	StartInteraction(GetSelectedInteractable(InteractionChannel), InteractionChannel, InteractionTag);
	
	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		ServerStartInteractionRPC(GetSelectedInteractable(InteractionChannel), InteractionChannel, InteractionTag);
	}
}

void UXInUInteractComponent::InputStopInteraction(const FGameplayTag InteractionChannel)
{
	if (GetInteractionTimeLeft(InteractionChannel) == -1.f) return;
		
	StopInteraction(InteractionChannel);
	
	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		ServerStopInteractionRPC(InteractionChannel);
	}
}

void UXInUInteractComponent::ServerStartInteractionRPC_Implementation(AActor* Interactable, const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag)
{
	StartInteraction(Interactable, InteractionChannel, InteractionTag);
}

void UXInUInteractComponent::ServerStopInteractionRPC_Implementation(const FGameplayTag InteractionChannel)
{
	StopInteraction(InteractionChannel);
}

void UXInUInteractComponent::StartInteraction(AActor* Interactable, const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag)
{
	float InteractionTime = GetInteractionTime(Interactable, InteractionChannel, InteractionTag);
	if (InteractionTime == 0.f)
	{
		Interact(Interactable, InteractionChannel, InteractionTag);
	}
	else if (InteractionTime > 0.f)
	{
		StartInteractionTimer(Interactable, InteractionChannel, InteractionTag, InteractionTime);
	}
}

void UXInUInteractComponent::StopInteraction(const FGameplayTag InteractionChannel)
{
	StopInteractionTimer(InteractionChannel);
}

float UXInUInteractComponent::GetInteractionTime(AActor* Interactable, const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag)
{
	float InteractionTime = -1.f;
	
	if (!IsInteractableInRange(InteractionChannel, Interactable)) return InteractionTime;

	IXInUInteractableInterface* InteractableInterface = Cast<IXInUInteractableInterface>(Interactable);
	if (!InteractableInterface) return InteractionTime;

	UXInUInteractableComponent* InteractableComponent = InteractableInterface->Execute_GetInteractableComponent(Interactable);
	if (!InteractableComponent) return InteractionTime;
	
	if (!InteractableComponent->GetAvailableForInteraction()) return InteractionTime;

	FXInUInteractionData InteractionData;
	if (InteractableComponent->GetInteractableData() && InteractableComponent->GetInteractableData()->GetInteractionData(InteractionTag, InteractionData))
	{
		InteractionTime = InteractionData.InteractionTime;
	}
	return InteractionTime;
}

void UXInUInteractComponent::Interact(AActor* Interactable, const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag)
{
	if (IXInUInteractInterface* InteractInterface = GetInteractInterface())
	{
		FGameplayTag InteractionStatus;
		if (InteractInterface->Execute_CanInteract(GetOwner(), Interactable, InteractionTag, InteractionStatus))
		{
			InteractInterface->Execute_TryInteract(GetOwner(), Interactable, InteractionTag);
		}
	}
}

void UXInUInteractComponent::StartInteractionTimer(AActor* Interactable, const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag, const float Timer)
{
	if (!InteractionTimer.Contains(InteractionChannel))
	{
		InteractionTimer.Emplace(InteractionChannel);
	}
	FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &ThisClass::InteractionTimerEnded, Interactable, InteractionChannel, InteractionTag);
	GetWorld()->GetTimerManager().SetTimer(InteractionTimer[InteractionChannel], TimerDelegate, Timer, false);
}

void UXInUInteractComponent::StopInteractionTimer(const FGameplayTag InteractionChannel)
{
	if (FTimerHandle* TimerHandlePtr = InteractionTimer.Find(InteractionChannel))
	{
		GetWorld()->GetTimerManager().ClearTimer(*TimerHandlePtr);
	}
}

void UXInUInteractComponent::InteractionTimerEnded(AActor* Interactable, const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag)
{
	if (GetInteractionTime(Interactable, InteractionChannel, InteractionTag) >= 0.f)
	{
		Interact(Interactable, InteractionChannel, InteractionTag);
	}
}

float UXInUInteractComponent::GetInteractionMaxTime(const FGameplayTag InteractionChannel)
{
	if (FTimerHandle* TimerHandlePtr = InteractionTimer.Find(InteractionChannel))
	{
		return GetWorld()->GetTimerManager().GetTimerRate(*TimerHandlePtr);
	}
	return -1.f;
}

float UXInUInteractComponent::GetInteractionTimeElapsed(const FGameplayTag InteractionChannel)
{
	if (FTimerHandle* TimerHandlePtr = InteractionTimer.Find(InteractionChannel))
	{
		return GetWorld()->GetTimerManager().GetTimerElapsed(*TimerHandlePtr);
	}
	return -1.f;
}

float UXInUInteractComponent::GetInteractionTimeLeft(const FGameplayTag InteractionChannel)
{
	if (FTimerHandle* TimerHandlePtr = InteractionTimer.Find(InteractionChannel))
	{
		return GetWorld()->GetTimerManager().GetTimerRemaining(*TimerHandlePtr);
	}
	return -1.f;
}

