// Copyright 2025, Davide Bosco. All Rights Reserved.


#include "XInUInteractionTypes.h"

void FXInUInteractionTimerData::Reset()
{
	StartTime = 0.f;
	Duration = 0.f;
}

FXInUInteractionState* FXInUInteractionInfo::GetInteractionState(const FGameplayTag& Action)
{
	return InteractionsState.Find(Action);
}

const FXInUInteractionState* FXInUInteractionInfo::GetInteractionStateConst(const FGameplayTag& Action) const
{
	return InteractionsState.Find(Action);
}

void FXInUInteractionInfo::AddInteractionState(const FGameplayTag& Action, const FXInUInteractionState& State)
{
	InteractionsState.FindOrAdd(Action, State);
}
