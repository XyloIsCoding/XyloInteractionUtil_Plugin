// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/XInUBaseInteractionComponent.h"

#include "GameplayTagContainer.h"

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
/* InteractionInfoDelegates */

void UXInUBaseInteractionComponent::ResetInteractionEntries(const FGameplayTag InteractionChannel, AActor* Interactable)
{
	ResetInteractionEntriesDelegate.Broadcast(InteractionChannel, Interactable);
}

void UXInUBaseInteractionComponent::UpdateInteractionEntries(const FXInUInteractionInfo& InteractionInfo)
{
	UpdateInteractionEntriesDelegate.Broadcast(InteractionInfo);
}

/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Interaction Timer */

float UXInUBaseInteractionComponent::GetInteractionMaxTime(const FGameplayTag InteractionChannel)
{
	return -1.f;
}

float UXInUBaseInteractionComponent::GetInteractionTimeElapsed(const FGameplayTag InteractionChannel)
{
	return -1.f;
}

float UXInUBaseInteractionComponent::GetInteractionTimeLeft(const FGameplayTag InteractionChannel)
{
	return -1.f;
}

/*--------------------------------------------------------------------------------------------------------------------*/


