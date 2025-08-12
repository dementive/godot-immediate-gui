#pragma once

#include "cg/DataBind.hpp"

namespace CG {
class DebugPerformanceMonitor : public DataBind {
	GDCLASS(DebugPerformanceMonitor, DataBind)

protected:
	static void _bind_methods();

public:
	DebugPerformanceMonitor();
	static String GetValue(int p_monitor);
};

} // namespace CG
