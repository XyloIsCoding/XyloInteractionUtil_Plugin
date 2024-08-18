// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "XInUInteractableComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAddEntryToInteractionWidgetSignature, TSubclassOf<UUserWidget>, InteractionWidgetEntryClass);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FResetInteractionWidgetSignature);

class UXInUInteractableData;

UCLASS(BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class XYLOINTERACTIONUTIL_API UXInUInteractableComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UXInUInteractableComponent();

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * UActorComponent Interface
	 */
	
protected:
	virtual void BeginPlay() override;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * Interactable
	 */

public:
	UPROPERTY(EditAnywhere, Category = "Interaction")
	UXInUInteractableData* InteractableData;

public:
	UFUNCTION(BlueprintCallable) 
	virtual void OnEnterInteractRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult, bool bInstantInteraction = false, FGameplayTag InstantInteractionTag = FGameplayTag());
	UFUNCTION(BlueprintCallable) 
	virtual void OnExitInteractRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
	virtual void ResetInteractionWidget();
	virtual void AddEntryToInteractionWidget(FGameplayTag InteractionTag, FGameplayTag StatusTag);

	UPROPERTY(BlueprintAssignable)
	FResetInteractionWidgetSignature ResetInteractionWidgetDelegate;
	UPROPERTY(BlueprintAssignable)
	FAddEntryToInteractionWidgetSignature AddEntryToInteractionWidgetDelegate;
};
