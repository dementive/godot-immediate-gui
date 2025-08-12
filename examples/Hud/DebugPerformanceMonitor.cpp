#include "DebugPerformanceMonitor.hpp"

#include "main/performance.h"

using namespace CG;

DebugPerformanceMonitor::DebugPerformanceMonitor() { set_base_instance(this); }

String DebugPerformanceMonitor::GetValue(int p_monitor) { return itos(Performance::get_singleton()->get_monitor(static_cast<Performance::Monitor>(p_monitor))); }

void DebugPerformanceMonitor::_bind_methods() { ClassDB::bind_static_method("DebugPerformanceMonitor", D_METHOD("GetValue"), &DebugPerformanceMonitor::GetValue); }
