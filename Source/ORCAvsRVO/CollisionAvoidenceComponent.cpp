// Fill out your copyright notice in the Description page of Project Settings.


#include "CollisionAvoidenceComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "ORCAvsRVOCharacter.h"
#include "ClpSimplex.hpp"


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

	ClpSimplex model;
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
	float angleResolution = FMath::DegreesToRadians(5.f);
	double CheckRange = FMath::DegreesToRadians(180);


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
			m_VelocityObject.Add(velToCheck);
			if (m_DrawDebug)
			{
				//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, "Added Potential Hit Vel!");
			}
		}
	}

	//choosing a vel outside of the velocity object andding it to the velcity of agent
	if (avoidCollision) AvoidCollision(avoidanceAgent);
	

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

	if (!agentToAvoid) return false;
	//check to see if we are on collision course in the first place

	AORCAvsRVOCharacter* avoidanceAgent = Cast<AORCAvsRVOCharacter>(GetOwner());

	

	if (FVector2D::Distance(avoidanceAgent->GetPosition2D(), agentToAvoid->GetPosition2D()) >
		FVector2D::Distance(avoidanceAgent->GetPosition2D(), avoidanceAgent->GetDestinationPos2D())) return false;

	FVector2D relVelocity = avoidanceAgent->GetVelocity2D() - agentToAvoid->GetVelocity2D();


	auto timeToCollision = FVector::Distance(avoidanceAgent->GetPosition(), agentToAvoid->GetPosition()) / relVelocity.Size();

	FVector2D futurePosAvoidenceAgent = avoidanceAgent->GetPosition2D() + (avoidanceAgent->GetVelocity2D() * timeToCollision);
	FVector2D futurePosAgentToAvoid = agentToAvoid->GetPosition2D() + (agentToAvoid->GetVelocity2D() * timeToCollision);


	if (IsIntersecting(futurePosAvoidenceAgent, futurePosAgentToAvoid, avoidanceAgent->GetRadius(), agentToAvoid->GetRadius())) return true;
	
	return false;
}


void UCollisionAvoidenceComponent::AvoidCollision(const AORCAvsRVOCharacter* avoidanceAgent)
{
	//dont avoid collision if you are not moving
	if (avoidanceAgent->GetVelocity().Size() <= 0) return;
	if (!avoidanceAgent->m_DestinationActor) return;
	if (m_VelocityObject.Num() <= 0) return;

	auto velRangeStart = m_VelocityObject.Top();
	auto velRangeEnd = m_VelocityObject.Last();

	//calculate desired velocity
	auto destinationPos = avoidanceAgent->m_DestinationActor->GetActorLocation();
	FVector2D destinationPos2D = FVector2D(destinationPos.X, destinationPos.Y);

	auto desiredDirection = (destinationPos2D - avoidanceAgent->GetPosition2D()).GetSafeNormal();
	auto desiredVelocity = desiredDirection * avoidanceAgent->GetPosition2D().Size();

	//choose a vel outside of velocity object 
	//auto currentVelAngle = FMath::DegreesToRadians(FMath::Atan2(avoidanceAgent->GetVelocity2D().Y, avoidanceAgent->GetVelocity2D().X));
	auto desiredVelAngle = FMath::DegreesToRadians(FMath::Atan2(desiredVelocity.Y, desiredVelocity.X));
	auto startAngle = FMath::DegreesToRadians(FMath::Atan2(velRangeStart.Y, velRangeStart.X));
	auto endAngle = FMath::DegreesToRadians(FMath::Atan2(velRangeEnd.Y, velRangeEnd.X));

	auto angleDiffToStart = FMath::Abs(desiredVelAngle - startAngle);
	auto angleDiffToEnd = FMath::Abs(desiredVelAngle - endAngle);

	double avoidOffset = FMath::DegreesToRadians(5);
	FVector2D avoidenceVelocity;
	int randSide;

	if (angleDiffToStart == angleDiffToEnd)
	{
		randSide = FMath::FRandRange(0, 1);
	}

	// Calculate the new angle with damping
	double avoidanceAngle;
	if (angleDiffToStart < angleDiffToEnd || randSide == 0)
	{
		avoidanceAngle = startAngle - avoidOffset;
	}
	else if(angleDiffToEnd < angleDiffToStart || randSide == 1)
	{
		avoidanceAngle = endAngle + avoidOffset;
	}

	
	// Calculate the avoidance velocity based on the damped angle
	avoidenceVelocity = CalcVelocityFromAngleAndSpeed(avoidanceAngle, avoidanceAgent->GetVelocity().Size());

	auto characterMovement = avoidanceAgent->GetCharacterMovement();

	//RVO Averaging concept
	auto reciprocalVelocityObstacleAvoidence = (avoidenceVelocity + avoidanceAgent->GetVelocity2D()) * 0.5;
	
	//RVO
	characterMovement->Velocity += (FVector(reciprocalVelocityObstacleAvoidence.X, reciprocalVelocityObstacleAvoidence.Y, 0));

	//Regular VO
	//characterMovement->Velocity += (FVector(avoidenceVelocity.X, avoidenceVelocity.Y, 0));

	float maxSpeed = characterMovement->MaxWalkSpeed;
	float currentSpeed = characterMovement->Velocity.Size();
	float clampedSpeed = FMath::Clamp(currentSpeed, 0.0f, maxSpeed * 1.5);

	characterMovement->Velocity = characterMovement->Velocity.GetSafeNormal() * clampedSpeed;

	
	//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Orange, "AvoidingCollision");
}

