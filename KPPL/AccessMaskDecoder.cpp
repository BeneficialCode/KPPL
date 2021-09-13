#include "pch.h"
#include "AccessMaskDecoder.h"

std::unordered_map<std::wstring, std::vector<AccessMaskDecoder::AccessMaskPair>> AccessMaskDecoder::Tables = {
	{ L"Process", {
		{ PROCESS_ALL_ACCESS, L"PROCESS_ALL_ACCESS", true },
		{ PROCESS_CREATE_THREAD            , L"CREATE_THREAD" },
		{ PROCESS_SET_SESSIONID            , L"SET_SESSIONID" },
		{ PROCESS_VM_OPERATION             , L"VM_OPERATION" },
		{ PROCESS_VM_READ                  , L"VM_READ" },
		{ PROCESS_VM_WRITE                 , L"VM_WRITE" },
		{ PROCESS_DUP_HANDLE               , L"DUP_HANDLE" },
		{ PROCESS_CREATE_PROCESS           , L"CREATE_PROCESS" },
		{ PROCESS_SET_QUOTA                , L"SET_QUOTA" },
		{ PROCESS_SET_INFORMATION          , L"SET_INFORMATION" },
		{ PROCESS_QUERY_INFORMATION        , L"QUERY_INFORMATION" },
		{ PROCESS_SUSPEND_RESUME           , L"SUSPEND_RESUME" },
		{ PROCESS_QUERY_LIMITED_INFORMATION, L"QUERY_LIMITED_INFORMATION" },
		{ PROCESS_SET_LIMITED_INFORMATION  , L"SET_LIMITED_INFORMATION" },
		},
	},
};

std::wstring AccessMaskDecoder::DecodeAccessMask(PCWSTR typeName, ACCESS_MASK access) {
	bool all = false;
	std::wstring result;
	if (access & 0xffff) {
		auto it = Tables.find(typeName);
		if (it != Tables.end()) {
			for (auto& pair : it->second) {
				if ((pair.AccessMask & access) == pair.AccessMask) {
					result += pair.Decoded + std::wstring(L" | ");
					if (pair.All) {
						all = true;
						break;
					}
				}
			}
		}
		else {
			result = L"<unknown> | ";
		}
	}

	// add generic access mask
	static AccessMaskPair generic[] = {
		{ ACCESS_SYSTEM_SECURITY, L"ACCESS_SYSTEM_SECURITY" },
		{ STANDARD_RIGHTS_ALL, L"STANDARD_RIGHTS_ALL", true },
		{ SYNCHRONIZE, L"SYNCHRONIZE" },
		{ WRITE_DAC, L"WRITE_DAC" },
		{ READ_CONTROL, L"READ_CONTROL" },
		{ WRITE_OWNER, L"WRITE_OWNER" },
		{ DELETE, L"DELETE" },
	};

	if (!all) {
		for (auto& pair : generic) {
			if ((pair.AccessMask & access) == pair.AccessMask) {
				result += pair.Decoded + std::wstring(L" | ");
				if (pair.All)
					break;
			}
		}
	}

	if (!result.empty())
		result = result.substr(0, result.length() - 3);
	return result;
}