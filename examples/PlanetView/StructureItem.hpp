#pragma once

#include "cg/DataBind.hpp"
#include "scene/resources/texture.h"

namespace GC {

struct StructureData;
struct PlanetData;
using StructureEntity = uint32_t;

class StructureItem : public DataBind {
	GDCLASS(StructureItem, DataBind)

private:
	const StructureData *data{};
	PlanetData *planet_data{};

protected:
	static void _bind_methods();

public:
	StructureItem();
	float GetConstructionProgress();
	String GetTooltip();
	int GetStructureLevel();
	bool BuildStructureDisabled();
	void BuildStructure();
	Ref<Texture2D> GetStructureIcon();
	void set_structure_item(StructureEntity p_entity, PlanetData *p_planet_data);
};

} // namespace GC
