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

	
}

void AORCAvsRVOCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	//UAgentManager::GetInstance()->ReleaseInstance();
}

void AORCAvsRVOCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
	//GEngine->AddOnScreenDebugMessage(-1, .5f, FColor::Blue, FString::FromInt(m_NeighbouringAgents.Num()));
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, "Tick");

	DrawDebugBox(GetWorld(), GetPosition(), FVector(10, 10, 10), FColor::Orange, false, 10, 0, 2.f);
	DrawDebugDirectionalArrow(GetWorld(),
		FVector(GetPosition2D().X, GetPosition2D().Y, 0),
		FVector(GetPosition2D().X, GetPosition2D().Y, 0) + FVector(GetVelocity2D().X, GetVelocity2D().Y, 0),
		50.f, FColor::Blue, false, 0.05f, 0, 5.f);
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


