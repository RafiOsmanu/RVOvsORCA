// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AgentInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UAgentInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ORCAVSRVO_API IAgentInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	virtual FVector GetPosition() const = 0;
	virtual FVector GetVelocity() const = 0;
	virtual float GetRadius() const = 0;
};
