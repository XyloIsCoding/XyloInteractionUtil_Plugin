// Fill out your copyright notice in the Description page of Project Settings.

#include "Interactor/XInUInteractorComponent.h"

#include "XInUInteractionInterface.h"
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

bool FXInUInteractableList::IsAvailable(const FGameplayTag& Channel, AActor* Interactable) const
{
	const FInteractables* InteractablesContainer = Available.Find(Channel);
	if (!InteractablesContainer) return false;
	
	return InteractablesContainer->Interactables.Contains(Interactable);
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

float FXInUSelectedInteractable::GetInteractionProgress(const FGameplayTag& Channel) const
{
	const FSelected* Selection = Selected.Find(Channel);
	if (!Selection) return 0.f;
	AActor* Interactable = Selection->Interactable.Get();
	if (!Interactable) return 0.f;

	float TimeElapsed = Interactable->GetWorld()->GetTimerManager().GetTimerElapsed(Selection->InteractionTimerHandle);
	if (TimeElapsed < 0) return 0.f;
	
	float Duration = Interactable->GetWorld()->GetTimerManager().GetTimerRate(Selection->InteractionTimerHandle);
	return TimeElapsed / Duration;
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

UXInUInteractorComponent::UXInUInteractorComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * UActorComponent Interface
 */

void UXInUInteractorComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UXInUInteractorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	UpdateSelection();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * UXInUInteractorComponent Interface
 */

void UXInUInteractorComponent::ResetInteractionState(AActor* Interactable, const FGameplayTag& Channel)
{
	InteractionInfoResetDelegate.Broadcast(Interactable, Channel);
}

void UXInUInteractorComponent::UpdateInteractionState(const FXInUInteractionInfo& InteractionInfo)
{
	InteractionInfoDelegate.Broadcast(InteractionInfo);
}

float UXInUInteractorComponent::GetInteractionProgress(const FGameplayTag& Channel) const
{
	return SelectedInteractables.GetInteractionProgress(Channel);
}

void UXInUInteractorComponent::RegisterInteractable(AActor* Interactable)
{
	if (InteractablesInRage.RegisterInteractable(Interactable))
	{
		UXInUInteractableComponent* InteractableComponent = UXInUInteractionUtilLibrary::GetInteractableComponent(Interactable);
		InteractableComponent->AvailabilityChangedDelegate.AddUObject(this, &ThisClass::UpdateInteractableAvailability);

		FGameplayTag Channel;
		if (InteractableComponent->GetInteractionChannel(Channel))
		{
			UpdateSelectionForChannel(Channel);
			// If it has been selected, then UpdateInteractableStatus was already run for this Interactable,
			// so we only update if unselected.
			if (!SelectedInteractables.IsSelected(Channel, Interactable))
			{
				UpdateInteractableStatus(Channel, Interactable, false);
			}
		}
	}
}

void UXInUInteractorComponent::UnRegisterInteractable(AActor* Interactable)
{
	if (InteractablesInRage.UnRegisterInteractable(Interactable))
	{
		UXInUInteractableComponent* InteractableComponent = UXInUInteractionUtilLibrary::GetInteractableComponent(Interactable);
		InteractableComponent->AvailabilityChangedDelegate.RemoveAll(this);

		FGameplayTag Channel;
		if (InteractableComponent->GetInteractionChannel(Channel))
		{
			// UpdateSelectionForChannel already runs UpdateInteractableStatus on old and new selection, so if this
			// interactable was selected, there is no need to call UpdateInteractableStatus again
			if (SelectedInteractables.IsSelected(Channel, Interactable))
			{
				UpdateSelectionForChannel(Channel);
			}
			else
			{
				UpdateInteractableStatus(Channel, Interactable, false);
			}
		}
	}
}

void UXInUInteractorComponent::UpdateInteractableAvailability(AActor* Interactable, bool bAvailable)
{
	InteractablesInRage.RegisterInteractable(Interactable);
}

void UXInUInteractorComponent::UpdateSelection()
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

void UXInUInteractorComponent::UpdateSelectionForChannel(const FGameplayTag& Channel)
{
	FTransform AimTransform;
	UXInUInteractionUtilLibrary::GetInteractionAimTransform(GetOwner(), AimTransform);
	FVector AimDirection = AimTransform.GetRotation().Vector();
	FVector AimLocation = AimTransform.GetLocation();
	
	UpdateSelectionInternal(Channel, AimLocation, AimDirection);
}

void UXInUInteractorComponent::UpdateSelectionInternal(const FGameplayTag& Channel, const FVector& AimLocation, const FVector& AimDirection)
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

void UXInUInteractorComponent::UpdateInteractableStatus(const FGameplayTag& Channel, AActor* Interactable, bool bSelected)
{
	UXInUInteractableComponent* InteractableComponent = UXInUInteractionUtilLibrary::GetInteractableComponent(Interactable);
	if (!InteractableComponent)
	{
		ResetInteractionState(Interactable, Channel);
		return;
	}
	
	// if not in range anymore, then reset and return
	if (!InteractablesInRage.IsAvailable(Channel, Interactable))
	{
		InteractableComponent->ResetInteractionState(GetOwner());
		ResetInteractionState(Interactable, Channel);
		return;
	}

	FXInUInteractionInfo InteractionInfo;
	InteractionInfo.Interactor = GetOwner();
	InteractionInfo.Interactable = Interactable;
	InteractionInfo.Channel = Channel;
	InteractionInfo.bSelected = bSelected;

	// if selected, or unselected behaviour is to compute interactions
	// Get all interaction tags from actor's InteractableData, and generate the status for each
	if (bSelected || InteractableComponent->GetUnselectedBehaviour() == EXInUInteractableUnselectedBehaviour::EUB_ComputeInteractions)
	{
		FGameplayTagContainer Actions;
		InteractableComponent->GetSupportedActions(Actions);
		for (FGameplayTag Action : Actions)
		{
			FXInUInteractionState InteractionState;
			CheckInteraction(Interactable, Channel, Action, InteractionState);
			InteractionInfo.AddInteractionState(Action, InteractionState);
		}
	}

	InteractableComponent->UpdateInteractionState(InteractionInfo);
	UpdateInteractionState(InteractionInfo);
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* Interaction Management */

void UXInUInteractorComponent::InputStartInteraction(const FGameplayTag Channel, const FGameplayTag Action)
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

void UXInUInteractorComponent::InputStopInteraction(const FGameplayTag Channel)
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
	
	// Resetting timer data for interactable
	if (GetOwner<APawn>() && GetOwner<APawn>()->IsLocallyControlled())
	{
		if (UXInUInteractableComponent* InteractableComponent = UXInUInteractionUtilLibrary::GetInteractableComponent(Interactable))
		{
			InteractableComponent->ResetInteractionTimerData();
		}
	}
}

void UXInUInteractorComponent::ServerStartInteractionRPC_Implementation(AActor* Interactable, const FGameplayTag& Channel, const FGameplayTag& Action)
{
	StartInteraction(Interactable, Channel, Action);
}

void UXInUInteractorComponent::ServerStopInteractionRPC_Implementation(AActor* Interactable, const FGameplayTag& Channel)
{
	StopInteraction(Interactable, Channel);
}

bool UXInUInteractorComponent::StartInteraction(AActor* Interactable, const FGameplayTag& Channel, const FGameplayTag& Action)
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

void UXInUInteractorComponent::StopInteraction(AActor* Interactable, const FGameplayTag& Channel)
{
	SelectedInteractables.StopInteractionTimer(Channel);
}

bool UXInUInteractorComponent::StartInteractionWithDuration(AActor* Interactable, const FGameplayTag& Channel, const FGameplayTag& Action, float Duration, const bool bClientOnly)
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

void UXInUInteractorComponent::InteractionTimerEnded(AActor* Interactable, const FGameplayTag Channel, const FGameplayTag Action, const bool bClientOnly)
{
	InteractFromTimer(Interactable, Channel, Action);

	if (bClientOnly && GetOwner() && !GetOwner()->HasAuthority())
	{
		ServerInteractFromTimerRPC(Interactable, Channel, Action);
	}
}

void UXInUInteractorComponent::InteractFromTimer(AActor* Interactable, const FGameplayTag& Channel, const FGameplayTag& Action)
{
	// since this function is delayed, we check that the interactable is still selected
	if (!SelectedInteractables.IsSelected(Channel, Interactable)) return;
	
	UXInUInteractableComponent* InteractableComponent = UXInUInteractionUtilLibrary::GetInteractableComponent(Interactable);
	if (!InteractableComponent) return;
	
	Interact(Interactable, Channel, Action);

	// Resetting timer data for interactable
	if (GetOwner<APawn>() && GetOwner<APawn>()->IsLocallyControlled())
	{
		InteractableComponent->ResetInteractionTimerData();
	}
}

void UXInUInteractorComponent::ServerInteractFromTimerRPC_Implementation(AActor* Interactable, const FGameplayTag& Channel, const FGameplayTag& Action)
{
	InteractFromTimer(Interactable, Channel, Action);
}

//~ Interaction Management
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Interaction Logic */

bool UXInUInteractorComponent::CheckInteraction(AActor* Interactable, const FGameplayTag& Channel, const FGameplayTag& Action, FXInUInteractionState OutState)
{
	bool bInteractionFound = false;

	// Try interactable side
	if (IXInUInteractionInterface* InteractableInteraction = Cast<IXInUInteractionInterface>(Interactable))
	{
		bInteractionFound = InteractableInteraction->CanInteract(GetOwner(), Channel, Action, OutState);
	}

	// Try interactor side
	if (!bInteractionFound)
	{
		if (IXInUInteractionInterface* InteractorInteraction = Cast<IXInUInteractionInterface>(GetOwner()))
		{
			bInteractionFound = InteractorInteraction->CanInteract(Interactable, Channel, Action, OutState);
		}
	}

	return bInteractionFound;
}

void UXInUInteractorComponent::Interact(AActor* Interactable, const FGameplayTag& Channel, const FGameplayTag& Action)
{
	FXInUInteractionState InteractionState;
	bool bInteractionFound = false;

	// Try interactable side
	if (IXInUInteractionInterface* InteractableInteraction = Cast<IXInUInteractionInterface>(Interactable))
	{
		bInteractionFound = InteractableInteraction->CanInteract(GetOwner(), Channel, Action, InteractionState);
		if (bInteractionFound)
		{
			InteractableInteraction->TryInteract(GetOwner(), Channel, Action);
		}
	}

	// Try interactor side
	if (!bInteractionFound)
	{
		if (IXInUInteractionInterface* InteractorInteraction = Cast<IXInUInteractionInterface>(GetOwner()))
		{
			bInteractionFound = InteractorInteraction->CanInteract(Interactable, Channel, Action, InteractionState);
			if (bInteractionFound)
			{
				InteractorInteraction->TryInteract(Interactable, Channel, Action);
			}
		}
	}
}

//~ Interaction Logic
/*--------------------------------------------------------------------------------------------------------------------*/

