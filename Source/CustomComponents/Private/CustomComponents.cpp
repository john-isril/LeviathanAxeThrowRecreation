#include "CustomComponents.h"

DEFINE_LOG_CATEGORY(CustomComponents);

#define LOCTEXT_NAMESPACE "FCustomComponents"

void FCustomComponents::StartupModule()
{
	UE_LOG(CustomComponents, Warning, TEXT("CustomComponents module has been loaded"));
}

void FCustomComponents::ShutdownModule()
{
	UE_LOG(CustomComponents, Warning, TEXT("CustomComponents module has been unloaded"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCustomComponents, CustomComponents)