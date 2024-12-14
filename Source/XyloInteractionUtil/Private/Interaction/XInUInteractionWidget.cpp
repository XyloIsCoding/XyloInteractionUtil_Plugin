// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/XInUInteractionWidget.h"

#include "Interaction/XInUBaseInteractionComponent.h"
#include "Interaction/XInUInteractionInfo.h"

UXInUInteractionWidget::UXInUInteractionWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * UUserWidget Interface
 */


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * UXInUInteractionWidget
 */

/*--------------------------------------------------------------------------------------------------------------------*/
/* InteractionComponent */

void UXInUInteractionWidget::SetInteractionComponent(UXInUBaseInteractionComponent* OwnerInteractionComponent)
{
	InteractionComponent = OwnerInteractionComponent;

	InteractionComponent->UpdateInteractionEntriesDelegate.AddUniqueDynamic(this, &ThisClass::OnInteractionUpdated);
	InteractionComponent->ResetInteractionEntriesDelegate.AddUniqueDynamic(this, &ThisClass::OnInteractionReset);
	
	BP_InteractionComponentSet();
}

/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Interaction Data */

void UXInUInteractionWidget::OnInteractionUpdated(const FXInUInteractionInfo& InteractionInfo)
{
	if (!Interactions.Contains(InteractionInfo.InteractionChannel))
	{
		Interactions.Emplace(InteractionInfo.InteractionChannel);
	}
	Interactions[InteractionInfo.InteractionChannel] = InteractionInfo;
	
	BP_OnInteractionUpdated(InteractionInfo);
}

void UXInUInteractionWidget::OnInteractionReset(const FGameplayTag InteractionChannel, AActor* Interactable)
{
	Interactions.Remove(InteractionChannel);
	
	BP_OnInteractionReset(InteractionChannel, Interactable);
}

/*--------------------------------------------------------------------------------------------------------------------*/
