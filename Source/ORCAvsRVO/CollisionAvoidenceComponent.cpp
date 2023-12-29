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

void UCollisionAvoidenceComponent::CalculateVelocityObject(const IAgentInterface* agentToAvoid)
{
	// implement VO
	//
	// get owner agent
	IAgentInterface* avoidanceAgent = Cast<IAgentInterface>(GetOwner());
	if (!avoidanceAgent) return;
	//go further with code else return

	//clear m_VelocityObstacle so that now a new vel obstacle is created
	m_VelocityObject.Empty();

	

	//calc time
	//auto timeToCollision = FVector::Distance(avoidanceAgent->GetPosition(), agentToAvoid->GetPosition()) / (relVelocity).Length();

	//if the time to have collision with the other agent based purley on distance and current speed is bigger than relevant time
	//we shouldnt calculate velocity for that agent
	//if (timeToCollision > m_MaxTimeRelavancy) return;

	auto currentVelAngle = avoidanceAgent->GetVelocity().HeadingAngle();
	float angleResolution = 5;
	double CheckRange = FMath::DegreesToRadians(90);
	for (double velAngle{ currentVelAngle - CheckRange }; velAngle < currentVelAngle + CheckRange; velAngle += angleResolution)
	{
		//calculate velocity to check collision from angle and speed
		auto velToCheck = CalcVelocityFromAngleAndSpeed(velAngle, avoidanceAgent->GetVelocity().Length());

		//calculate the relative vel 
		FVector relVelocity = velToCheck - agentToAvoid->GetVelocity();

		auto timeToCollision = FVector::Distance(avoidanceAgent->GetPosition(), agentToAvoid->GetPosition()) / (relVelocity).Length();

		FVector futurePosAvoidenceAgent = avoidanceAgent->GetPosition() + relVelocity * timeToCollision;
		FVector futurePosAgentToAvoid = agentToAvoid->GetPosition() + relVelocity * timeToCollision;

		//check for intersection using velangle to create a velocity vector
		// if intersection is there add to velocity object 

	}

}

FVector UCollisionAvoidenceComponent::CalcVelocityFromAngleAndSpeed(double angle, double speed)
{

	FVector returnVelocity;
	returnVelocity.X = FMath::Cos(angle);
	returnVelocity.Y = FMath::Sin(angle);
	
	returnVelocity *= speed;

	return returnVelocity;
}

