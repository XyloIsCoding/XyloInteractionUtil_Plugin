// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/XInUBaseInteractionComponent.h"

#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"

// Sets default values for this component's properties
UXInUBaseInteractionComponent::UXInUBaseInteractionComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * UActorComponent Interaction
 */

void UXInUBaseInteractionComponent::BeginPlay()
{
	Super::BeginPlay();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Base Interaction
 */

/*--------------------------------------------------------------------------------------------------------------------*/
/* InteractionWidget */

void UXInUBaseInteractionComponent::ShowInteractionWidget(const bool bShow)
{
	ShowInteractionWidgetDelegate.Broadcast(bShow);
}

void UXInUBaseInteractionComponent::ResetInteractionEntries(const FGameplayTag InteractionChannel)
{
	ResetInteractionEntriesDelegate.Broadcast(InteractionChannel);
}

void UXInUBaseInteractionComponent::AddInteractionEntry(const FXInUInteractionInfo& InteractionInfo)
{
	AddInteractionEntryDelegate.Broadcast(InteractionInfo);
}

/*--------------------------------------------------------------------------------------------------------------------*/


