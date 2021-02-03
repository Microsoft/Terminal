---
author: Carlos Zamora @carlos-zamora
created on: 2019-08-30
last updated: 2020-06-26
issue id: 715
---

# Keyboard Selection

## Abstract

This spec describes a new set of keybindings that allows the user to create and update a selection without the use of a mouse or stylus.

## Inspiration

ConHost allows the user to modify a selection using the keyboard. Holding `Shift` allows the user to move the second selection endpoint in accordance with the arrow keys. The selection anchor updates by one cell per key event, allowing the user to refine the selected region.

Mark mode allows the user to create a selection using only the keyboard, then edit it as mentioned above.


## Solution Design

The fundamental solution design for keyboard selection is that the responsibilities between the Terminal Control and Terminal Core must be very distinct. The Terminal Control is responsible for handling user interaction and directing the Terminal Core to update the selection. The Terminal Core will need to update the selection according to the preferences of the Terminal Control.


### Fundamental Terminal Control Changes

`TermControl::_KeyDownHandler()` is responsible for interpreting the key events. At the time of writing this spec, there are 3 cases handled in this order:
- Alt+Gr events
- Keybindings
- Send Key Event (also clears selection)

Conditionally effective keybindings can be introduced to update the selection accordingly. A well known conditionally effective keybinding is the `Copy` action where text is copied if a selection is active, otherwise the key event is marked as `handled = false`. See "UI/UX Design" --> "Keybindings" for more information regarding keybindings.

#### Idea: Make keyboard selection a simulation of mouse selection
It may seem that some effort can be saved by making the keyboard selection act as a simulation of mouse selection. There is a union of mouse and keyboard activity that can be represented in a single set of selection motion interfaces that are commanded by the TermControl's Mouse/Keyboard handler and adapted into appropriate motions in the Terminal Core.

However, the mouse handler operates by translating a pixel coordinate on the screen to a text buffer coordinate. This would have to be rewritten and the approach was deemed unworthy.


### Fundamental Terminal Core Changes

The Terminal Core will need to expose a `MoveSelectionEndpoint()` function that is called by the keybinding handler. The following parameters will need to be passed in:
- `enum Direction`: the direction that the selection anchor will attempt to move to. Possible values include `Up`, `Down`, `Left`, and `Right`.
- `enum SelectionExpansionMode`: the selection expansion mode that the selection anchor will adhere to. Possible values include `Cell`, `Word`, `Viewport`, `Buffer`.

#### Moving by Cell
For `SelectionExpansionMode = Cell`, the selection anchor will be updated according to the buffer's output pattern. For **horizontal movements**, the selection anchor will attempt to move left or right. If a viewport boundary is hit, the anchor will wrap appropriately (i.e.: hitting the left boundary moves it to the last cell of the line above it).

For **vertical movements**, the selection anchor will attempt to move up or down. If a **viewport boundary** is hit and there is a scroll buffer, the anchor will move and scroll accordingly by a line. If a **buffer boundary** is hit, the anchor will not move. In this case, however, the event will still be considered handled.

**NOTE**: An important thing to handle properly in all cases is wide glyphs. The user should not be allowed to select a portion of a wide glyph; it should be all or none of it. When calling `_ExpandWideGlyphSelection` functions, the result must be saved to the anchor.

#### Moving by Word
For `SelectionExpansionMode = Word`, the selection anchor will also be updated according to the buffer's output pattern, as above. However, the selection will be updated in accordance with "chunk selection" (performing a double-click and dragging the mouse to expand the selection). For **horizontal movements**, the selection anchor will be updated according to the `_ExpandDoubleClickSelection` functions. The result must be saved to the anchor. As before, if a boundary is hit, the anchor will wrap appropriately. See [Future Considerations](#FutureConsiderations) for how this will interact with line wrapping.

For **vertical movements**, the movement is a little more complicated than before. The selection will still respond to buffer and viewport boundaries as before. If the user is trying to move up, the selection anchor will attempt to move up by one line, then selection will be expanded leftwards. Alternatively, if the user is trying to move down, the selection anchor will attempt to move down by one line, then the selection will be expanded rightwards.

#### Moving by Viewport
For `SelectionExpansionMode = Viewport`, the selection anchor will be updated according to the viewport's height. Horizontal movements will be updated according to the viewport's width, thus resulting in the anchor being moved to the left/right boundary of the viewport.

#### Moving by Buffer

For `SelectionExpansionMode = Buffer`, the selection anchor will be moved to the beginning or end of all the text within the buffer. If moving up or left, set the position to 0,0 (the origin of the buffer). If moving down or right, set the position to the last character in the buffer.


**NOTE**: In all cases, horizontal movements attempting to move past the left/right viewport boundaries result in a wrap. Vertical movements attempting to move past the top/bottom viewport boundaries will scroll such that the selection is at the edge of the screen. Vertical movements attempting to move past the top/bottom buffer boundaries will be clamped to be within buffer boundaries.

Every combination of the `Direction` and `SelectionExpansionMode` will map to a keybinding. These pairings are shown below in the UI/UX Design --> Keybindings section.

**NOTE**: If `copyOnSelect` is enabled, we need to make sure we update the clipboard on every change in selection.


## UI/UX Design

### Keybindings

Thanks to Keybinding Args, there will only be 2 new commands that need to be added:
| Action | Keybinding Args | Description |
|--|--|--|
| `moveSelectionPoint` |                                                               | If a selection exists, moves the last selection anchor.
|                       | `Enum direction { up, down, left, right }`                     | The direction the selection will be moved in. |
|                       | `Enum expandBy { cell, word, page, all }`   | The context for which to move the selection anchor to. (defaults to `cell`)
| `selectAll`  | | Select the entire text buffer.


By default, the following keybindings will be set:
```JS
// Cell Selection
{ "command": { "action": "moveSelectionPoint", "direction": "down" },  "keys": "shift+down" },
{ "command": { "action": "moveSelectionPoint", "direction": "up" },    "keys": "shift+up" },
{ "command": { "action": "moveSelectionPoint", "direction": "left" },  "keys": "shift+left" },
{ "command": { "action": "moveSelectionPoint", "direction": "right" }, "keys": "shift+right" },

// Word Selection
{ "command": { "action": "moveSelectionPoint", "direction": "left",    "expandBy": "word" }, "keys": "ctrl+shift+left" },
{ "command": { "action": "moveSelectionPoint", "direction": "right",   "expandBy": "word" }, "keys": "ctrl+shift+right" },

// Viewport Selection
{ "command": { "action": "moveSelectionPoint", "direction": "left",    "expandBy": "page" }, "keys": "shift+home" },
{ "command": { "action": "moveSelectionPoint", "direction": "right",   "expandBy": "page" }, "keys": "shift+end" },
{ "command": { "action": "moveSelectionPoint", "direction": "up",      "expandBy": "page" }, "keys": "shift+pgup" },
{ "command": { "action": "moveSelectionPoint", "direction": "down",    "expandBy": "page" }, "keys": "shift+pgdn" },

// Buffer Corner Selection
{ "command": { "action": "moveSelectionPoint", "direction": "up",      "expandBy": "all" }, "keys": "ctrl+shift+home" },
{ "command": { "action": "moveSelectionPoint", "direction": "down",    "expandBy": "all" }, "keys": "ctrl+shift+end" },

// Select All
{ "command": "selectAll", "keys": "ctrl+shift+a" },
```
These are in accordance with ConHost's keyboard selection model.

## Capabilities

### Accessibility

Using the keyboard is generally a more accessible experience than using the mouse. Being able to modify a selection by using the keyboard is a good first step towards making selecting text more accessible.

### Security

N/A

### Reliability

With regards to the Terminal Core, the newly introduced code should rely on already existing and tested code. Thus no crash-related bugs are expected.

With regards to Terminal Control and the settings model, crash-related bugs are not expected. However, ensuring that the selection is updated and cleared in general use-case scenarios must be ensured.

### Compatibility

N/A

### Performance, Power, and Efficiency

## Potential Issues

The settings model makes all of these features easy to disable, if the user wishes to do so.

## Future considerations

### Grapheme Clusters
When grapheme cluster support is inevitably added to the Text Buffer, moving by "cell" is expected to move by "character" or "cluster". This is similar to how wide glyphs are handled today. Either all of it is selected, or none of it.

### Word Selection Wrap
At the time of writing this spec, expanding or moving by word is interrupted by the beginning or end of the line, regardless of the wrap flag being set. In the future, selection and the accessibility models will respect the wrap flag on the text buffer.

### Contextual Keybindings
This feature introduces a large number of keybindings that only work if a selection is active. Currently, key bindings cannot be bound to a context, so if a user binds `moveSelectionPoint` to `shift+up` and there is no selection, `shift+up` is sent directly to the Terminal. In the future, a `context` key could be added to new bindings to get around this problem. That way, users could bind other actions to `shift+up` to run specifically when a selection is not active.

## Mark Mode

This functionality will be expanded to create a feature similar to Mark Mode. This will allow a user to create a selection using only the keyboard.


## Resources

- https://blogs.windows.com/windowsdeveloper/2014/10/07/console-improvements-in-the-windows-10-technical-preview/
