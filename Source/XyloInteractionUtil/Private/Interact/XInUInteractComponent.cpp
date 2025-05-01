// Fill out your copyright notice in the Description page of Project Settings.


#include "Interact/XInUInteractComponent.h"

#include "XInUInteractionTypes.h"
#include "XInUInteractionUtilLibrary.h"
#include "Interactable/XInUInteractableComponent.h"

bool FXInUInteractableList::RegisterInteractable(AActor* Interactable)
{
	UXInUInteractableComponent* InteractableComponent = UXInUInteractionUtilLibrary::GetInteractableComponent(Interactable);
	if (!InteractableComponent) return false;

	FGameplayTag Channel;
	if (!InteractableComponent->GetInteractionChannel(Channel)) return false;

	if (InteractableComponent->IsAvailable())
	{
		UnAvailable.FindOrAdd(Channel).Interactables.Remove(Interactable);
		Available.FindOrAdd(Channel).Interactables.AddUnique(Interactable);
	}
	else
	{
		Available.FindOrAdd(Channel).Interactables.Remove(Interactable);
		UnAvailable.FindOrAdd(Channel).Interactables.AddUnique(Interactable);
	}
	
	return true;
}

bool FXInUInteractableList::UnRegisterInteractable(AActor* Interactable)
{
	UXInUInteractableComponent* InteractableComponent = UXInUInteractionUtilLibrary::GetInteractableComponent(Interactable);
	if (!InteractableComponent) return false;

	FGameplayTag Channel;
	if (!InteractableComponent->GetInteractionChannel(Channel)) return false;
	
	int32 RemovedFromUnAvailable = UnAvailable.FindOrAdd(Channel).Interactables.Remove(Interactable);
	int32 RemovedFromAvailable = Available.FindOrAdd(Channel).Interactables.Remove(Interactable);

	return RemovedFromUnAvailable > 0 || RemovedFromAvailable > 0;
}

void FXInUInteractableList::GetChannels(TArray<FGameplayTag>& OutChannels)
{
	Available.GetKeys(OutChannels);
}

bool FXInUInteractableList::GetAvailable(const FGameplayTag& Channel, TArray<AActor*>& OutInteractables)
{
	TArray<TWeakObjectPtr<AActor>>& Interactables = Available.FindOrAdd(Channel).Interactables;

	for (auto It = Interactables.CreateIterator() ; It ; ++It)
	{
		if (AActor* Interactable = It->Get())
		{
			OutInteractables.Add(Interactable);
		}
		else
		{
			It.RemoveCurrent();
		}
	}
	return OutInteractables.Num() > 0;
}

AActor* FXInUSelectedInteractable::GetSelected(const FGameplayTag& Channel)
{
	FSelected* Selection = Selected.Find(Channel);
	if (!Selection) return nullptr;

	return Selection->Interactable.Get();
}

bool FXInUSelectedInteractable::IsSelected(const FGameplayTag& Channel, AActor* Interactable)
{
	return GetSelected(Channel) == Interactable;
}

void FXInUSelectedInteractable::UpdateSelection(const FGameplayTag& Channel, AActor* Interactable)
{
	FSelected& Selection = Selected.FindOrAdd(Channel);
	
	Selection.InteractionTimerHandle.Invalidate();
	Selection.Interactable = Interactable;
}

void FXInUSelectedInteractable::InvalidateSelection(const FGameplayTag& Channel, AActor* Interactable)
{
	if (IsSelected(Channel, Interactable))
	{
		UpdateSelection(Channel, nullptr);
	}
}

bool FXInUSelectedInteractable::StartInteractionTimer(const FGameplayTag& Channel, const FTimerDelegate& TimerDelegate, float Duration, bool bClientOnly)
{
	FSelected* Selection = Selected.Find(Channel);
	if (!Selection) return false;
	AActor* Interactable = Selection->Interactable.Get();
	if (!Interactable) return false;

	Selection->bClientOnlyTimer = bClientOnly;
	Interactable->GetWorld()->GetTimerManager().SetTimer(Selection->InteractionTimerHandle, TimerDelegate, Duration, false);
	return true;
}

bool FXInUSelectedInteractable::StopInteractionTimer(const FGameplayTag& Channel)
{
	FSelected* Selection = Selected.Find(Channel);
	if (!Selection) return false;
	AActor* Interactable = Selection->Interactable.Get();
	if (!Interactable) return false;
	
	if (!Interactable->GetWorld()->GetTimerManager().IsTimerActive(Selection->InteractionTimerHandle)) return false;
	Interactable->GetWorld()->GetTimerManager().ClearTimer(Selection->InteractionTimerHandle);
	return true;
}

EXInUInteractionTimerStatus FXInUSelectedInteractable::GetInteractionTimerStatus(const FGameplayTag& Channel)
{
	FSelected* Selection = Selected.Find(Channel);
	if (!Selection) return EXInUInteractionTimerStatus::ETS_None;
	AActor* Interactable = Selection->Interactable.Get();
	if (!Interactable) return EXInUInteractionTimerStatus::ETS_None;

	if (!Interactable->GetWorld()->GetTimerManager().IsTimerActive(Selection->InteractionTimerHandle))
	{
		return EXInUInteractionTimerStatus::ETS_Inactive;
	}
	if (Selection->bClientOnlyTimer) return EXInUInteractionTimerStatus::ETS_ClientSide;
	return EXInUInteractionTimerStatus::ETS_ServerAuth;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*--------------------------------------------------------------------------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UXInUInteractComponent::UXInUInteractComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
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
	
	UpdateSelection();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * UXInUInteractComponent Interface
 */

void UXInUInteractComponent::RegisterInteractable(AActor* Interactable)
{
	if (InteractablesInRage.RegisterInteractable(Interactable))
	{
		UXInUInteractableComponent* InteractableComponent = UXInUInteractionUtilLibrary::GetInteractableComponent(Interactable);
		InteractableComponent->AvailabilityChangedDelegate.AddUObject(this, &ThisClass::UpdateInteractableAvailability);

		FGameplayTag Channel;
		if (InteractableComponent->GetInteractionChannel(Channel))
		{
			UpdateSelectionForChannel(Channel);
		}
	}
}

void UXInUInteractComponent::UnRegisterInteractable(AActor* Interactable)
{
	if (InteractablesInRage.UnRegisterInteractable(Interactable))
	{
		UXInUInteractableComponent* InteractableComponent = UXInUInteractionUtilLibrary::GetInteractableComponent(Interactable);
		InteractableComponent->AvailabilityChangedDelegate.RemoveAll(this);

		FGameplayTag Channel;
		if (InteractableComponent->GetInteractionChannel(Channel))
		{
			UpdateSelectionForChannel(Channel);
		}
	}
}

void UXInUInteractComponent::UpdateInteractableAvailability(AActor* Interactable, bool bAvailable)
{
	InteractablesInRage.RegisterInteractable(Interactable);
}

void UXInUInteractComponent::UpdateSelection()
{
	FTransform AimTransform;
	UXInUInteractionUtilLibrary::GetInteractionAimTransform(GetOwner(), AimTransform);
	FVector AimDirection = AimTransform.GetRotation().Vector();
	FVector AimLocation = AimTransform.GetLocation();
	
	TArray<FGameplayTag> Channels;
	InteractablesInRage.GetChannels(Channels);
	for (const FGameplayTag& Channel : Channels)
	{
		UpdateSelectionInternal(Channel, AimLocation, AimDirection);
	}
}

void UXInUInteractComponent::UpdateSelectionForChannel(const FGameplayTag& Channel)
{
	FTransform AimTransform;
	UXInUInteractionUtilLibrary::GetInteractionAimTransform(GetOwner(), AimTransform);
	FVector AimDirection = AimTransform.GetRotation().Vector();
	FVector AimLocation = AimTransform.GetLocation();
	
	UpdateSelectionInternal(Channel, AimLocation, AimDirection);
}

void UXInUInteractComponent::UpdateSelectionInternal(const FGameplayTag& Channel, const FVector& AimLocation, const FVector& AimDirection)
{
	// Find new preferred interactable
	float MaxCrossProduct = -1.f;
	AActor* NewSelectedActor = nullptr;
	TArray<AActor*> Interactables;
	InteractablesInRage.GetAvailable(Channel, Interactables);
	for (AActor* Interactable : Interactables)
	{
		const float NewCrossProduct = (Interactable->GetActorLocation() - AimLocation).GetSafeNormal() | AimDirection;
		if (NewCrossProduct > MaxCrossProduct)
		{
			MaxCrossProduct = NewCrossProduct;
			NewSelectedActor = Interactable;
		}
	}

	// if we found a new closer interactable, update the status of both old and new
	AActor* OldSelectedActor = SelectedInteractables.GetSelected(Channel);
	if (NewSelectedActor != OldSelectedActor)
	{
		// Cleanup old selection
		InputStopInteraction(Channel);
		UpdateInteractableStatus(Channel, OldSelectedActor, false);

		// Select new
		SelectedInteractables.UpdateSelection(Channel, NewSelectedActor);
		UpdateInteractableStatus(Channel, NewSelectedActor, true);
	}
}

void UXInUInteractComponent::UpdateInteractableStatus(const FGameplayTag& Channel, AActor* Interactable, bool bSelected)
{
	UXInUInteractableComponent* InteractableComponent = UXInUInteractionUtilLibrary::GetInteractableComponent(Interactable);

	
	// TODO: broadcast things from here and from interactable component

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

/*--------------------------------------------------------------------------------------------------------------------*/
/* Interaction */

void UXInUInteractComponent::InputStartInteraction(const FGameplayTag& Channel, const FGameplayTag& Action)
{
	AActor* Interactable = SelectedInteractables.GetSelected(Channel);
	if (!StartInteraction(Interactable, Channel, Action)) return;

	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		// we return if it is a timed interaction with client only timer (if server side timer, or instant, we need to call rpc now)
		if (SelectedInteractables.GetInteractionTimerStatus(Channel) == EXInUInteractionTimerStatus::ETS_ClientSide) return;
		ServerStartInteractionRPC(Interactable, Channel, Action);
	}
}

void UXInUInteractComponent::InputStopInteraction(const FGameplayTag& Channel)
{
	EXInUInteractionTimerStatus TimerStatus = SelectedInteractables.GetInteractionTimerStatus(Channel);
	if (TimerStatus <= EXInUInteractionTimerStatus::ETS_Inactive) return;

	AActor* Interactable = SelectedInteractables.GetSelected(Channel);
	StopInteraction(Interactable, Channel);

	// we do not call rpc if timer was client only
	if (TimerStatus != EXInUInteractionTimerStatus::ETS_ClientSide && GetOwner() && !GetOwner()->HasAuthority())
	{
		ServerStopInteractionRPC(Interactable, Channel);
	}
}

void UXInUInteractComponent::ServerStartInteractionRPC_Implementation(AActor* Interactable, const FGameplayTag& Channel, const FGameplayTag& Action)
{
	StartInteraction(Interactable, Channel, Action);
}

void UXInUInteractComponent::ServerStopInteractionRPC_Implementation(AActor* Interactable, const FGameplayTag& Channel)
{
	StopInteraction(Interactable, Channel);
}

bool UXInUInteractComponent::StartInteraction(AActor* Interactable, const FGameplayTag& Channel, const FGameplayTag& Action)
{
	UXInUInteractableComponent* InteractableComponent = UXInUInteractionUtilLibrary::GetInteractableComponent(Interactable);
	if (!InteractableComponent) return false;
	
	float InteractionTime = InteractableComponent->GetInteractionDuration(Action);
	if (InteractionTime == -1.f) return false;
	
	if (InteractionTime == 0.f)
	{
		Interact(Interactable, Channel, Action);
		return true;
	}

	bool bClientOnlyTimer = InteractableComponent->IsInteractionDurationClientSideOnly(Action);
	return StartInteractionWithDuration(Interactable, Channel, Action, InteractionTime, bClientOnlyTimer);
}

void UXInUInteractComponent::StopInteraction(AActor* Interactable, const FGameplayTag& Channel)
{
	SelectedInteractables.StopInteractionTimer(Channel);
}

void UXInUInteractComponent::Interact(AActor* Interactable, const FGameplayTag& Channel, const FGameplayTag& Action)
{
	//TODO: implement interaction interface
	//TODO: we want to check first the interactable, then this owner for interaction interface stuff
}

bool UXInUInteractComponent::StartInteractionWithDuration(AActor* Interactable, const FGameplayTag& Channel, const FGameplayTag& Action, float Duration, const bool bClientOnly)
{
	FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &ThisClass::InteractionTimerEnded, Interactable, Channel, Action, bClientOnly);
	if (!SelectedInteractables.StartInteractionTimer(Channel, TimerDelegate, Duration, bClientOnly)) return false;

	// Passing timer data to interactable
	if (GetOwner<APawn>() && GetOwner<APawn>()->IsLocallyControlled())
	{
		if (UXInUInteractableComponent* InteractableComponent = UXInUInteractionUtilLibrary::GetInteractableComponent(Interactable))
		{
			FXInUInteractionTimerData InteractionTimerData;
			InteractionTimerData.Action = Action;
			InteractionTimerData.StartTime = GetWorld()->GetTimeSeconds();
			InteractionTimerData.Duration = Duration;
			InteractableComponent->UpdateInteractionTimerData(InteractionTimerData);
		}
	}
	return true;
}

void UXInUInteractComponent::InteractionTimerEnded(AActor* Interactable, const FGameplayTag Channel, const FGameplayTag Action, const bool bClientOnly)
{
	InteractFromTimer(Interactable, Channel, Action);

	if (bClientOnly && GetOwner() && !GetOwner()->HasAuthority())
	{
		ServerInteractFromTimerRPC(Interactable, Channel, Action);
	}
}

void UXInUInteractComponent::InteractFromTimer(AActor* Interactable, const FGameplayTag& Channel, const FGameplayTag& Action)
{
	// since this function is delayed, we check that the interactable is still selected
	if (!SelectedInteractables.IsSelected(Channel, Interactable)) return;
	
	UXInUInteractableComponent* InteractableComponent = UXInUInteractionUtilLibrary::GetInteractableComponent(Interactable);
	if (!InteractableComponent) return;
	
	Interact(Interactable, Channel, Action);

	// Passing timer data to interactable
	if (GetOwner<APawn>() && GetOwner<APawn>()->IsLocallyControlled())
	{
		InteractableComponent->ResetInteractionTimerData();
	}
}

void UXInUInteractComponent::ServerInteractFromTimerRPC_Implementation(AActor* Interactable, const FGameplayTag& Channel, const FGameplayTag& Action)
{
	InteractFromTimer(Interactable, Channel, Action);
}

//~ Interaction
/*--------------------------------------------------------------------------------------------------------------------*/

