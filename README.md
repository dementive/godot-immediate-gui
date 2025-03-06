# Godot cpp Data Bind

Godot 4.4 C++ GDExtension that adds a DataBind class that enables simple non-intrusive use of the MVC design pattern with as little boilerplate as possible for GUI scenes.

## Usage

The steps to use the DataBind class in your project are:

1. Make a custom class that Extends the DataBind class, this will be your controller.
2. Add methods to the controller that call out to your data to update the UI state.
3. Add the controller Node as the root node of a new GUI scene in the Godot editor (or with code) and add all the control nodes you'll need for your UI into it.
4. To bind methods from your controller class to your view all you have to do is set String metadata properties, that correlate to actual Control properties, in the editor with a value that is a valid godot [Expression](https://docs.godotengine.org/en/stable/tutorials/scripting/evaluating_expressions.html) with methods from controller. Since Expressions are used to execute the controller methods you can use boolean logic or even do math in the Expressions for maximum flexibility, so something like `X() && Y() || Z()` would work for controls with complex state.
5. Finally to actually update the UI call the DataBind::update() function on your controller, which you will have to call somewhere in your code whenever your model changes or when the view is opened to ensure the UI state is always in sync with your data. The DataBind will check all your controls in the scene for the metadata properties you set, execute the Expression, and then uses the result of the expression to call the appropriate method to update the UI state.

## Example

See DataBindExample.hpp and read the comments for basic example usage.

## Installing and Compiling

To setup add the repo as a submodule of your gdextension and then compile DataBind.hpp (don't forget to add the DataBind class to register_types as a RUNTIME_CLASS!) with godot-cpp 4.4, im sure you'll figure it out :)

Once compiled the DataBind class will work from either C++ or gdscript.

## Limitations

The DataBind class does not work for all properties, it currently only supports the properties I needed at the time of writing, which includes:

- pressed: for buttons, connects the pressed signal to a method from the controller class
- visible: for any control, calls set_visible()
- disabled: for buttons, calls set_disabled()
- text: for labels, calls set_text()
- texture: for any Control with a set_texture() method

However, adding more bindable properties is trivially done by editing \_notifcation() and update() in DataBind.cpp.

Another limitation/design choice is that DataBinds don't do anything with \_process() because evaluating a ton of Expressions every frame doesn't seem ideal to me, although you can still just call update() somewhere else in \_process. So things that constantly update like progress bars you'll still need to use a signal based approach or maybe add a similar function to update() in the DataBind class that is meant to run every frame.


## Other Similar Projects

- https://github.com/jamie-pate/godot-control-data-binds

- https://github.com/HotariTobu/gd-data-binding

- https://github.com/Boxxfish/ViewGD

## License

MIT License

Copyright (c) 2025 Dementive

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
