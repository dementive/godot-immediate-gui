# Godot Immediate Mode Gui

Godot 4.5 C++ module code that adds a DataBind class that allows you to write immediate mode gui with data bindings with as little boilerplate as possible.

## Usage

The steps to use the DataBind class in your project are:

1. Make a custom class that Extends the DataBind class.
2. Add methods to the data bind class that call out to your data to update the UI state.
3. Add the data bind Node as the root node of a new GUI scene in the Godot editor (or with code) and add all the control nodes you'll need for your UI into it.
4. To bind methods from your data bind class to your view all you have to do is set String metadata properties, that correlate to actual Control properties, in the editor with a value that is a valid godot [Expression](https://docs.godotengine.org/en/stable/tutorials/scripting/evaluating_expressions.html) or Callable with methods from the data bind class. Since Expressions are used to execute the controller methods you can use boolean logic or even do math in the Expressions for maximum flexibility, so something like `X() && Y() || Z()` would work for controls with complex state.
5. If the metadata properties were correctly hooked up to the DataBind node when the scene with the data bind enters the tree all UI will automatically update every frame.

## Example

There are also several examples in the `examples` directory with code I pulled straight from my projects where I actually use DataBinds. The code is not complete but reading over it should give a good idea of the things you can do.
The examples are:

- Hud - Simple example that only has a label and a few buttons for a game hud.

- TransferResourcesPopup - Moderately complex example of a popup that transfers in game resources from one game object to another.

- PlanetView - Very complex example that shows just about everything you can do with DataBinds. It shows progress bars, using expressions in data bind properties, setting textures, dynamically creating item lists, handling open/close logic, and using datamodels.

The TransferResourcesPopup and PlanetView both use an older version of the DataBind class so everything won't be the same in those as it currently works. The Hud example is up to date with the current version though.

## Installing and Compiling

Copy the DataBind.cpp and DataBind.hpp files into your project and update your build system to compile them.

## Implementation Details

Normally with godot you have to write a lot of code to keep the data that comes from your game logic in sync with the UI. When there gets to be a lot of data coming from a lot of places and being displayed in a lot of places this becomes pretty easy to mess up. Not only is it bug-prone but it is also a lot of code to write that doesn't really do anything, users will never care how data is synchronized with UI as long as it works. Using Immediate mode for gui solves the data synchronization problem completely as the data automatically gets updated every frame. This way you can write a DataBind class, hook it up to your game logic data, hook the UI in the SceneTree up to the DataBind and then the UI will update automatically without the need to write any extra data synchronization code.


Godot uses retained mode for it's Control gui system so unless we completely forsake the Control system (which I don't want to do because it is really nice to use) the immediate mode part has to be hacked on top of the existing Control system somehow. This sounds pretty sketchy but it's not too bad, ReactJS does this same kind of thing with their "virtual DOM" hacked on top of the browsers DOM.


The way I ended up solving this with the DataBind class was by hacking "data bind properties" on top of godot's Object `metadata` property. Every Object has a metadata Dictionary and with this we can associate data bind properties with the Expression/Callable of functions on the DataBind node. 


This works out pretty well since metadata properties can be set directly in the editor SceneTree so data bind properties can easily be adjusted at runtime without recompiling. A nice side effect that's part of this hack is that there is almost never a need to directly reference a control node with get_node (or some syntax sugar that calls get_node) because the DataBind handles setting everything up and updating it. Normally when writing gui in godot there has to be tons of get_node calls at some point to reference a node in the scene tree in order to update it but with the DataBind you don't have to do any of this.


When setting up DataBind nodes in a scene they must be the root node of the scene they are in so they can properly find all data bind properties on all nodes in the scene.


The DataBind class works in 3 stages:

1. Initialization - When a scene with a DataBind Node in it is instantiated the first thing it does is traverse the SceneTree. This will register all Control Nodes and associate them with their data bind metadata properties. Different properties have different initialization steps. For example the `pressed` property will automatically connect the pressed signal of a Button control Node and the `datamodel` property will automatically instantiate nested data bind scenes. 

2. Update - Every frame a data bind scene is in the tree every data bind property it found when initializing will be executed.

3. Execute - If a data bind property needs to be updated then it's meta data function is called and the result of it is sent into the corresponding godot method to update the UI. For example, given a meta data property of `visible` with a value of `IsThingVisible()` the DataBind will call the IsThingVisible function and use it's result to call the godot `set_visible` function to actually change the control's visibility. If IsThingVisible has no arguments it will be executed as a Callable, otherwise it will be executed as an Expression. Callables also do not need parentheses, so a meta data property value of `IsThingVisible()` will end up being an Expression but `IsThingVisible` will be a Callable. Executing Callables is a lot faster than Expressions but Expressions are significantly more flexible and can do more so there are options to do both.

## Limitations

The DataBind class does not work for all properties on a Control Node, it currently only supports the properties I needed at the time of writing, which includes:

- pressed: for buttons, connects the pressed signal to a method from the controller class
- datamodel: a data model is used for instantiating other scenes that have a different DataBind, this allows nesting data model scenes in the scene tree. The metadata argument function call must return an Array of Nodes where each node is the root node of the data model scene to instantiate. For example if you were making an Inventory UI you might have a "InventorySlot" scene with 20 slots, instead of putting the 20 scenes right in the tree the datamodel will handle all this automatically.
- visible - Calls a control's set_visible function. Visible has different behavior than all other properties as it must be checked every frame to ensure correct visibility of nested data binds. All other properties that get checked every frame only get their functions run if they are actually visible in the scene tree.
- disabled - Calls a control's set_disabled function.
- text - Calls a label's set_text function.
- texture - Calls a control's set_texture function.
- icon - Calls a control's set_button_icon function.
- tooltip - Calls a control's set_tooltip function.
- progress - Calls a control's set_progress function.

The `datamodel` and `pressed` properties are not checked every frame, their functions are only run one time when the data model scene is first instantiated.

However, adding more bindable properties is trivially done by editing \_notifcation() and update() in DataBind.cpp.

## Performance

Most of the time signal based UI architecture will be more performant than immediate mode since it only has to do updates when you explicitly tell it to update instead of every frame.


However I've found that 99% of the time the extra performance hit of doing UI updates every frame makes almost no difference at all on the total performance of the application. I've profiled my godot applications with intel vtune, perf, hotspot, and valgrind and in all my projects a majority of the cpu time is spent just rendering the scene. The relative cost of updating the UI every frame is mostly negligible compared to the how expensive rendering every frame is.


With that said you can of course still write code that is super slow if your UI updates are very expensive. If your UI update logic is already super slow it's going to be much more noticeable with immediate mode than with retained mode since updates happen a lot more often.


There are easy ways around this though if you analyze how your data is used and cache the value. For example if you have a function `do_thing()` that calls into your game data and then computes some super expensive value that takes 5ms to compute it's probably not a great idea to call it every frame. Most values likely don't need to be computed every frame. Caching the result of `do_thing()` by only calling it when the computed data actually changes (or just call it less frequently like every 120th frame instead of every frame) and storing it in a variable somewhere can help fix slow updates.

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
