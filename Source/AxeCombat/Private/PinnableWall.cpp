// Fill out your copyright notice in the Description page of Project Settings.


#include "PinnableWall.h"

APinnableWall::APinnableWall()
{
	PrimaryActorTick.bCanEverTick = false;

	m_Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(m_Mesh);
	
	m_Mesh->SetCollisionProfileName(FName{ "BlockAllDynamic" });
	m_Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

