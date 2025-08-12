#include "StructureItem.hpp"

#include "cg/Preloader.hpp"
#include "data/PlanetData.hpp"
#include "data/ResourceData.hpp"
#include "data/Structure.hpp"
#include "data/StructureLevel.hpp"
#include "systems/Construction.hpp"

using namespace GC;

StructureItem::StructureItem() { set_base_instance(this); }

float StructureItem::GetConstructionProgress() {
	if (Construction::self->is_constructing_structure(planet_data->entity, data->entity))
		return Construction::self->get_progress(planet_data->entity, 0);
	else
		return 0;
}

String StructureItem::GetTooltip() {
	String tooltip = "Upgrade " + tr(data->name);
	const uint8_t new_level = GetStructureLevel() + 1;

	if (new_level > MAX_STRUCTURE_LEVEL)
		return tr(data->name) + " already at max level.";

	const Vector<StructureAllow> &allow = StructureLevels::self->get_allow(data->entity, new_level);

	for (const StructureAllow &allow_node : allow) {
		const uint32_t stockpiled_value = planet_data->resource_stockpile[allow_node.entity];
		const String &resource_name = ResourceDB::self->get(allow_node.entity).name;
		const String text = vformat("\n%s cost: %d (currently: %d %s in stockpile on %s)", resource_name, allow_node.value, stockpiled_value, resource_name, planet_data->name);
		tooltip += text;
	}

	return tooltip;
}

int StructureItem::GetStructureLevel() { return Construction::self->get_structure_level_after_construction(planet_data->entity, data->entity); }

bool StructureItem::BuildStructureDisabled() {
	const uint8_t new_level = GetStructureLevel() + 1;

	if (new_level > MAX_STRUCTURE_LEVEL)
		return true;

	const Vector<StructureAllow> &allow = StructureLevels::self->get_allow(data->entity, new_level);
	for (const StructureAllow &allow_node : allow)
		if (planet_data->resource_stockpile[allow_node.entity] < allow_node.value)
			return true;

	return false;
}

void StructureItem::BuildStructure() {
	Construction::self->add(planet_data->entity, data->entity);
}

Ref<Texture2D> StructureItem::GetStructureIcon() { return Preloader::self->get_resource(data->name); }

void StructureItem::set_structure_item(StructureEntity p_entity, PlanetData *p_planet_data) {
	data = &StructureDB::self->get(p_entity);
	planet_data = p_planet_data;
}

void StructureItem::_bind_methods() {
	ClassDB::bind_method(D_METHOD("GetTooltip"), &StructureItem::GetTooltip);
	ClassDB::bind_method(D_METHOD("GetStructureIcon"), &StructureItem::GetStructureIcon);
	ClassDB::bind_method(D_METHOD("GetStructureLevel"), &StructureItem::GetStructureLevel);
	ClassDB::bind_method(D_METHOD("BuildStructure"), &StructureItem::BuildStructure);
	ClassDB::bind_method(D_METHOD("BuildStructureDisabled"), &StructureItem::BuildStructureDisabled);
	ClassDB::bind_method(D_METHOD("GetConstructionProgress"), &StructureItem::GetConstructionProgress);
}
