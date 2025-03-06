#pragma once

#include "godot_cpp/classes/button.hpp"
#include "godot_cpp/classes/label.hpp"
#include "godot_cpp/classes/texture_rect.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include "DataBind.hpp"

using namespace godot;

namespace GUI {

// This is the "Model"
struct Data {
	String name = "hello?";
	bool has_something = false;
};

// This is the "Controller"
// DataBindExample to set as the root node of your GUI scenes
class DataBindExample : public DataBind {
	GDCLASS(DataBindExample, DataBind)

private:
	Data data;

protected:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("GetName"), &DataBindExample::GetName);
		ClassDB::bind_method(D_METHOD("HasSomething"), &DataBindExample::HasSomething);
		ClassDB::bind_static_method("DataBindExample", D_METHOD("OnButtonClick"), &DataBindExample::OnButtonClick);
	}

public:
	DataBindExample() {
		set_base_instance(this); // must set base_instance in constructor so the object can be passed into the executed Expression, this allows any bound methods in this class to be called from the expression.
	}

	String GetName() const {
		return data.name;
	}

	bool HasSomething() const {
		return data.has_something;
	}

	void OnButtonClick() {
		UtilityFunctions::print("Button clicked!!!");
		data.name = "hello!";
	}
};

// This is the "View"
// some node in the scene that DataBindExample is root of to show example of setting metadata Expressions that the DataBind class will call.
class DataBindLabelExample : public Label {
	GDCLASS(DataBindLabelExample, Label)

protected:
	static void _bind_methods() {}

public:
	DataBindLabelExample() {
		// The DataBind base class will check for this visible metadata and call HasSomething() as an Expression and use the result in Control::set_visible() whenever the update() function of the DataBind is called.
		// This metadata would normally be set in the editor. The DataBind class iterates over all controls with certain metadata that are children of DataBindExample() in the scene it is found in and they will all have set_visible() called.
		set_meta("visible", "HasSomething() and 1 > 0");
		set_meta("text", "GetName()");

		Button *button = memnew(Button());
		add_child(button);

		// Setting the 'pressed' metadata on a button will automatically connect the signal to the method in the DataBind
		// Note that this ends up being a Callable NOT an Expression so parentheses are not needed.
		button->set_meta("pressed", "OnButtonClick");
		button->set_meta("disabled", "HasSomething()");
	}
};

} // namespace GUI
