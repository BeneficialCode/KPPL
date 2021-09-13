#pragma once

class SysManager {
public:
	SysManager(std::wstring sysPath, std::wstring serviceName, std::wstring displayName,bool isFileSys) :_sysPath(sysPath),
		_serviceName(serviceName), _displayName(displayName),_isFileSys(isFileSys){
	}
	bool Install() const;
	bool Run() const;
	bool Stop() const;
	bool Remove() const;
	SC_HANDLE GetServiceHandle() const;
private:
	std::wstring _sysPath;
	std::wstring _serviceName;
	std::wstring _displayName;
	bool _isFileSys;
};