// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AgentInterface.h"
#include "ORCAvsRVOCharacter.generated.h"

UCLASS(Blueprintable)
class AORCAvsRVOCharacter : public ACharacter, public IAgentInterface
{
	GENERATED_BODY()

public:
	AORCAvsRVOCharacter();

	virtual void BeginPlay() override;

	// Called every frame.
	virtual void Tick(float DeltaSeconds) override;

	virtual FVector GetPosition() const override;
	virtual FVector GetVelocity() const override;

private:
	//members
	TArray<TScriptInterface<IAgentInterface>> m_NeighbouringAgents;

	//methods
	void InitializeNeighbours();

	
	
};

