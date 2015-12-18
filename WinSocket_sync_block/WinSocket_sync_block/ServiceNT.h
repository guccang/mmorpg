#pragma once
namespace GUGGAME
{
	bool InstallService(const char* service_name, const char* service_desc, const char* command);
	bool UninstallService(const char* service_name);
}
