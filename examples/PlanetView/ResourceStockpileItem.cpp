#include "ResourceStockpileItem.hpp"

#include "cg/NodeManager.hpp"
#include "cg/Preloader.hpp"
#include "data/PlanetData.hpp"
#include "data/ResourceData.hpp"
#include "Fleet.hpp"
#include "gui/TransferResourcePopup.hpp"

using namespace GC;

ResourceStockpileItem::ResourceStockpileItem() { set_base_instance(this); }

String ResourceStockpileItem::GetTooltip() {
	if (type == PLANET)
		return vformat("%s has %d %s in stockpile.", planet_data->name, GetResourceCount(), data->name);
	else
		return vformat("%s has %d %s in stockpile.", ship_data->name, GetResourceCount(), data->name);
}

int ResourceStockpileItem::GetResourceCount() {
	if (type == PLANET)
		return planet_data->get_resource_count(data->entity);
	else
		return ship_data->get_resource_count(data->entity);
}

Ref<Texture2D> ResourceStockpileItem::GetResourceIcon() { return Preloader::self->get_resource(data->name); }

void ResourceStockpileItem::ShowTransferResourcePopup() {
	// Open popup that lets you transfer resources to nearby planets
	if (type == PLANET)
		NM::transfer_resource_popup->OpenTransferResourcePopup(data->entity, TransferResourcePopup::PLANET, planet_data->entity);
	else
		NM::transfer_resource_popup->OpenTransferResourcePopup(data->entity, TransferResourcePopup::SHIP, ship_data->entity);
}

void ResourceStockpileItem::set_resource_item(ResourceEntity p_entity, Type p_type, Entity p_target_entity) {
	data = &ResourceDB::self->get(p_entity);
	type = p_type;
	if (type == PLANET)
		planet_data = &PlanetDB::self->get(p_target_entity);
	else
		ship_data = &Fleet::self->get(p_target_entity);
}

void ResourceStockpileItem::_bind_methods() {
	ClassDB::bind_method(D_METHOD("GetTooltip"), &ResourceStockpileItem::GetTooltip);
	ClassDB::bind_method(D_METHOD("GetResourceCount"), &ResourceStockpileItem::GetResourceCount);
	ClassDB::bind_method(D_METHOD("ShowTransferResourcePopup"), &ResourceStockpileItem::ShowTransferResourcePopup);
	ClassDB::bind_method(D_METHOD("GetResourceIcon"), &ResourceStockpileItem::GetResourceIcon);
}
