// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "XInUInteractableComponent.generated.h"

UENUM(BlueprintType)
enum EXInUInteractableUnselectedBehaviour
{
	XInUIUB_ShowInteractions UMETA(DisplayName = "Show Interactions"),
	XInUIUB_ShowDefault UMETA(DisplayName = "Show Default"),
	XInUIUB_ShowNone UMETA(DisplayName = "Don't Show"),
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAvailableForInteractionSignature, AActor*, Interactable, bool, AvailableForInteraction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FShowInteractionWidgetSignature, bool, ShowInteractionWidget);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAddEntryToInteractionWidgetSignature, TSubclassOf<UUserWidget>, InteractionWidgetEntryClass);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FResetInteractionWidgetSignature);

class UXInUInteractableData;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
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
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * Interactable
	 */

public:
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	virtual bool GetAvailableForInteraction() const { return bAvailableForInteraction; }
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	virtual void SetAvailableForInteraction(bool bAvailable);
private:
	UPROPERTY(EditAnywhere, Category = "Interaction", ReplicatedUsing = OnRep_AvailableForInteraction)
	bool bAvailableForInteraction;
protected:
	UFUNCTION()
	virtual void OnRep_AvailableForInteraction();
public:
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FAvailableForInteractionSignature AvailableForInteractionDelegate;
	
public:
	/** Data asset containing the possible interactions and the widgets to show for each of them */
	UPROPERTY(EditAnywhere, Category = "Interaction")
	UXInUInteractableData* InteractableData;

	/** Behaviour of the interaction widget, when this actor is in range but not selected */
	UPROPERTY(EditAnywhere, Category = "Interaction")
	TEnumAsByte<EXInUInteractableUnselectedBehaviour> UnselectedBehaviour;
	
public:
	/** To be called when an actor enters the interaction range.
	 * Checks if the actor has an interact component, if so, adds this actor to its inRange list */
	UFUNCTION(BlueprintCallable, Category = "Interaction") 
	virtual void OnEnterInteractRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult, bool bInstantInteraction = false, FGameplayTag InstantInteractionTag = FGameplayTag());
	/** To be called when an actor exits the interaction range.
	 * Checks if the actor has an interact component, if so, removes this actor from its inRange list */
	UFUNCTION(BlueprintCallable, Category = "Interaction") 
	virtual void OnExitInteractRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
	/** Broadcasts ShowInteractionWidgetDelegate */
	virtual void ShowInteractionWidget(bool bShow);
	/** Broadcasts ResetInteractionWidgetDelegate */
	virtual void ResetInteractionWidget();
	/** Broadcasts AddEntryToInteractionWidgetDelegate, with default interaction widget row */
	virtual void AddDefaultEntryToInteractionWidget();
	/** Broadcasts AddEntryToInteractionWidgetDelegate, with interaction widget row belonging to the interaction and status tags */
	virtual void AddEntryToInteractionWidget(FGameplayTag InteractionTag, FGameplayTag StatusTag);

public:
	/** Used to bind function to show/hide the interaction widget */
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FShowInteractionWidgetSignature ShowInteractionWidgetDelegate;
	/** Used to bind function to add interaction widget rows to the interaction widget */
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FAddEntryToInteractionWidgetSignature AddEntryToInteractionWidgetDelegate;
	/** Used to bind function to remove all widget rows from the interaction widget */
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FResetInteractionWidgetSignature ResetInteractionWidgetDelegate;
};
