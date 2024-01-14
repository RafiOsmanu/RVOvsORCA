// Copyright Epic Games, Inc. All Rights Reserved.

#include "ORCAvsRVOCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Materials/Material.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include <chrono>
#include "Engine/World.h"

#include "AgentManager.h"


AORCAvsRVOCharacter::AORCAvsRVOCharacter()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;



	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, "Lets GOO");

	m_AvoidanceComponent = CreateDefaultSubobject<UCollisionAvoidenceComponent>(TEXT("AvoidanceComponent"));
}

void AORCAvsRVOCharacter::BeginPlay()
{
	Super::BeginPlay();
	InitializeNeighbours();
	m_StartTrackingComputation = false;
	m_MaxTraceAmount = 100;

	
}



void AORCAvsRVOCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	//UAgentManager::GetInstance()->ReleaseInstance();
}

void AORCAvsRVOCharacter::OnBeginOverlap(AActor* OtherActor)
{
	if (Cast<AORCAvsRVOCharacter>(OtherActor))
	{
		++m_HitCount;
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, "Succesfull Cast");
	}
	else
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Cast Failed");
	}

	PrintTotalCollisionCount();
}

void AORCAvsRVOCharacter::StartPerformanceTrace()
{
	if(m_TrackComputationSpeed)
	m_StartTrackingComputation = true;

	if (m_ComputationTimes.Num() >= m_MaxTraceAmount)
	{
		long long sum{0};
		for (const auto& computeTime : m_ComputationTimes)
		{
			sum += computeTime;
		}

		auto average = sum / m_MaxTraceAmount;
		UE_LOG(LogTemp, Warning, TEXT("Average: %lld nanoseconds"), average);
	}
}

void AORCAvsRVOCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

	DrawDebugBox(GetWorld(), GetPosition(), FVector(5, 5, 5), FColor::Orange, false, 10, 0, 2.f);
	/*DrawDebugDirectionalArrow(GetWorld(),
		FVector(GetPosition2D().X, GetPosition2D().Y, 0),
		FVector(GetPosition2D().X, GetPosition2D().Y, 0) + FVector(GetVelocity2D().X, GetVelocity2D().Y, 0),
		50.f, FColor::Blue, false, 0.05f, 0, 5.f);*/
	//DrawDebugSphere(GetWorld(), GetPosition(), m_AgentRadius, 26, FColor::Magenta, false, -1, 0, 1.f);
	

	//GEngine->AddOnScreenDebugMessage(-1, .5f, FColor::Blue, "X: " + FString::FromInt(GetVelocity2D().X) + ", Y: " + FString::FromInt(GetVelocity2D().Y));

}

FVector AORCAvsRVOCharacter::GetPosition() const
{
	//return current position
	return GetActorLocation();
}

FVector2D AORCAvsRVOCharacter::GetPosition2D() const
{
	
	//return current position
	return FVector2D(GetActorLocation().X, GetActorLocation().Y);
}

FVector AORCAvsRVOCharacter::GetVelocity() const
{
	//return current velocity
	return GetCharacterMovement()->Velocity;
}

FVector2D AORCAvsRVOCharacter::GetVelocity2D() const
{
	//return current velocity
	return FVector2D(GetCharacterMovement()->Velocity.X, GetCharacterMovement()->Velocity.Y);


}

FVector2D AORCAvsRVOCharacter::GetDestinationPos2D() const
{
	return FVector2D(m_DestinationActor->GetActorLocation().X, m_DestinationActor->GetActorLocation().Y);
}

float AORCAvsRVOCharacter::GetRadius() const
{
	return m_AgentRadius;
}

void AORCAvsRVOCharacter::CalculateVelocityObject()
{
	std::chrono::steady_clock::time_point start;
	switch (m_AvoidenceType)
	{
	case AvoidanceType::RVO:
		

		if (m_StartTrackingComputation && m_TraceAmount <= m_MaxTraceAmount)
		{
			start = std::chrono::high_resolution_clock::now();
			
		}

		for (const auto& neighborAgent : m_NeighbouringAgents)
		{
			if (!neighborAgent) continue;
			//VO / RVO
			m_AvoidanceComponent->CalculateVelocityObject(neighborAgent, true);
		}

		if (m_StartTrackingComputation && m_TraceAmount <= m_MaxTraceAmount)
		{
			// Stop measuring time
			auto stop = std::chrono::high_resolution_clock::now();

			// Calculate the duration
			auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);

			// Output the time taken
			long long durationDouble = duration.count();

			UE_LOG(LogTemp, Warning, TEXT("Time taken: %lld nanoseconds"), durationDouble);
			m_ComputationTimes.Add(durationDouble);
			++m_TraceAmount;
		}

		break;
	case AvoidanceType::ORCA:

		if (m_StartTrackingComputation && m_TraceAmount <= m_MaxTraceAmount)
		{
			start = std::chrono::high_resolution_clock::now();

		}

		for (const auto& neighborAgent : m_NeighbouringAgents)
		{
			if (!neighborAgent) continue;
			//VO / RVO
			m_AvoidanceComponent->CalculateVelocityObject(neighborAgent, false);

			//ORCA
			m_AvoidanceComponent->CalculateOrcaLine(neighborAgent);
		}
		//ORCA
		m_AvoidanceComponent->ChooseOptimalVelocity();

		if (m_StartTrackingComputation && m_TraceAmount <= m_MaxTraceAmount)
		{
			// Stop measuring time
			auto stop = std::chrono::high_resolution_clock::now();

			// Calculate the duration
			auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);

			// Output the time taken
			long long durationDouble = duration.count();

			UE_LOG(LogTemp, Warning, TEXT("Time taken: %lld nanoseconds"), durationDouble);
			m_ComputationTimes.Add(durationDouble);
			++m_TraceAmount;
		}
		break;
	}
}

void AORCAvsRVOCharacter::PrintTotalCollisionCount()
{
	auto totalHitCount = m_HitCount;

	for (const auto& neighbor : m_NeighbouringAgents)
	{
		totalHitCount += neighbor->m_HitCount;
	}
	// every overlap count up hit for both agent so divide by 2 to make it accurate hitcount
	totalHitCount /= 2;

	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, "Total Hit Count: " + FString::FromInt(m_HitCount));
}

void AORCAvsRVOCharacter::InitializeNeighbours()
{
	TArray<AActor*> allActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AORCAvsRVOCharacter::StaticClass(), allActors);


	TArray<AORCAvsRVOCharacter*> allAgents;

	allAgents.Reserve(allActors.Num()); // Optional: Reserve space for efficiency

	for (AActor* actor : allActors) {
		AORCAvsRVOCharacter* Character = Cast<AORCAvsRVOCharacter>(actor);
		if (Character) {
			allAgents.Add(Character);
		}
	}

	for (const auto& agent : allAgents)
	{
		if (agent != this)
		{
			m_NeighbouringAgents.Add(agent);
		}
	}
}



