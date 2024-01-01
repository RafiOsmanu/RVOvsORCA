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


void UCollisionAvoidenceComponent::CalculateVelocityObject(const AORCAvsRVOCharacter* agentToAvoid, const bool avoidCollision)
{

	// get owner agent
	AORCAvsRVOCharacter* avoidanceAgent = Cast<AORCAvsRVOCharacter>(GetOwner());
	if (!avoidanceAgent || !agentToAvoid) return;

	//clear m_VelocityObstacle so that now a new vel obstacle is created
	m_VelocityObject.Empty();

	//check to see if we are on collision course in the first place
	if (!IsOnCollisionCourse(agentToAvoid)) return;


	auto currentVelAngle = FMath::DegreesToRadians(FMath::Atan2(avoidanceAgent->GetVelocity2D().Y, avoidanceAgent->GetVelocity2D().X));
	float angleResolution = FMath::DegreesToRadians(10.f);
	double CheckRange = FMath::DegreesToRadians(180);

	//first vel that starts the range
	bool startVelFound = false;
	FVector2D velRangeStart;

	//last vel that ends the range
	bool endVelFound = false;
	FVector2D velRangeEnd;

	//velocity obstacle calculation
	for (double velAngle{ currentVelAngle - CheckRange }; velAngle <= currentVelAngle + CheckRange; velAngle += angleResolution)
	{
		//calculate velocity to check collision from angle and speed
		auto velToCheck = CalcVelocityFromAngleAndSpeed(velAngle, avoidanceAgent->GetVelocity().Size());

		
		//calculate the relative vel 
		FVector2D relVelocity = velToCheck - agentToAvoid->GetVelocity2D();
		

		auto timeToCollision = FVector::Distance(avoidanceAgent->GetPosition(), agentToAvoid->GetPosition()) / relVelocity.Size();
		

		if (timeToCollision > m_MaxTimeRelavancy) continue;

		FVector2D futurePosAvoidenceAgent = avoidanceAgent->GetPosition2D() + (velToCheck * timeToCollision);
		FVector2D futurePosAgentToAvoid = agentToAvoid->GetPosition2D() + (agentToAvoid->GetVelocity2D() * timeToCollision);

	
		if (IsIntersecting(futurePosAvoidenceAgent, futurePosAgentToAvoid, avoidanceAgent->GetRadius(), agentToAvoid->GetRadius()))
		{
			//cache the "start and end vel" of VO
			if (!startVelFound) velRangeStart = velToCheck;
			if (velAngle + angleResolution <= currentVelAngle + CheckRange && !endVelFound) velRangeEnd = velToCheck;

			m_VelocityObject.Add(velToCheck);
			if (m_DrawDebug)
			{
				GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, "Added Potential Hit Vel!");
			}
		}
	}

	//choosing a vel outside of the velocity object andding it to the velcity of agent
	if (avoidCollision) AvoidCollision(avoidanceAgent, velRangeStart, velRangeEnd);
	

	if (!m_DrawDebug) return;

	for (const auto& collideVel : m_VelocityObject)
	{
		DrawDebugDirectionalArrow(GetWorld(),
			FVector(avoidanceAgent->GetPosition2D().X, avoidanceAgent->GetPosition2D().Y, 0),
			FVector(avoidanceAgent->GetPosition2D().X, avoidanceAgent->GetPosition2D().Y, 0) + FVector(collideVel.X, collideVel.Y, 0),
			50.f, FColor::Red, false, 0.5f, 0, 2.f);

		DrawDebugLine(
			GetWorld(),
			FVector(avoidanceAgent->GetPosition2D().X, avoidanceAgent->GetPosition2D().Y, 0),
			FVector(avoidanceAgent->GetPosition2D().X, avoidanceAgent->GetPosition2D().Y, 0) + FVector(collideVel.X, collideVel.Y, 0),
			FColor::Green,
			false,
			0.5f, 0, 3.f);

	}

	
	
	

}

FVector2D UCollisionAvoidenceComponent::CalcVelocityFromAngleAndSpeed(double angle, double speed)
{

	FVector2D returnVelocity;
	returnVelocity.X = FMath::Cos(angle);
	returnVelocity.Y = FMath::Sin(angle);
	
	returnVelocity *= speed;

	if (!m_DrawDebug) return returnVelocity;

	//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, FString::FromInt(angle));

	return returnVelocity;
}

bool UCollisionAvoidenceComponent::IsIntersecting(FVector2D futurePos1, FVector2D futurePos2, float radius1, float radius2)
{
	float minkowskiSum = radius1 + radius2;

	if (FVector2D::Distance(futurePos1, futurePos2) <= minkowskiSum) return true;

	return false;
}

bool UCollisionAvoidenceComponent::IsOnCollisionCourse(const AORCAvsRVOCharacter* agentToAvoid)
{
	//check to see if we are on collision course in the first place

	AORCAvsRVOCharacter* avoidanceAgent = Cast<AORCAvsRVOCharacter>(GetOwner());

	FVector2D relVelocity = avoidanceAgent->GetVelocity2D() - agentToAvoid->GetVelocity2D();


	auto timeToCollision = FVector::Distance(avoidanceAgent->GetPosition(), agentToAvoid->GetPosition()) / relVelocity.Size();

	FVector2D futurePosAvoidenceAgent = avoidanceAgent->GetPosition2D() + (avoidanceAgent->GetVelocity2D() * timeToCollision);
	FVector2D futurePosAgentToAvoid = agentToAvoid->GetPosition2D() + (agentToAvoid->GetVelocity2D() * timeToCollision);


	if (IsIntersecting(futurePosAvoidenceAgent, futurePosAgentToAvoid, avoidanceAgent->GetRadius(), agentToAvoid->GetRadius())) return true;
	
	return false;
}


void UCollisionAvoidenceComponent::AvoidCollision(const AORCAvsRVOCharacter* avoidanceAgent, FVector2D velRangeStart, FVector2D velRangeEnd)
{
	//choose a vel outside of velocity object 
	auto currentVelAngle = FMath::DegreesToRadians(FMath::Atan2(avoidanceAgent->GetVelocity2D().Y, avoidanceAgent->GetVelocity2D().X));
	auto startAngle = FMath::DegreesToRadians(FMath::Atan2(velRangeStart.Y, velRangeStart.X));
	auto endAngle = FMath::DegreesToRadians(FMath::Atan2(velRangeEnd.Y, velRangeEnd.X));

	auto angleDiffToStart = FMath::Abs(currentVelAngle - startAngle);
	auto angleDiffToEnd = FMath::Abs(currentVelAngle - endAngle);

	double avoidOffset = FMath::DegreesToRadians(10);

	if (angleDiffToStart == angleDiffToEnd)
	{

	}
	else
	{
		auto closestRangeCap = FMath::Min(angleDiffToStart, angleDiffToEnd);


	}
}

