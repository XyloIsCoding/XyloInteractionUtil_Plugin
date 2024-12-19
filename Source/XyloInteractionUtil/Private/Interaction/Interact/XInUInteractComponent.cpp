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
 * UXInUBaseInteractionComponent Interface
 */

/*--------------------------------------------------------------------------------------------------------------------*/
/* Interaction Timer */

float UXInUInteractComponent::GetDefaultInteractionDurationByTag(const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag)
{
	if (UXInUInteractableComponent* InteractableComponent = GetInteractableComponent(GetSelectedInteractable(InteractionChannel))) //TODO: we do not have selectedInteractable on server so this does not work
	{
		return InteractableComponent->GetDefaultInteractionDurationByTag(InteractionChannel, InteractionTag);
	}
	return -1.f;
}

float UXInUInteractComponent::GetDefaultInteractionDurationByTag(AActor* Interactable, const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag)
{
	if (UXInUInteractableComponent* InteractableComponent = GetInteractableComponent(Interactable))
	{
		return InteractableComponent->GetDefaultInteractionDurationByTag(InteractionChannel, InteractionTag);
	}
	return -1.f;
}

float UXInUInteractComponent::GetInteractionDurationByTag(const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag)
{
	FXInUInteractionTimerHandle* InteractionTimerHandlePtr = InteractionTimer.Find(InteractionChannel);
	if (!InteractionTimerHandlePtr || !InteractionTimerHandlePtr->InteractionTag.MatchesTagExact(InteractionTag)) return -1.f;
	
	return GetWorld()->GetTimerManager().GetTimerRate(InteractionTimerHandlePtr->TimerHandle);
}

float UXInUInteractComponent::GetInteractionTimeElapsedByTag(const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag)
{
	FXInUInteractionTimerHandle* InteractionTimerHandlePtr = InteractionTimer.Find(InteractionChannel);
	if (!InteractionTimerHandlePtr || !InteractionTimerHandlePtr->InteractionTag.MatchesTagExact(InteractionTag)) return -1.f;
	
	return GetWorld()->GetTimerManager().GetTimerElapsed(InteractionTimerHandlePtr->TimerHandle);
}

float UXInUInteractComponent::GetInteractionTimeLeftByTag(const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag)
{
	FXInUInteractionTimerHandle* InteractionTimerHandlePtr = InteractionTimer.Find(InteractionChannel);
	if (!InteractionTimerHandlePtr || !InteractionTimerHandlePtr->InteractionTag.MatchesTagExact(InteractionTag)) return -1.f;
	
	return GetWorld()->GetTimerManager().GetTimerRemaining(InteractionTimerHandlePtr->TimerHandle);
}

/*--------------------------------------------------------------------------------------------------------------------*/


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Interact
 */

IXInUInteractInterface* UXInUInteractComponent::GetInteractInterface()
{
	return GetOwner<IXInUInteractInterface>();
}

UXInUInteractableComponent* UXInUInteractComponent::GetInteractableComponent(AActor* Interactable)
{
	if (!Interactable) return nullptr;
	
	IXInUInteractableInterface* InteractableInterface = Cast<IXInUInteractableInterface>(Interactable);
	if (!InteractableInterface) return nullptr;
	
	return InteractableInterface->Execute_GetInteractableComponent(Interactable);
}

UXInUInteractableComponent* UXInUInteractComponent::GetAvailableInteractableComponent(AActor* Interactable, const FGameplayTag InteractionChannel)
{
	UXInUInteractableComponent* InteractableComponent = GetInteractableComponent(Interactable);
	if (!InteractableComponent) return nullptr;
	
	if (!IsInteractableInRange(Interactable, InteractionChannel)) return nullptr;
	if (!InteractableComponent->GetAvailableForInteraction()) return nullptr;

	return InteractableComponent;
}

void UXInUInteractComponent::SetSelectedInteractable(AActor* NewInteractable, const FGameplayTag InteractionChannel)
{
	if (!SelectedInteractable.Contains(InteractionChannel))
	{
		SelectedInteractable.Emplace(InteractionChannel);
	}
	SelectedInteractable[InteractionChannel] = NewInteractable;
}

AActor* UXInUInteractComponent::GetSelectedInteractable(const FGameplayTag InteractionChannel) const
{
	if (SelectedInteractable.Contains(InteractionChannel))
	{
		return SelectedInteractable[InteractionChannel];
	}
	return nullptr;
}

bool UXInUInteractComponent::IsInteractableInRange(const AActor* Interactable, const FGameplayTag InteractionChannel) const
{
	return InteractablesInRange.Contains(InteractionChannel) && InteractablesInRange[InteractionChannel].Interactables.Contains(Interactable);
}

int32 UXInUInteractComponent::RemoveInteractableFromInRangeMap(AActor* Interactable, const FGameplayTag InteractionChannel) 
{
	if (!InteractablesInRange.Contains(InteractionChannel)) return 0;
	return InteractablesInRange[InteractionChannel].Interactables.Remove(Interactable);
}

int32 UXInUInteractComponent::AddInteractableToInRangeMap(AActor* Interactable, const FGameplayTag InteractionChannel)
{
	if (!InteractablesInRange.Contains(InteractionChannel))
	{
		InteractablesInRange.Emplace(InteractionChannel, FXInUInteractablesContainer());
	}
	return InteractablesInRange[InteractionChannel].Interactables.AddUnique(Interactable);
}

bool UXInUInteractComponent::IsInteractableSelected(const AActor* Interactable, const FGameplayTag InteractionChannel) const
{
	return Interactable && Interactable == GetSelectedInteractable(InteractionChannel);
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
			SetSelectedInteractable(NewSelectedActor, InteractionChannel);
			UpdateInteractableStatus_Local(NewSelectedActor, true);
		}
	}
}

void UXInUInteractComponent::UpdateInteractableStatus_Local(AActor* Interactable, const bool bSelected)
{
	UXInUInteractableComponent* InteractableComponent = GetInteractableComponent(Interactable);
	if (!InteractableComponent) return;

	const FGameplayTag InteractionChannel = InteractableComponent->GetInteractionChannelTag();
	if (!InteractionChannel.IsValid()) return;

	// stop interaction if we were interacting with this actor and now it is no longer selected
	if (!bSelected && HasActiveTimerForChannel(InteractionChannel))
	{
		InputStopInteraction(InteractionChannel);
	}
	
	// if has no interactable data or not in range, then reset and return
	const UXInUInteractableData* InteractableData = InteractableComponent->GetInteractableData();
	if (!InteractableData || !IsInteractableInRange(Interactable, InteractionChannel))
	{
		InteractableComponent->ResetInteractionEntries(Interactable, InteractionChannel);
		ResetInteractionEntries(Interactable, InteractionChannel);
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
	UXInUInteractableComponent* InteractableComponent = GetInteractableComponent(NewInteractable);
	if (!InteractableComponent) return;

	const FGameplayTag InteractionChannel = InteractableComponent->GetInteractionChannelTag();
	if (!InteractionChannel.IsValid()) return;

	InteractableComponent->AvailableForInteractionDelegate.AddUniqueDynamic(this, &ThisClass::OnInteractableAvailabilityChanged);
	
	// if available for interaction, add to in range array and update local status for this actor
	if (InteractableComponent->GetAvailableForInteraction())
	{
		AddInteractableToInRangeMap(NewInteractable, InteractionChannel);

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
	UXInUInteractableComponent* InteractableComponent = GetInteractableComponent(NewInteractable);
	if (!InteractableComponent) return;

	const FGameplayTag InteractionChannel = InteractableComponent->GetInteractionChannelTag();
	if (!InteractionChannel.IsValid()) return;
	
	const bool bWasAvailable = RemoveInteractableFromInRangeMap(NewInteractable, InteractionChannel) > 0;
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
	UXInUInteractableComponent* InteractableComponent = GetInteractableComponent(Interactable);
	if (!InteractableComponent) return;

	const FGameplayTag InteractionChannel = InteractableComponent->GetInteractionChannelTag();
	if (!InteractionChannel.IsValid()) return;
	
	// if set to available remove from disabled array and add to in range array
	if (bAvailable)
	{
		if (DisabledInteractablesInRange.Remove(Interactable) > 0)
		{
			AddInteractableToInRangeMap(Interactable, InteractionChannel);
		}
	}
	// if set in unavailable remove from in range array and add to disabled array
	else
	{
		if (RemoveInteractableFromInRangeMap(Interactable, InteractionChannel) > 0)
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

/*--------------------------------------------------------------------------------------------------------------------*/
/* Interaction */

void UXInUInteractComponent::InputStartInteraction(const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag)
{
	if (!StartInteraction(GetSelectedInteractable(InteractionChannel), InteractionChannel, InteractionTag)) return;

	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		// we return if it is a timed interaction with client only timer
		if (HasClientOnlyTimer(InteractionChannel)) return;
		
		ServerStartInteractionRPC(GetSelectedInteractable(InteractionChannel), InteractionChannel, InteractionTag);
	}
}

void UXInUInteractComponent::InputStopInteraction(const FGameplayTag InteractionChannel)
{
	if (!HasActiveTimerForChannel(InteractionChannel)) return;
	const bool bClientOnlyTimerActive = HasClientOnlyTimer(InteractionChannel);

	StopInteraction(GetSelectedInteractable(InteractionChannel), InteractionChannel);

	// we do not call rpc if timer was client only
	if (!bClientOnlyTimerActive && GetOwner() && !GetOwner()->HasAuthority())
	{
		ServerStopInteractionRPC(GetSelectedInteractable(InteractionChannel), InteractionChannel);
	}
}

void UXInUInteractComponent::ServerStartInteractionRPC_Implementation(AActor* Interactable, const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag)
{
	StartInteraction(Interactable, InteractionChannel, InteractionTag);
}

void UXInUInteractComponent::ServerStopInteractionRPC_Implementation(AActor* Interactable, const FGameplayTag InteractionChannel)
{
	StopInteraction(Interactable, InteractionChannel);
}

bool UXInUInteractComponent::StartInteraction(AActor* Interactable, const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag)
{
	float InteractionTime = GetDefaultInteractionDurationByTag(Interactable, InteractionChannel, InteractionTag);
	if (InteractionTime == -1.f) return false;
	
	if (InteractionTime == 0.f)
	{
		Interact(Interactable, InteractionChannel, InteractionTag);
		return true;
	}
	return StartInteractionWithDuration(Interactable, InteractionChannel, InteractionTag, InteractionTime);
}

bool UXInUInteractComponent::StartInteractionWithDuration(AActor* Interactable, const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag, const float InteractionTime)
{
	UXInUInteractableComponent* InteractableComponent = GetInteractableComponent(Interactable);
	if (!InteractableComponent || !InteractableComponent->GetInteractableData()) return false;

	FXInUInteractionData InteractionData;
	if (!InteractableComponent->GetInteractableData()->GetInteractionData(InteractionTag, InteractionData)) return false;
	
	StartInteractionTimer(Interactable, InteractionChannel, InteractionTag, InteractionTime, InteractionData.bClientOnlyInteractionTime);

	// Passing timer data to interactable
	if (GetOwner<APawn>() && GetOwner<APawn>()->IsLocallyControlled())
	{
		FXInUInteractionTimerData InteractionTimerData;
		InteractionTimerData.InteractionTag = InteractionTag;
		InteractionTimerData.StartTime = GetWorld()->GetTimeSeconds();
		InteractionTimerData.Duration = InteractionTime;
		InteractableComponent->UpdateInteractionTimerData(InteractionTimerData);
	}
	return true;
}

void UXInUInteractComponent::StopInteraction(AActor* Interactable, const FGameplayTag InteractionChannel)
{
	StopInteractionTimer(InteractionChannel);

	// Passing timer data to interactable
	if (GetOwner<APawn>() && GetOwner<APawn>()->IsLocallyControlled())
	{
		UXInUInteractableComponent* InteractableComponent = GetInteractableComponent(Interactable);
		if (!InteractableComponent) return;
		
		InteractableComponent->ResetInteractionTimerData();
	}
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

void UXInUInteractComponent::InteractFromTimer(AActor* Interactable, const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag)
{
	// since this function is delayed, we check that the interactable is still selected
	UXInUInteractableComponent* InteractableComponent = GetAvailableInteractableComponent(Interactable, InteractionChannel);
	if (!InteractableComponent) return;
	
	Interact(Interactable, InteractionChannel, InteractionTag);

	// Passing timer data to interactable
	if (GetOwner<APawn>() && GetOwner<APawn>()->IsLocallyControlled())
	{
		InteractableComponent->ResetInteractionTimerData();
	}
}

void UXInUInteractComponent::ServerInteractFromTimerRPC_Implementation(AActor* Interactable, const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag)
{
	InteractFromTimer(Interactable, InteractionChannel, InteractionTag);
}

/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Interaction Timer */

bool UXInUInteractComponent::HasActiveTimerForChannel(const FGameplayTag InteractionChannel)
{
	FXInUInteractionTimerHandle* InteractionTimerHandlePtr = InteractionTimer.Find(InteractionChannel);
	return InteractionTimerHandlePtr && GetWorld()->GetTimerManager().IsTimerActive(InteractionTimerHandlePtr->TimerHandle);
}

FXInUInteractionTimerHandle* UXInUInteractComponent::GetActiveTimerByChannel(const FGameplayTag InteractionChannel)
{
	FXInUInteractionTimerHandle* InteractionTimerHandlePtr = InteractionTimer.Find(InteractionChannel);
	if (!InteractionTimerHandlePtr) return nullptr;
	if (!GetWorld()->GetTimerManager().IsTimerActive(InteractionTimerHandlePtr->TimerHandle)) return nullptr;
	return InteractionTimerHandlePtr;
}

bool UXInUInteractComponent::HasClientOnlyTimer(const FGameplayTag InteractionChannel)
{
	FXInUInteractionTimerHandle* InteractionTimerHandlePtr = GetActiveTimerByChannel(InteractionChannel);
	return InteractionTimerHandlePtr && InteractionTimerHandlePtr->bClientOnly;
}

void UXInUInteractComponent::StartInteractionTimer(AActor* Interactable, const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag, const float Timer, const bool bClientOnly)
{
	if (!InteractionTimer.Contains(InteractionChannel))
	{
		InteractionTimer.Emplace(InteractionChannel);
	}
	InteractionTimer[InteractionChannel].InteractionTag = InteractionTag;
	InteractionTimer[InteractionChannel].bClientOnly = bClientOnly;
	FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &ThisClass::InteractionTimerEnded, Interactable, InteractionChannel, InteractionTag, bClientOnly);
	GetWorld()->GetTimerManager().SetTimer(InteractionTimer[InteractionChannel].TimerHandle, TimerDelegate, Timer, false);
}

void UXInUInteractComponent::StopInteractionTimer(const FGameplayTag InteractionChannel)
{
	if (FXInUInteractionTimerHandle* InteractionTimerHandlePtr = InteractionTimer.Find(InteractionChannel))
	{
		GetWorld()->GetTimerManager().ClearTimer(InteractionTimerHandlePtr->TimerHandle);
	}
}

void UXInUInteractComponent::InteractionTimerEnded(AActor* Interactable, const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag, const bool bClientOnly)
{
	InteractFromTimer(Interactable, InteractionChannel, InteractionTag);

	if (bClientOnly && GetOwner() && !GetOwner()->HasAuthority())
	{
		ServerInteractFromTimerRPC(Interactable, InteractionChannel, InteractionTag);
	}
}

/*--------------------------------------------------------------------------------------------------------------------*/


