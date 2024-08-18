// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "XInUInteractComponent.generated.h"


class IXInUInteractInterface;
struct FGameplayTag;

UCLASS(BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class XYLOINTERACTIONUTIL_API UXInUInteractComponent : public UPawnComponent
{
	GENERATED_BODY()

public:	
	UXInUInteractComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * UActorComponent Interface
	 */
	
protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 * Interact
	 */

private:
	IXInUInteractInterface* InteractInterface;
protected:
	IXInUInteractInterface* GetInteractInterface();
	
private:
	UPROPERTY()
	TArray<AActor*> InteractablesInRange;
	UPROPERTY()
	AActor* SelectedInteractable;

protected:
	virtual void UpdateSelectedInteractable_Client();
	virtual void UpdateInteractableStatus_Client(AActor* Interactable);
	
public:
	virtual void AddInteractableInRange(AActor* NewInteractable);
	virtual void RemoveInteractableInRange(AActor* NewInteractable);

	virtual void Interact(FGameplayTag InteractionTag);
	UFUNCTION(Server, Reliable)
	virtual void ServerInteract(AActor* Interactable, FGameplayTag InteractionTag);
};
