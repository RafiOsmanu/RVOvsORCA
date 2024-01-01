// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AgentInterface.h"
#include "ORCAvsRVOCharacter.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AgentManager.generated.h"

/**
 * 
 */
UCLASS()
class ORCAVSRVO_API UAgentManager : public UGameInstance
{
	GENERATED_BODY()

public:

	//constructor
	UAgentManager();

	// Function to register agents that implement IAgentInterface
	void RegisterAgent(const AORCAvsRVOCharacter* agent);
	

	TArray<AORCAvsRVOCharacter*> GetAllAgents() { return m_Agents; }

	UFUNCTION(BlueprintCallable, Category = "Singleton")
	static UAgentManager* GetInstance();



private:

	
	//static UAgentManager* Instance;

	// Array to store references to agents
	TArray<AORCAvsRVOCharacter*> m_Agents;
};
