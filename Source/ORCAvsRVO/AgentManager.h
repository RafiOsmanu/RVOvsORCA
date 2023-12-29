// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AgentInterface.h"
#include "AgentManager.generated.h"

/**
 * 
 */
UCLASS()
class ORCAVSRVO_API UAgentManager : public UObject
{
	GENERATED_BODY()

public:
	// Function to get the singleton instance of the manager
	static UAgentManager* GetInstance();

	// Function to register agents that implement IAgentInterface
	void RegisterAgent(const TScriptInterface<IAgentInterface>& agent);

	TArray<TScriptInterface<IAgentInterface>>& GetAllAgents() { return m_Agents; }



private:
	// Array to store references to agents
	TArray<TScriptInterface<IAgentInterface>> m_Agents;

	
};
