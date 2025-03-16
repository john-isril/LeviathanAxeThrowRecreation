// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RandomizedMontage.generated.h"

USTRUCT(BlueprintType)
struct FRandomizedMontageInfo
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Montage"))
	TObjectPtr<UAnimMontage> m_Montage{ nullptr };

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Section Names"))
	TArray<FName> m_SectionNames{};

	FName GetRandomSection() const
	{
		if (!ensureAlwaysMsgf(m_SectionNames.Num(), TEXT("Section Names is empty!"))) return FName{};

		const int32 SectionNameIdx{ FMath::RandRange(0, m_SectionNames.Num() - 1) };

		return m_SectionNames[SectionNameIdx];
	}
};
