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

void UXInUBaseInteractionComponent::ResetInteractionEntries(AActor* Interactable, const FGameplayTag InteractionChannel)
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

float UXInUBaseInteractionComponent::GetDefaultInteractionDurationByTag(const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag)
{
	return -1.f;
}

float UXInUBaseInteractionComponent::GetInteractionDurationByTag(const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag)
{
	return -1.f;
}

float UXInUBaseInteractionComponent::GetInteractionTimeElapsedByTag(const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag)
{
	return -1.f;
}

float UXInUBaseInteractionComponent::GetInteractionTimeLeftByTag(const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag)
{
	return -1.f;
}

/*--------------------------------------------------------------------------------------------------------------------*/


