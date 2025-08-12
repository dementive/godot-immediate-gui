#pragma once

#include "cg/DataBind.hpp"
#include "core/input/input_event.h"
#include "defs/types.hpp"
#include "scene/resources/texture.h"

class ItemList;

namespace GC {

struct ResourceData;
using PlanetEntity = Entity;
using SolarSystemEntity = Entity;
using ResourceEntity = Entity;

class TransferResourcePopup : public DataBind {
	GDCLASS(TransferResourcePopup, DataBind)

public:
	enum Type : uint8_t { SHIP, PLANET };

private:
	const ResourceData *data{};
	PackedInt32Array *from_stockpile{};
	String transfer_from;
	ItemList *planet_selection_list{};
	ItemList *ship_selection_list{};
	Vector3 origin;
	SolarSystemEntity transfer_system{};
	int transfer_amount{};
	Type transfer_type = SHIP;
	Entity target_entity{}; // store planet transfering from if transfer_type is not SHIP.

	int get_total();
	int get_total_selected_items();

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	TransferResourcePopup();
	void unhandled_key_input(const Ref<InputEvent> &p_event) final;
	void update_item_list();
	void create_planet_item_list();
	void create_ship_item_list();
	void Add();
	bool AddDisabled() const;
	bool SubtractDisabled() const;
	String GetAddTooltip();
	String GetSubtractTooltip();
	void Subtract();
	void reset();
	bool AcceptDisabled();
	void Accept();
	String GetAcceptTooltip();
	String GetTransferAmount() const;
	String GetTitle();
	Ref<Texture2D> GetResourceIcon();
	void OpenTransferResourcePopup(ResourceEntity p_entity, Type p_transfer_type, Entity p_target_entity);
};

} // namespace GC