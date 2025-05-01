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

void FXInUSelectedInteractable::UpdateSelection(const FGameplayTag& Channel, AActor* Interactable)
{
	FSelected& Selection = Selected.FindOrAdd(Channel);
	
	Selection.InteractionTimerHandle.Invalidate();
	Selection.Interactable = Interactable;
}

AActor* FXInUSelectedInteractable::GetSelected(const FGameplayTag& Channel)
{
	FSelected* Selection = Selected.Find(Channel);
	if (!Selection) return nullptr;

	return Selection->Interactable.Get();
}

bool FXInUSelectedInteractable::StopInteractionTimer(const FGameplayTag& Channel)
{
	FSelected* Selection = Selected.Find(Channel);
	if (!Selection) return false;
	
	if (AActor* OldSelection = Selection->Interactable.Get())
	{
		if (!OldSelection->GetWorld()->GetTimerManager().IsTimerActive(Selection->InteractionTimerHandle)) return false;
		OldSelection->GetWorld()->GetTimerManager().ClearTimer(Selection->InteractionTimerHandle);
	}
	return true;
}

bool FXInUSelectedInteractable::IsInteractionTimerActive(const FGameplayTag& Channel)
{
	FSelected* Selection = Selected.Find(Channel);
	if (!Selection) return false;
	AActor* OldSelection = Selection->Interactable.Get();
	if (!OldSelection) return false;
	
	return OldSelection->GetWorld()->GetTimerManager().IsTimerActive(Selection->InteractionTimerHandle);
}

bool FXInUSelectedInteractable::IsInteractionTimerClientSide(const FGameplayTag& Channel)
{
	FSelected* Selection = Selected.Find(Channel);
	if (!Selection) return false;
	
	return Selection->IsClientOnlyTimer;
}

bool FXInUSelectedInteractable::IsInteractionTimerActiveClientSide(const FGameplayTag& Channel)
{
	FSelected* Selection = Selected.Find(Channel);
	if (!Selection) return false;

	if (!Selection->IsClientOnlyTimer) return false;
	AActor* OldSelection = Selection->Interactable.Get();
	if (!OldSelection) return false;
	if (!OldSelection->GetWorld()->GetTimerManager().IsTimerActive(Selection->InteractionTimerHandle)) return false;

	// TODO: finish
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
	}
}

void UXInUInteractComponent::UnRegisterInteractable(AActor* Interactable)
{
	if (InteractablesInRage.UnRegisterInteractable(Interactable))
	{
		UXInUInteractableComponent* InteractableComponent = UXInUInteractionUtilLibrary::GetInteractableComponent(Interactable);
		InteractableComponent->AvailabilityChangedDelegate.RemoveAll(this);
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
}

void UXInUInteractComponent::UpdateInteractableStatus(const FGameplayTag& Channel, AActor* Interactable, bool bSelected)
{
	UXInUInteractableComponent* InteractableComponent = UXInUInteractionUtilLibrary::GetInteractableComponent(Interactable);

	// TODO: broadcast things from here and from interactable component
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* Interaction */

void UXInUInteractComponent::InputStartInteraction(const FGameplayTag& Channel, const FGameplayTag& Action)
{
	AActor* Interactable = SelectedInteractables.GetSelected(Channel);
	if (!StartInteraction(Interactable, Channel, Action)) return;

	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		// we return if it is a timed interaction with client only timer
		if (SelectedInteractables.IsInteractionTimerClientSide(Channel)) return;
		ServerStartInteractionRPC(Interactable, Channel, Action);
	}
}

void UXInUInteractComponent::InputStopInteraction(const FGameplayTag& Channel)
{
	if (!SelectedInteractables.IsInteractionTimerActive(Channel)) return;
	const bool bClientOnlyTimerActive = SelectedInteractables.IsInteractionTimerClientSide(Channel);

	AActor* Interactable = SelectedInteractables.GetSelected(Channel);
	StopInteraction(Interactable, Channel);

	// we do not call rpc if timer was client only
	if (!bClientOnlyTimerActive && GetOwner() && !GetOwner()->HasAuthority())
	{
		ServerStopInteractionRPC(Interactable, Channel);
	}
}

//TODO: do this stuff

void UXInUInteractComponent::ServerStartInteractionRPC_Implementation(AActor* Interactable, const FGameplayTag& Channel, const FGameplayTag& Action)
{
}

void UXInUInteractComponent::ServerStopInteractionRPC_Implementation(AActor* Interactable, const FGameplayTag& Channel)
{
}

bool UXInUInteractComponent::StartInteraction(AActor* Interactable, const FGameplayTag& Channel, const FGameplayTag& Action)
{
}

bool UXInUInteractComponent::StartInteractionWithDuration(AActor* Interactable, const FGameplayTag& Channel, const FGameplayTag& Action, const float InteractionTime)
{
}

void UXInUInteractComponent::StopInteraction(AActor* Interactable, const FGameplayTag& Channel)
{
}

void UXInUInteractComponent::Interact(AActor* Interactable, const FGameplayTag& Channel, const FGameplayTag& Action)
{
	//TODO: implement interaction interface
	//TODO: we want to check first the interactable, then this owner for interaction interface stuff
}

void UXInUInteractComponent::InteractFromTimer(AActor* Interactable, const FGameplayTag& Channel, const FGameplayTag& Action)
{
}

void UXInUInteractComponent::ServerInteractFromTimerRPC_Implementation(AActor* Interactable, const FGameplayTag& Channel, const FGameplayTag& Action)
{
}

//~ Interaction
/*--------------------------------------------------------------------------------------------------------------------*/

