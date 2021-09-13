#pragma once

struct HandleInfo {
	PVOID Object;
	ULONG ProcessId;
	ULONG HandleValue;
	ULONG GrantedAccess;
	ULONG HandleAttributes;
	USHORT ObjectTypeIndex;
};

class ObjectManager {
public:
	bool EnumHandles(DWORD pid = 0);
	const std::vector<std::shared_ptr<HandleInfo>>& GetHandles() const;
private:
	std::vector<std::shared_ptr<HandleInfo>> _handles;
};