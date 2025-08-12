#include "PlanetView.hpp"

#include "cg/NodeManager.hpp"
#include "cg/Preloader.hpp"
#include "cg/Utility.hpp"
#include "data/PlanetData.hpp"
#include "data/ResourceData.hpp"
#include "data/Structure.hpp"
#include "Fleet.hpp"
#include "GameData.hpp"
#include "ResourceStockpileItem.hpp"
#include "scene/gui/item_list.h"
#include "StructureItem.hpp"
#include "systems/Colonization.hpp"
#include "systems/Construction.hpp"

using namespace GC;

PlanetView::PlanetView() { set_base_instance(this); }

void PlanetView::_bind_methods() {
	ClassDB::bind_method(D_METHOD("GetName"), &PlanetView::GetName);
	ClassDB::bind_method(D_METHOD("GetResourceIcon", "index"), &PlanetView::GetResourceIcon);

	ClassDB::bind_method(D_METHOD("IsColonizing"), &PlanetView::IsColonizing);
	ClassDB::bind_method(D_METHOD("IsColonized"), &PlanetView::IsColonized);
	ClassDB::bind_method(D_METHOD("IsUncolonized"), &PlanetView::IsUncolonized);
	ClassDB::bind_method(D_METHOD("OnColonizePressed"), &PlanetView::OnColonizePressed);

	ClassDB::bind_method(D_METHOD("GetStructures"), &PlanetView::GetStructures);
	ClassDB::bind_method(D_METHOD("GetResourceStockpile"), &PlanetView::GetResourceStockpile);

	ClassDB::bind_method(D_METHOD("GetPlanetTexture"), &PlanetView::GetPlanetTexture);
	ClassDB::bind_method(D_METHOD("HasBuildingQueue"), &PlanetView::HasBuildingQueue);
	ClassDB::bind_method(D_METHOD("ColonizeDisabled"), &PlanetView::ColonizeDisabled);
	ClassDB::bind_method(D_METHOD("GetColonizeTooltip"), &PlanetView::GetColonizeTooltip);
	ClassDB::bind_method(D_METHOD("GetColonizationProgress"), &PlanetView::GetColonizationProgress);
}

void PlanetView::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			construction_queue = Object::cast_to<ItemList>(get_node(NodePath("%ConstructionQueue")));
			construction_queue->connect("item_selected", callable_mp(this, &PlanetView::on_construction_queue_item_selected));
			player = &Fleet::self->get_player();
			set_process(true);
		} break;
		case NOTIFICATION_PROCESS: {
			update_building_queue();
			update_player_position();
		} break;
	}
}

void PlanetView::update_player_position() {
	if (!IsUncolonized())
		return;

	distance_from_player = -1.0;
	if (player->location == data->location)
		distance_from_player = player->position.distance_to(data->position) * MATTER_TRANSFER_RANGE_MULTIPLIER;
}

void PlanetView::update_building_queue() {
	if (!HasBuildingQueue())
		return;

	construction_queue->clear();
	// Rebuild building queue items
	const List<QueuedConstruction> &active_constructions = Construction::self->get_queued_construction(data->entity);
	Array tt_array;
	tt_array.resize(2);
	for (int i = 0; i < active_constructions.size(); ++i) {
		const QueuedConstruction &construction = active_constructions.get(i);
		const StructureData &structure_data = StructureDB::self->get(construction.entity);
		construction_queue->call_deferred("add_icon_item", Preloader::self->get_resource(structure_data.name));

		tt_array[0] = tr(structure_data.name);
		tt_array[1] = tr(itos(100 - Construction::self->get_progress(data->entity, i)));
		construction_queue->call_deferred("set_item_tooltip", i, tr("CONSTRUCTION_QUEUE_TOOLTIP").format(tt_array));
	}
}

void PlanetView::on_construction_queue_item_selected(int p_index) {
	Construction::self->remove_from_queue(data->entity, p_index);
	construction_queue->remove_item(p_index);
}

float PlanetView::GetColonizationProgress() { return Colonization::self->get_progress(data->entity); }

void PlanetView::OnColonizePressed() { Colonization::self->add(data->entity); }

String PlanetView::GetName() { return tr(data->name); }

Ref<Texture2D> PlanetView::GetResourceIcon(uint8_t p_index) {
	const ResourceData &resource_data = ResourceDB::self->get(data->resources[p_index]);
	return Preloader::self->get_resource(resource_data.name);
}

bool PlanetView::ColonizeDisabled() const {
	if (data->location != player->location)
		return true;

	if (distance_from_player > GameData::self->matter_transfer_range)
		return true;

	for (const KeyValue<ResourceEntity, uint16_t> &resource_cost : Colonization::self->resource_cost)
		if (player->resource_stockpile[resource_cost.key] < resource_cost.value)
			return true;

	return false;
}

String PlanetView::GetColonizeTooltip() {
	String tooltip = tr("COLONIZE") + data->name;
	Array arr;
	arr.resize(3);

	if (data->location != player->location)
		return vformat("Ship not in same system as %s.", data->name);

	arr[0] = data->name;
	arr[1] = GameData::self->matter_transfer_range;
	arr[2] = int(distance_from_player);
	if (distance_from_player > GameData::self->matter_transfer_range)
		tooltip += tr("COLONIZE_PLANET_DISTANCE_ALLOW_FAIL_TOOLTIP").format(arr);
	else
		tooltip += tr("COLONIZE_PLANET_DISTANCE_ALLOW_PASS_TOOLTIP").format(arr);

	for (const KeyValue<ResourceEntity, uint16_t> &resource_cost : Colonization::self->resource_cost) {
		const ResourceData &resource_data = ResourceDB::self->get(resource_cost.key);
		const uint16_t current_value = player->resource_stockpile[resource_cost.key];
		arr[0] = resource_cost.value;
		arr[1] = tr(resource_data.name);
		arr[2] = current_value;

		if (current_value < resource_cost.value) {
			tooltip += tr("COLONIZE_PLANET_RESOURCE_ALLOW_FAIL_TOOLTIP").format(arr);
			continue;
		}
		// good
		tooltip += tr("COLONIZE_PLANET_RESOURCE_ALLOW_PASS_TOOLTIP").format(arr);
	}

	return tooltip;
}

bool PlanetView::IsColonizing() { return data->colony_state == COLONIZING ? true : false; }

bool PlanetView::IsColonized() { return data->colony_state == COLONIZED ? true : false; }

bool PlanetView::IsUncolonized() { return data->colony_state == UNCOLONIZED ? true : false; }

Array PlanetView::GetStructures() {
	Array structure_nodes;
	const int size = StructureDB::self->size();
	structure_nodes.resize(size);
	structure_items.resize(size);

	for (int i = 0; i < size; ++i) {
		StructureItem *item = InitDataBind(StructureItem, "res://scenes/gui/shared/structure_item.tscn");
		structure_nodes[i] = item->get_parent();
		structure_items.set(i, item);
	}

	return structure_nodes;
}

Array PlanetView::GetResourceStockpile() {
	Array resource_nodes;
	int size = ResourceDB::self->size();
	resource_nodes.resize(size);
	resource_stockpile_items.resize(size);

	for (int i = 0; i < size; ++i) {
		ResourceStockpileItem *item = InitDataBind(ResourceStockpileItem, "res://scenes/gui/shared/resource_stockpile_item.tscn");
		resource_nodes[i] = item->get_parent();
		resource_stockpile_items.set(i, item);
	}

	return resource_nodes;
}

bool PlanetView::HasBuildingQueue() { return Construction::self->planet_has_construction(data->entity); }

Ref<ImageTexture> PlanetView::GetPlanetTexture() { return texture; }

void PlanetView::OpenPlanetView(PlanetEntity p_entity) {
	data = &PlanetDB::self->access(p_entity);

	NM::root->add_child(get_parent());

	// Update datamodel pointers
	for (int i = 0; i < structure_items.size(); ++i) {
		StructureItem *item = structure_items[i];
		item->set_structure_item(i, data);
	}
	for (int i = 0; i < resource_stockpile_items.size(); ++i) {
		ResourceStockpileItem *item = resource_stockpile_items[i];
		item->set_resource_item(i, ResourceStockpileItem::PLANET, data->entity);
	}
}
