#pragma once

struct SecurityHelper final {
	static bool EnablePrivilege(PCWSTR privName, bool enable);
};