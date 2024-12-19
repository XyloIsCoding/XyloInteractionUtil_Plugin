// Copyright XyloIsCoding 2024

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "Interaction/XInUBaseInteractionComponent.h"
#include "Interaction/XInUInteractionInfo.h"
#include "XInUInteractableComponent.generated.h"

struct FXInUInteractionTimerData;

UENUM(BlueprintType)
enum class EXInUInteractableUnselectedBehaviour : uint8
{
	EUB_ComputeInteractions UMETA(DisplayName = "Compute Interactions"),
	EUB_SkipInteractions UMETA(DisplayName = "Skip Interactions"),
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAvailableForInteractionSignature, AActor*, Interactable, const bool, bAvailableForInteraction);

class UXInUInteractableData;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class XYLOINTERACTIONUTIL_API UXInUInteractableComponent : public UXInUBaseInteractionComponent
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
	 * UXInUBaseInteractionComponent Interface
	 */

/*--------------------------------------------------------------------------------------------------------------------*/
	/* InteractionInfoDelegates */

public:
	virtual void ResetInteractionEntries(AActor* Interactable, const FGameplayTag InteractionChannel) override;
	virtual void UpdateInteractionEntries(const FXInUInteractionInfo& InteractionInfo) override;
	
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
	/* Interaction Timer */

public:
	virtual float GetDefaultInteractionDurationByTag(const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag) override;
	virtual float GetInteractionDurationByTag(const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag) override;
	virtual float GetInteractionTimeElapsedByTag(const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag) override;
	virtual float GetInteractionTimeLeftByTag(const FGameplayTag InteractionChannel, const FGameplayTag InteractionTag) override;

/*--------------------------------------------------------------------------------------------------------------------*/
	
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
	UFUNCTION(BlueprintCallable)
	virtual UXInUInteractableData* GetInteractableData() const { return InteractableData; }
	UFUNCTION(BlueprintCallable)
	virtual EXInUInteractableUnselectedBehaviour GetUnselectedBehaviour() const { return UnselectedBehaviour; }
	virtual FGameplayTag GetInteractionChannelTag() const;
private:
	/** Data asset containing the possible interactions and the widgets to show for each of them */
	UPROPERTY(EditAnywhere, Category = "Interaction")
	UXInUInteractableData* InteractableData;
	/** Behaviour of the interaction widget, when this actor is in range but not selected */
	UPROPERTY(EditAnywhere, Category = "Interaction")
	EXInUInteractableUnselectedBehaviour UnselectedBehaviour;
	
public:
	/** To be called when an actor enters the interaction range.
	 * Checks if the actor has an interact component, if so, adds this actor to its inRange list */
	UFUNCTION(BlueprintCallable, Category = "Interaction") 
	virtual void OnEnterInteractRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult, bool bInstantInteraction = false, FGameplayTag InstantInteractionTag = FGameplayTag());
	/** To be called when an actor exits the interaction range.
	 * Checks if the actor has an interact component, if so, removes this actor from its inRange list */
	UFUNCTION(BlueprintCallable, Category = "Interaction") 
	virtual void OnExitInteractRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
/*--------------------------------------------------------------------------------------------------------------------*/
	/* Interaction Timer */
	
public:
	virtual void UpdateInteractionTimerData(const FXInUInteractionTimerData& NewInteractionTimer);
	virtual void ResetInteractionTimerData();
private:
	FXInUInteractionTimerData InteractionTimer;

/*--------------------------------------------------------------------------------------------------------------------*/
	
};
