// Fill out your copyright notice in the Description page of Project Settings.


#include "CollisionAvoidenceComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "ORCAvsRVOCharacter.h"

THIRD_PARTY_INCLUDES_START
#include "ClpSimplex.hpp"
THIRD_PARTY_INCLUDES_END


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
	//AORCAvsRVOCharacter* avoidanceAgent = Cast<AORCAvsRVOCharacter>(GetOwner());
	//auto currVelAngle = avoidanceAgent->GetActorRotation();
	//GEngine->AddOnScreenDebugMessage(-1, 0.02, FColor::Green, FString::Printf(TEXT("Current Vel Angle: %f"), (currVelAngle.Yaw)));

	// ...
}


void UCollisionAvoidenceComponent::CalculateVelocityObject(const AORCAvsRVOCharacter* agentToAvoid, const bool avoidCollision)
{
	// get owner agent
	AORCAvsRVOCharacter* avoidanceAgent = Cast<AORCAvsRVOCharacter>(GetOwner());
	auto characterMovement = avoidanceAgent->GetCharacterMovement();
	if (!avoidanceAgent || !agentToAvoid) return;

	//clear m_VelocityObstacle so that now a new vel obstacle is created
	m_VelocityObject.Empty();

	//check to see if we are on collision course in the first place
	if (!IsOnCollisionCourse(agentToAvoid)) return;

	auto agentRotator = avoidanceAgent->GetActorRotation();
	auto currVelAngle = FMath::DegreesToRadians(agentRotator.Yaw);
	float angleResolution = FMath::DegreesToRadians(5.f);
	double CheckRange = FMath::DegreesToRadians(180);


	for (double velAngle = currVelAngle - CheckRange; velAngle <= currVelAngle + CheckRange; velAngle += angleResolution)
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
			//GEngine->AddOnScreenDebugMessage(-1, 100, FColor::Green, "VelocityAdded");
			m_VelocityObject.Add(relVelocity);
		}
	}

	//choosing a vel outside of the velocity object and adding it to the velcity of agent
	if (avoidCollision) AvoidCollision(avoidanceAgent);
	

	if (!m_DrawDebug) return;

	//GEngine->AddOnScreenDebugMessage(-1, 100, FColor::Blue, FString::FromInt(m_VelocityObject.Num()));

	for (const auto& collideVel : m_VelocityObject)
	{

		if (collideVel == m_VelocityObject[0])
		{
			DrawDebugDirectionalArrow(GetWorld(),
				FVector(avoidanceAgent->GetPosition2D().X, avoidanceAgent->GetPosition2D().Y, 0),
				FVector(avoidanceAgent->GetPosition2D().X, avoidanceAgent->GetPosition2D().Y, 0) + FVector(collideVel.X, collideVel.Y, 0),
				50.f, FColor::Red, false, 0.5f, 0, 8.f);

			//GEngine->AddOnScreenDebugMessage(-1, 100, FColor::Red, "range start");
			continue;
		}

		if (collideVel == m_VelocityObject[m_VelocityObject.Num() - 1])
		{
			DrawDebugDirectionalArrow(GetWorld(),
				FVector(avoidanceAgent->GetPosition2D().X, avoidanceAgent->GetPosition2D().Y, 0),
				FVector(avoidanceAgent->GetPosition2D().X, avoidanceAgent->GetPosition2D().Y, 0) + FVector(collideVel.X, collideVel.Y, 0),
				50.f, FColor::Blue, false, 0.5f, 0, 8.f);

			//GEngine->AddOnScreenDebugMessage(-1, 100, FColor::Blue, "range end");
		}
		/*DrawDebugDirectionalArrow(GetWorld(),
			FVector(avoidanceAgent->GetPosition2D().X, avoidanceAgent->GetPosition2D().Y, 0),
			FVector(avoidanceAgent->GetPosition2D().X, avoidanceAgent->GetPosition2D().Y, 0) + FVector(collideVel.X, collideVel.Y, 0),
			50.f, FColor::Red, false, 0.5f, 0, 2.f);*/

		/*DrawDebugLine(
			GetWorld(),
			FVector(avoidanceAgent->GetPosition2D().X, avoidanceAgent->GetPosition2D().Y, 0),
			FVector(avoidanceAgent->GetPosition2D().X, avoidanceAgent->GetPosition2D().Y, 0) + FVector(collideVel.X, collideVel.Y, 0),
			FColor::Green,
			false,
			0.5f, 0, 3.f);*/

	}
}

double UCollisionAvoidenceComponent::NormalizeAngle(double angle)
{
	
	return angle;
}

void UCollisionAvoidenceComponent::CalculateOrcaLine(const AORCAvsRVOCharacter* agentToAvoid)
{
	AORCAvsRVOCharacter* avoidanceAgent = Cast<AORCAvsRVOCharacter>(GetOwner());
	if (!avoidanceAgent || !agentToAvoid) return;
	if (m_VelocityObject.Num() < 2) return;
	if (!IsOnCollisionCourse(agentToAvoid)) return;

	//calculate the relative vel 
	FVector2D relVelocity = avoidanceAgent->GetVelocity2D() - agentToAvoid->GetVelocity2D();
	
	auto timeToCollision = FVector::Distance(avoidanceAgent->GetPosition(), agentToAvoid->GetPosition()) / relVelocity.Size();


	if (timeToCollision > m_MaxTimeRelavancy) return;
	FVector2D futurePosAvoidenceAgent = avoidanceAgent->GetPosition2D() + (avoidanceAgent->GetVelocity2D() * timeToCollision);
	FVector2D futurePosAgentToAvoid = agentToAvoid->GetPosition2D() + (agentToAvoid->GetVelocity2D() * timeToCollision);


	if (IsIntersecting(futurePosAvoidenceAgent, futurePosAgentToAvoid, avoidanceAgent->GetRadius(), agentToAvoid->GetRadius()))
	{
		FVector2D minimalAvoidVel = FindVectorToClosestPointOnVOBound(relVelocity);
		
		//GEngine->AddOnScreenDebugMessage(-1, 100, FColor::Green, "Relative Velocity: " + FString::FromInt(relVelocity.X) + " , " + FString::FromInt(relVelocity.Y));
		//GEngine->AddOnScreenDebugMessage(-1, 100, FColor::Blue, "Current Velocity: " + FString::FromInt(avoidanceAgent->GetVelocity2D().X) + " , " + FString::FromInt(avoidanceAgent->GetVelocity2D().Y));
		//GEngine->AddOnScreenDebugMessage(-1, 100, FColor::Red, "MidPoint: " + FString::FromInt(midPoint.X) + " , " + FString::FromInt(midPoint.Y));
		//GEngine->AddOnScreenDebugMessage(-1, 100, FColor::Orange, "Direction: " + FString::FromInt(direction.X) + " , " + FString::FromInt(direction.Y));

		if (minimalAvoidVel == FVector2D()) return;

		//point in which the orca line should pass through
		FVector2D midPoint = avoidanceAgent->GetVelocity2D() + (minimalAvoidVel * 0.5);

		FVector2D debugDirection = FVector2D(-minimalAvoidVel.Y, minimalAvoidVel.X).GetSafeNormal();

		//GEngine->AddOnScreenDebugMessage(-1, 100, FColor::Green, "OrcaLineAdded");
		m_OrcaLines.Add((OrcaLine{ midPoint, minimalAvoidVel.GetSafeNormal()}));

		if (!m_DrawDebug) return;

		//arrow of velocity
		/*DrawDebugDirectionalArrow(GetWorld(), FVector(avoidanceAgent->GetPosition().X, avoidanceAgent->GetPosition().Y, 0),
			FVector(avoidanceAgent->GetPosition().X, avoidanceAgent->GetPosition().Y, 0) + FVector(avoidanceAgent->GetVelocity().X, avoidanceAgent->GetVelocity().Y, 0),
			20, FColor::Red, false, 5, 0, 4);*/

		//arrow to orca line
		DrawDebugDirectionalArrow(GetWorld(), FVector(avoidanceAgent->GetPosition().X, avoidanceAgent->GetPosition().Y, 0) + FVector(avoidanceAgent->GetVelocity().X, avoidanceAgent->GetVelocity().Y, 0),
			FVector(avoidanceAgent->GetPosition().X, avoidanceAgent->GetPosition().Y, 0) + (FVector(midPoint.X, midPoint.Y, 0)),
			20, FColor::Yellow, false, 5, 0, 4);


		//orca line
		DrawDebugDirectionalArrow(GetWorld(), FVector(avoidanceAgent->GetPosition().X, avoidanceAgent->GetPosition().Y, 0) + FVector(midPoint.X, midPoint.Y, 0),
			FVector(avoidanceAgent->GetPosition().X, avoidanceAgent->GetPosition().Y, 0) + FVector(midPoint.X, midPoint.Y, 0) + (FVector(debugDirection.X, debugDirection.Y, 0) * 200 ),
			20, FColor::Purple, false, 5, 0, 4);

		DrawDebugDirectionalArrow(GetWorld(),
			FVector(avoidanceAgent->GetPosition2D().X, avoidanceAgent->GetPosition2D().Y, 0),
			FVector(avoidanceAgent->GetPosition2D().X, avoidanceAgent->GetPosition2D().Y, 0) + FVector(relVelocity.X, relVelocity.Y, 0),
			50.f, FColor::Black, false, 0.5f, 0, 2.f);

	}

}

void UCollisionAvoidenceComponent::ChooseOptimalVelocity()
{
	if (m_OrcaLines.Num() <= 0) return;
	
	AORCAvsRVOCharacter* avoidanceAgent = Cast<AORCAvsRVOCharacter>(GetOwner());
	auto characterMovement = avoidanceAgent->GetCharacterMovement();
	float maxSpeed = characterMovement->MaxWalkSpeed;

	ClpSimplex model;
	
	// Define variables for vx and vy
	int numCols = 2;
	double colLowerBound[2] = { -maxSpeed, -maxSpeed }; // Lower bounds 
	double colUpperBound[2] = { maxSpeed, maxSpeed };   // Upper bounds 
	double objective[2];                        // Objective function coefficients

	// Set preferred velocities as the objective
	auto currentDirection = avoidanceAgent->GetVelocity2D().GetSafeNormal();
	objective[0] = currentDirection.X; // Preferred velocity in x -> A coefficient 
	objective[1] = currentDirection.Y; // Preferred velocity in y -> B coefficient

	// Add columns (variables) to the model
	model.addColumns(numCols, colLowerBound, colUpperBound, objective, nullptr, nullptr, nullptr);


	// Add constraints for each ORCA line
	for (const OrcaLine& line : m_OrcaLines) {
		//A* vx + B * vy ≤ C
		//line.point is a point on the ORCA line and line.direction is the normal vector of the ORCA line
		FVector2D normal = line.direction;
		double C = FVector2D::DotProduct(normal, line.point); // Calculate C based on the ORCA line equation

		if (m_DrawDebug)
		{
			GEngine->AddOnScreenDebugMessage(-1, 100, FColor::Orange,
				"Preferred Velocity Direction: "
				+ FString::SanitizeFloat(C));
		}

		// Coefficients for the inequality A*vx + B*vy ≤ C
		double elements[2] = { normal.X, normal.Y };
		int columns[2] = { 0, 1 }; // Indices of the variables (velocityX and velocityY)

		// Add the row constraint to the model
		model.addRow(2, columns, elements, -COIN_DBL_MAX, C);
	}

	// Solve the problem
	model.primal();

	// Get the solution
	FVector optimalVelocity(-model.getColSolution()[0], -model.getColSolution()[1], 0);

	auto optimalDirection = optimalVelocity.GetSafeNormal();

	if (m_DrawDebug)
	{
		DrawDebugDirectionalArrow(GetWorld(),
			FVector(avoidanceAgent->GetPosition2D().X, avoidanceAgent->GetPosition2D().Y, 0),
			FVector(avoidanceAgent->GetPosition2D().X, avoidanceAgent->GetPosition2D().Y, 0) + FVector(optimalVelocity.X, optimalVelocity.Y, 0),
			50.f, FColor::Orange, false, 10, 0, 10.f);

		GEngine->AddOnScreenDebugMessage(-1, 100, FColor::Green,
			"Preferred Velocity Direction: "
			+ FString::SanitizeFloat(optimalDirection.X)
			+ " , "
			+ FString::SanitizeFloat(optimalDirection.Y));

		GEngine->AddOnScreenDebugMessage(-1, 100, FColor::Blue,
			"Preferred Velocity Direction: "
			+ FString::SanitizeFloat(currentDirection.X)
			+ " , "
			+ FString::SanitizeFloat(currentDirection.Y));
	}

	characterMovement->Velocity = optimalVelocity;

	//empty out array after getting the right velocity
	m_OrcaLines.Empty();
}

FVector2D UCollisionAvoidenceComponent::FindVectorToClosestPointOnVOBound(FVector2D startPoint)
{
	auto velRangeStart = m_VelocityObject[0];
	auto velRangeEnd = m_VelocityObject[m_VelocityObject.Num() - 1];

	if (velRangeStart.Size() == 0 || velRangeEnd.Size() == 0) return FVector2D();

	//calculates vec to closest point on the start range vec
	auto smallestVecToStartRange = FMath::ClosestPointOnInfiniteLine(FVector(), FVector(velRangeStart.X, velRangeStart.Y, 0), FVector(startPoint.X, startPoint.Y, 0));
	smallestVecToStartRange -= FVector(startPoint.X, startPoint.Y, 0);
	//calculates vec to closest point on the end range vec
	auto smallestVecToEndRange = FMath::ClosestPointOnInfiniteLine(FVector(), FVector(velRangeEnd.X, velRangeEnd.Y, 0), FVector(startPoint.X, startPoint.Y, 0));
	smallestVecToEndRange -= FVector(startPoint.X, startPoint.Y, 0);

	int randNumb;
	if (smallestVecToStartRange.Size() == smallestVecToEndRange.Size())
	{
		randNumb = FMath::FRandRange(0, 1);
	}

	// Calculate the new angle with damping
	if (smallestVecToStartRange.Size() < smallestVecToEndRange.Size() || randNumb == 0)
	{
		return FVector2D(smallestVecToStartRange.X, smallestVecToStartRange.Y);
	}
	else if (smallestVecToEndRange.Size() < smallestVecToStartRange.Size() || randNumb == 1)
	{
		return FVector2D(smallestVecToEndRange.X, smallestVecToEndRange.Y);
	}
	
	return FVector2D();
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


	//ClpSimplex model;
	//{
	//// Define variables for vx and vy
	//int numCols = 2;
	//double colLowerBound[2] = { 0, 0 }; // Lower bounds 
	//double colUpperBound[2] = { maxSpeed, maxSpeed };   // Upper bounds 
	//double objective[2];                        // Objective function coefficients

	//// Set preferred velocities as the objective
	//objective[0] = avoidanceAgent->GetVelocity2D().X; // Preferred velocity in x -> A coefficient 
	//objective[1] = avoidanceAgent->GetVelocity2D().Y; // Preferred velocity in y -> B coefficient

	//// Add columns (variables) to the model
	//model.addColumns(numCols, colLowerBound, colUpperBound, objective, nullptr, nullptr, nullptr);

	//// For each ORCA line
	//for (const auto& orcaLine : m_OrcaLines) {
	//	// Constraint: A*vx + B*vy ? C
	//	double elements[2] = { orcaLine.direction.X, orcaLine.direction.Y };
	//	int columns[2] = { 0, 1 }; // Column indices for vx and vy
	//	double rowUpperBound = orcaLine.point.X * orcaLine.direction.X + orcaLine.point.Y * orcaLine.direction.Y; // Upper bound
	//	// Add the constraint to the model
	//	model.addRow(2, columns, elements, -COIN_DBL_MAX, rowUpperBound);
	//}

	//// Solve the problem
	//model.primal();

	//if (!model.isProvenOptimal())
	//{
	//	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, "NOT OPTIMAL");
	//	return;
	//}
	//// Retrieve the solution
	//double* solution = model.primalColumnSolution();
	//double optimalvelX = solution[0];
	//double optimalvelY = solution[1];

	//GEngine->AddOnScreenDebugMessage(-1, 100, FColor::Green, "Current Velocity: " + FString::FromInt(avoidanceAgent->GetVelocity().X) + " , " + FString::FromInt(avoidanceAgent->GetVelocity().Y));
	//GEngine->AddOnScreenDebugMessage(-1, 100, FColor::Red, "Optimal Avoidance Velocity: " + FString::FromInt(optimalvelX) + " , " + FString::FromInt(optimalvelY));

	//characterMovement->Velocity = FVector( optimalvelX, optimalvelY, 0 );
	//}