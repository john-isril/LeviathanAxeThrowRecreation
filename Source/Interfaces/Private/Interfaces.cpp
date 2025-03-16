#include "Interfaces.h"

DEFINE_LOG_CATEGORY(Interfaces);

#define LOCTEXT_NAMESPACE "FInterfaces"

void FInterfaces::StartupModule()
{
	UE_LOG(Interfaces, Warning, TEXT("Interfaces module has been loaded"));
}

void FInterfaces::ShutdownModule()
{
	UE_LOG(Interfaces, Warning, TEXT("Interfaces module has been unloaded"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FInterfaces, Interfaces)