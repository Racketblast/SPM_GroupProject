// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_RefreshPlayerLocation.h"
#include "BehaviorTree/BlackboardComponent.h"


UBTService_RefreshPlayerLocation::UBTService_RefreshPlayerLocation()
{
	NodeName = "Force Update Player Location";
	bNotifyBecomeRelevant = true;
	bNotifyTick = true;

	// Hur ofta som servicen körs
	Interval = 1.0f;
	RandomDeviation = 0.0f;
}

uint16 UBTService_RefreshPlayerLocation::GetInstanceMemorySize() const
{
	return sizeof(float); // Hur lång tid som har passerat
}

void UBTService_RefreshPlayerLocation::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	float& TimeSinceLastUpdate = *(float*)NodeMemory;
	TimeSinceLastUpdate += DeltaSeconds;

	const float UpdateInterval = 5.0f;

	if (TimeSinceLastUpdate >= UpdateInterval)
	{
		TimeSinceLastUpdate = 0.0f;

		UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
		if (BlackboardComp)
		{
			BlackboardComp->SetValueAsBool(UpdatePlayerLocationKey.SelectedKeyName, true);
			BlackboardComp->SetValueAsBool(UpdatePlayerLocationKey.SelectedKeyName, false);
		}
	}
}