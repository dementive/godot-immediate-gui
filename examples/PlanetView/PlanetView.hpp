#pragma once

#include "cg/DataBind.hpp"
#include "scene/resources/image_texture.h"

class ItemList;

namespace GC {

using PlanetEntity = uint32_t;
struct PlanetData;
struct ShipData;
class StructureItem;
class ResourceStockpileItem;

class PlanetView : public DataBind {
	GDCLASS(PlanetView, DataBind)

private:
	PlanetData *data{};
	ItemList *construction_queue{};
	float distance_from_player = -1.0;
	const ShipData *player{};

	Ref<ImageTexture> texture;
	Vector<StructureItem *> structure_items;
	Vector<ResourceStockpileItem *> resource_stockpile_items;

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	PlanetView();
	void update_player_position();
	void update_building_queue();
	void on_construction_queue_item_selected(int p_index);
	float GetColonizationProgress();
	void OnColonizePressed();
	String GetName();
	Ref<Texture2D> GetResourceIcon(uint8_t p_index);
	bool ColonizeDisabled() const;
	String GetColonizeTooltip();
	bool IsColonizing();
	bool IsColonized();
	bool IsUncolonized();
	Array GetStructures();
	Array GetResourceStockpile();
	bool HasBuildingQueue();
	Ref<ImageTexture> GetPlanetTexture();
	void OpenPlanetView(PlanetEntity p_entity);
};

} // namespace GC
