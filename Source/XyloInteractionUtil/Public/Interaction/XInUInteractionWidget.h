// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "XInUInteractionWidget.generated.h"

struct FGameplayTag;
struct FXInUInteractionInfo;
class UXInUBaseInteractionComponent;

/**
 * 
 */
UCLASS()
class XYLOINTERACTIONUTIL_API UXInUInteractionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UXInUInteractionWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

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
	
public:
	UFUNCTION(BlueprintCallable, Category = "Interaction Widget")
	UXInUBaseInteractionComponent* GetInteractionComponent() const { return InteractionComponent; }
	UFUNCTION(BlueprintCallable, Category = "Interaction Widget")
	virtual void SetInteractionComponent(UXInUBaseInteractionComponent* OwnerInteractionComponent);
	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction Widget", meta = (DisplayName = "Interaction Component Set"))
	void BP_InteractionComponentSet();
private:
	UPROPERTY()
	TObjectPtr<UXInUBaseInteractionComponent> InteractionComponent;

/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
	/* Interaction Data */
	
protected:
	UFUNCTION()
	void OnInteractionUpdated(const FXInUInteractionInfo& InteractionInfo);
	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction Widget", meta = (DisplayName = "On Interaction Updated"))
	void BP_OnInteractionUpdated(const FXInUInteractionInfo& InteractionInfo);
	UFUNCTION()
	void OnInteractionReset(const FGameplayTag InteractionChannel, AActor* Interactable);
	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction Widget", meta = (DisplayName = "On Interaction Reset"))
	void BP_OnInteractionReset(const FGameplayTag InteractionChannel, AActor* Interactable);
private:
	TMap<FGameplayTag, FXInUInteractionInfo> Interactions;
	
/*--------------------------------------------------------------------------------------------------------------------*/
	
};
