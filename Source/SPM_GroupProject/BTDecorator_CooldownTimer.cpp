// Fill out your copyright notice in the Description page of Project Settings.


#include "BTDecorator_CooldownTimer.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_CooldownTimer::UBTDecorator_CooldownTimer()
{
	NodeName = "Cooldown Timer";
}

bool UBTDecorator_CooldownTimer::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	if (auto* Blackboard = OwnerComp.GetBlackboardComponent())
	{
		float LastTime = Blackboard->GetValueAsFloat(TimeKey);
		float CurrentTime = OwnerComp.GetWorld()->GetTimeSeconds();

		/*UE_LOG(LogTemp, Warning, TEXT("Cooldown check: current=%f, last=%f, passed=%d"),
		CurrentTime, LastTime, (CurrentTime - LastTime) >= Cooldown);*/
		
		return (CurrentTime - LastTime) >= Cooldown;
	}
	return false;
}