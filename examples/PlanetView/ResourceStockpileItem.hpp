#pragma once

#include "cg/DataBind.hpp"
#include "defs/types.hpp"
#include "scene/resources/texture.h"

namespace GC {

struct PlanetData;
struct ResourceData;
struct ShipData;
using ResourceEntity = Entity;

class ResourceStockpileItem : public DataBind {
	GDCLASS(ResourceStockpileItem, DataBind)

public:
	enum Type : uint8_t { SHIP, PLANET };

private:
	Type type{};
	const ResourceData *data{};
	union {
		const PlanetData *planet_data{};
		const ShipData *ship_data;
	};

protected:
	static void _bind_methods();

public:
	ResourceStockpileItem();
	String GetTooltip();
	int GetResourceCount();
	Ref<Texture2D> GetResourceIcon();
	void ShowTransferResourcePopup();
	void set_resource_item(ResourceEntity p_entity, Type p_type, Entity p_target_entity);
};

} // namespace GC
