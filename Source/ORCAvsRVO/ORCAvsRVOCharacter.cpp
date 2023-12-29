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

	//register self to the manager that keeps all agents
	UAgentManager::GetInstance()->RegisterAgent(this);
}

void AORCAvsRVOCharacter::BeginPlay()
{
	InitializeNeighbours();
}

void AORCAvsRVOCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
}

FVector AORCAvsRVOCharacter::GetPosition() const
{
	//return current position
	return this->GetPosition();
}

FVector AORCAvsRVOCharacter::GetVelocity() const
{
	//return current velocity
	return this->GetVelocity();
	
	
}

void AORCAvsRVOCharacter::InitializeNeighbours()
{
	auto allAgents = UAgentManager::GetInstance()->GetAllAgents();

	for (const auto& agent : allAgents)
	{
		if (agent != this)
		{
			m_NeighbouringAgents.Add(agent);
		}
	}
}


