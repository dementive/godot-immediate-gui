#include "gui/TransferResourcePopup.hpp"

#include "cg/NodeManager.hpp"
#include "cg/Preloader.hpp"
#include "data/PlanetData.hpp"
#include "data/ResourceData.hpp"
#include "data/SolarSystemData.hpp"
#include "Fleet.hpp"
#include "GameData.hpp"
#include "scene/gui/item_list.h"
#include "scene/main/viewport.h"

using namespace GC;

TransferResourcePopup::TransferResourcePopup() { set_base_instance(this); }

int TransferResourcePopup::get_total_selected_items() { return planet_selection_list->get_selected_items().size() + ship_selection_list->get_selected_items().size(); }
int TransferResourcePopup::get_total() { return transfer_amount * MAX(1, get_total_selected_items()); }

void TransferResourcePopup::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			planet_selection_list = Object::cast_to<ItemList>(get_node(NodePath("%PlanetItemList")));
			ship_selection_list = Object::cast_to<ItemList>(get_node(NodePath("%ShipItemList")));
			set_process(true);
			set_process_unhandled_key_input(true);
		} break;
		case NOTIFICATION_PROCESS: {
			update_item_list();
		} break;
	}
}

void TransferResourcePopup::unhandled_key_input(const Ref<InputEvent> &p_event) {
	if (Input::get_singleton()->is_key_pressed(Key::ESCAPE)) {
		get_viewport()->set_input_as_handled();
		reset();
	}
}

void TransferResourcePopup::update_item_list() {
	// Update ship origin
	if (transfer_type == SHIP) {
		const ShipData &ship_data = Fleet::self->get(target_entity);
		origin = ship_data.position;
	}

	// Update disabled and tooltip properties for each planet item.
	for (int i = 0; i < planet_selection_list->get_item_count(); ++i) {
		const PlanetData &location_data = PlanetDB::self->get(planet_selection_list->get_item_metadata(i));
		const int distance = origin.distance_to(location_data.position) * MATTER_TRANSFER_RANGE_MULTIPLIER;
		const bool planet_in_range = distance <= GameData::self->matter_transfer_range;

		String tooltip = vformat("%s is colonized and within %dkm (current: %d) of %s.\nShift+click multiple items to transfer to more than one location at a time.", location_data.name,
				GameData::self->matter_transfer_range, distance, transfer_from);

		if (!planet_in_range or location_data.colony_state != COLONIZED) {
			planet_selection_list->call_deferred("set_item_disabled", i, true);
			tooltip = "X " + tooltip;
		} else {
			tooltip = "+ " + tooltip;
			planet_selection_list->call_deferred("set_item_disabled", i, false);
		}

		planet_selection_list->call_deferred("set_item_tooltip", i, tooltip);
	}

	// Update disabled and tooltip properties for each ship item.
	for (int i = 0; i < ship_selection_list->get_item_count(); ++i) {
		const ShipData &ship_data = Fleet::self->get(ship_selection_list->get_item_metadata(i));
		const int distance = origin.distance_to(ship_data.position) * MATTER_TRANSFER_RANGE_MULTIPLIER;
		String tooltip = vformat("%s is within %dkm (current: %d) of %s.\nShift+click multiple items to transfer to more than one location at a time.", tr(ship_data.name),
				GameData::self->matter_transfer_range, distance, transfer_from);

		if (distance > GameData::self->matter_transfer_range) {
			planet_selection_list->call_deferred("set_item_disabled", i, true);
			tooltip = "X " + tooltip;
		} else {
			planet_selection_list->call_deferred("set_item_disabled", i, false);
			tooltip = "+ " + tooltip;
		}

		planet_selection_list->call_deferred("set_item_tooltip", i, tooltip);
	}
}

void TransferResourcePopup::create_ship_item_list() {
	ship_selection_list->clear();
	PackedInt32Array locations = SolarSystemDB::self->get(transfer_system).planets;
	// TODO
}

void TransferResourcePopup::create_planet_item_list() {
	planet_selection_list->clear();
	PackedInt32Array locations = SolarSystemDB::self->get(transfer_system).planets;

	for (int i = 0; i < locations.size(); ++i) {
		const int32_t location = locations[i];

		if (transfer_type == PLANET and int32_t(target_entity) == location) {
			// Planet shoulnd't include itself in list.
			continue;
		}

		const PlanetData &location_data = PlanetDB::self->get(location);
		const int distance = origin.distance_to(location_data.position) * MATTER_TRANSFER_RANGE_MULTIPLIER;
		const bool planet_in_range = distance <= GameData::self->matter_transfer_range;
		if (transfer_type == PLANET and !planet_in_range) {
			// if transfering from planet to planet don't make an item if the planet is out of range as planets do not move and will never be in range.
			// Ship can just move to be in range so in that case it's better to show all planets in the system.
			continue;
		}

		planet_selection_list->add_item(location_data.name);
		planet_selection_list->set_item_metadata(i, location);
	}
}

void TransferResourcePopup::Add() {
	if (Input::get_singleton()->is_key_pressed(Key::SHIFT))
		transfer_amount = MIN(transfer_amount + 100, from_stockpile->get(data->entity));
	else
		transfer_amount = MIN(transfer_amount + 10, from_stockpile->get(data->entity));
}

bool TransferResourcePopup::AddDisabled() const {
	if (transfer_amount + 10 > from_stockpile->get(data->entity))
		return true;

	return false;
}

bool TransferResourcePopup::SubtractDisabled() const {
	if (transfer_amount - 10 < 0)
		return true;

	return false;
}

String TransferResourcePopup::GetAddTooltip() { return vformat("Click to add 10 %s to the transfer amount.\nShift click to add 100.", data->name); }
String TransferResourcePopup::GetSubtractTooltip() { return vformat("Click to subtract 10 %s to the transfer amount.\nShift click to subtract 100.", data->name); }

void TransferResourcePopup::Subtract() {
	if (Input::get_singleton()->is_key_pressed(Key::SHIFT))
		transfer_amount = MAX(transfer_amount - 100, 0);
	else
		transfer_amount = MAX(transfer_amount - 10, 0);
}

void TransferResourcePopup::reset() {
	NM::root->remove_child(get_parent());
	from_stockpile = nullptr;
	data = nullptr;
	transfer_from = "";
	transfer_amount = 0;
}

bool TransferResourcePopup::AcceptDisabled() {
	if (get_total() > from_stockpile->get(data->entity))
		return true;

	if (get_total_selected_items() == 0 or transfer_amount == 0)
		return true;

	return false;
}

void TransferResourcePopup::Accept() {
	// Subtract total from stockpile
	from_stockpile->set(data->entity, from_stockpile->get(data->entity) - get_total());

	// Add transfer_amount to every selected planet entity
	for (const int32_t selected_index : planet_selection_list->get_selected_items()) {
		const PlanetEntity entity = planet_selection_list->get_item_metadata(selected_index);
		PlanetDB::self->access(entity).resource_stockpile.write[data->entity] += transfer_amount;
	}

	// Add transfer_amount to every selected ship entity
	for (const int32_t selected_index : ship_selection_list->get_selected_items()) {
		const ShipEntity entity = ship_selection_list->get_item_metadata(selected_index);
		Fleet::self->access(entity).resource_stockpile.write[data->entity] += transfer_amount;
	}

	reset();
}

String TransferResourcePopup::GetAcceptTooltip() {
	// Show proper message if disabled
	if (AcceptDisabled()) {
		if (get_total_selected_items() == 0)
			return vformat("Must select at least one location to transfer %s to.", data->name);
		else if (transfer_amount == 0)
			return vformat("Must transfer more than 0 %s.", data->name);
		return vformat("Total transfer amount: %d is less than stockpiled %s on %s (%d).", get_total(), data->name, transfer_from, from_stockpile->get(data->entity));
	}

	// Update tooltip text with each entity being transferred to.
	String tooltip;
	for (const int32_t selected_index : planet_selection_list->get_selected_items()) {
		const int entity = planet_selection_list->get_item_metadata(selected_index);
		const String &name = tr(PlanetDB::self->get(entity).name);
		tooltip += vformat("Transfer %s %s to %s.\n", transfer_amount, data->name, name);
	}

	for (const int32_t selected_index : ship_selection_list->get_selected_items()) {
		const int entity = ship_selection_list->get_item_metadata(selected_index);
		const String &name = tr(Fleet::self->get(entity).name);
		tooltip += vformat("Transfer %s %s to %s.\n", transfer_amount, data->name, name);
	}

	return tooltip;
}

String TransferResourcePopup::GetTransferAmount() const { return itos(transfer_amount); }

String TransferResourcePopup::GetTitle() { return vformat("Transfer %s from %s", data->name, transfer_from); }

Ref<Texture2D> TransferResourcePopup::GetResourceIcon() { return Preloader::self->get_resource(data->name); }

void TransferResourcePopup::OpenTransferResourcePopup(ResourceEntity p_entity, Type p_transfer_type, Entity p_target_entity) {
	data = &ResourceDB::self->get(p_entity);
	transfer_type = p_transfer_type;
	target_entity = p_target_entity;

	if (transfer_type == SHIP) {
		ShipData &ship_data = Fleet::self->access(target_entity);
		from_stockpile = &ship_data.resource_stockpile;
		transfer_from = ship_data.name;
		origin = ship_data.position;
		transfer_system = ship_data.location;
	} else {
		PlanetData &planet_data = PlanetDB::self->access(target_entity);
		from_stockpile = &planet_data.resource_stockpile;
		transfer_from = planet_data.name;
		origin = planet_data.position;
		transfer_system = planet_data.location;
	}

	if (!is_inside_tree())
		NM::root->add_child(get_parent());

	create_planet_item_list();
	create_ship_item_list();
}

void TransferResourcePopup::_bind_methods() {
	ClassDB::bind_method(D_METHOD("reset"), &TransferResourcePopup::reset);
	ClassDB::bind_method(D_METHOD("AcceptDisabled"), &TransferResourcePopup::AcceptDisabled);
	ClassDB::bind_method(D_METHOD("Accept"), &TransferResourcePopup::Accept);
	ClassDB::bind_method(D_METHOD("GetAcceptTooltip"), &TransferResourcePopup::GetAcceptTooltip);
	ClassDB::bind_method(D_METHOD("GetTransferAmount"), &TransferResourcePopup::GetTransferAmount);
	ClassDB::bind_method(D_METHOD("GetTitle"), &TransferResourcePopup::GetTitle);
	ClassDB::bind_method(D_METHOD("GetResourceIcon"), &TransferResourcePopup::GetResourceIcon);

	ClassDB::bind_method(D_METHOD("Add"), &TransferResourcePopup::Add);
	ClassDB::bind_method(D_METHOD("Subtract"), &TransferResourcePopup::Subtract);
	ClassDB::bind_method(D_METHOD("GetAddTooltip"), &TransferResourcePopup::GetAddTooltip);
	ClassDB::bind_method(D_METHOD("GetSubtractTooltip"), &TransferResourcePopup::GetSubtractTooltip);
	ClassDB::bind_method(D_METHOD("AddDisabled"), &TransferResourcePopup::AddDisabled);
	ClassDB::bind_method(D_METHOD("SubtractDisabled"), &TransferResourcePopup::SubtractDisabled);
}
