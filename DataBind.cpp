#include "godot_cpp/classes/control.hpp"
#include "godot_cpp/classes/expression.hpp"
#include "godot_cpp/classes/base_button.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include "DataBind.hpp"

using namespace GUI;

Expression *DataBind::get_expression(const String &expression_string) {
	Expression *expression = memnew(Expression());
	Error err = expression->parse(expression_string);
	if (err != OK)
		UtilityFunctions::printerr(expression->get_error_text());

	return expression;
}

void DataBind::execute_expression(Expression *expr, Control *node, const StringName &method, Variant::Type expected_type, const String &expected_class) {
	// If the method to call doesn't exist there is no reason to even execute the expression
	if (!node->has_method(method)) {
		UtilityFunctions::printerr("Executing ", method, " expression for ", node->get_path(), " failed: ", method, " does not exist.");
		return;
	}

	Variant result = expr->execute(dummy_input_array, base_instance);

	// Bail if execution fails, parse() will also report this in get_expression.
	if (expr->has_execute_failed()) {
		UtilityFunctions::printerr("Executing ", method, " expression for ", node->get_path(), " failed: ", expr->get_error_text());
		return;
	}

	// If variant types don't match return
	if (result.get_type() != expected_type) {
		UtilityFunctions::printerr("Executing ", method, " expression for ", node->get_path(), " failed: Result type is ", Variant::get_type_name(result.get_type()),
				" expected: ", Variant::get_type_name(expected_type));
		return;
	}

	// If type is Object also verify that the class is correct.
	if (result.get_type() == Variant::OBJECT) {
		Object *obj = Object::cast_to<Object>(result); // need to cast to call is_class
		if (!obj->is_class(expected_class)) {
			UtilityFunctions::printerr(
					"Executing ", method, " expression for ", node->get_path(), " failed: Result class is ", Variant::get_type_name(result.get_type()), " expected: ", expected_class);
			return;
		}
	}

	// Call the godot method with the result of the expression
	// For example if the metadata is 'visible', this will call the set_visible method.
	node->call(method, result);
}

void DataBind::setup_pressed(Control *node) {
	if (node->has_meta("pressed")) {
		const String &pressed_method = node->get_meta("pressed");
		Callable pressed_callable = Callable(base_instance, pressed_method);

		if (!pressed_callable.is_valid()) {
			String error_reason = "";
			if (pressed_method.ends_with("()"))
				error_reason = " Callables do not need '()' at the end.";
			UtilityFunctions::printerr("Callable '", pressed_method, "' assigned to 'pressed' signal for ", node->get_path(), " is not valid.", error_reason);
			return;
		}
		Object::cast_to<BaseButton>(node)->connect("pressed", pressed_callable);
	}
}

void DataBind::_notification(int p_what) {
	if (p_what == NOTIFICATION_READY) {
		TypedArray<Control> meta_nodes = find_children("*", "Control");
		for (int i = 0; i < meta_nodes.size(); ++i) {
			Control *node = Object::cast_to<Control>(meta_nodes[i]);
			HashMap<String, Expression *> properties;

			if (node->has_meta("visible"))
				properties["visible"] = get_expression(node->get_meta("visible"));

			if (node->has_meta("disabled"))
				properties["disabled"] = get_expression(node->get_meta("disabled"));

			if (node->has_meta("text"))
				properties["text"] = get_expression(node->get_meta("text"));

			if (node->has_meta("texture"))
				properties["texture"] = get_expression(node->get_meta("texture"));

			setup_pressed(node);

			if (properties.size() > 0)
				node_properties[node] = properties;
		}
	}
}

void DataBind::update() {
	for (const auto &properties : node_properties) {
		if (properties.value.has("visible"))
			execute_expression(properties.value["visible"], properties.key, "set_visible", Variant::BOOL);

		if (properties.value.has("disabled"))
			execute_expression(properties.value["disabled"], properties.key, "set_disabled", Variant::BOOL);

		if (properties.value.has("text"))
			execute_expression(properties.value["text"], properties.key, "set_text", Variant::STRING);

		if (properties.value.has("texture"))
			execute_expression(properties.value["texture"], properties.key, "set_texture", Variant::OBJECT, "Resource"); // Texture2D?
	}
}

void DataBind::set_base_instance(Object *p_object) { base_instance = p_object; }

void DataBind::_bind_methods() {
	ClassDB::bind_method(D_METHOD("update"), &DataBind::update);
	ClassDB::bind_method(D_METHOD("set_base_instance"), &DataBind::set_base_instance);
}
