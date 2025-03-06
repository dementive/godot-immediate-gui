#pragma once

#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/templates/hash_map.hpp"

using namespace godot;

namespace godot {
	class Expression;
	class Control;
}

namespace GUI {

class DataBind : public Node {
	GDCLASS(DataBind, Node)

private:
	HashMap<Control *, HashMap<String, Expression *>> node_properties;
	Object *base_instance{};
	Array dummy_input_array; // empty array must be passed into Expression::execute and it would be stupid to construct a new one every time.

	static Expression *get_expression(const String &expression_string);
	void execute_expression(Expression *expr, Control *node, const StringName &method, Variant::Type expected_type, const String &expected_class = "Object");
	void setup_pressed(Control *node);

protected:
	static void _bind_methods();

public:
	void set_base_instance(Object *p_object);
	void _notification(int p_what);
	void update();
};

} // namespace GUI
