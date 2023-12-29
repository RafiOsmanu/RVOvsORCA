// Fill out your copyright notice in the Description page of Project Settings.


#include "CollisionAvoidenceComponent.h"
#include "ORCAvsRVOCharacter.h"

// Sets default values for this component's properties
UCollisionAvoidenceComponent::UCollisionAvoidenceComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UCollisionAvoidenceComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UCollisionAvoidenceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UCollisionAvoidenceComponent::CalculateVelocityObject(const TArray<TScriptInterface<IAgentInterface>>& agents)
{
	// implement VO
	//
	// get owner agent
	IAgentInterface* avoidanceAgent = Cast<IAgentInterface>(GetOwner());
	if (!avoidanceAgent) return;
	//go further with code else return

	//clear m_VelocityObstacle so that now a new vel obstacle is created
	m_VelocityObject.Empty();
	
	//in for loop
	//{
	//calculate rel vel v1 - v2
	for (auto& agentToAvoid : agents)
	{
		FVector relVelocity = avoidanceAgent->GetVelocity() - agentToAvoid->GetVelocity();
		//calc time
		auto timeToCollision = FVector::Distance(avoidanceAgent->GetPosition(), agentToAvoid->GetPosition()) / (relVelocity).Length();

		//if the time to have collision with the other agent based purley on distance and current speed is bigger than relevant time
		//we shouldnt calculate velocity for that agent
		if (timeToCollision > m_MaxTimeRelavancy)
		{

		}

	}


	//check if p1 + rel vel * timeToCollision intersects with minkowski sum of p1 and p2 centered at p2
	// if intersection happend add to m_VelocityObstacle
	//}
}

