# FM Function Reference — Browser Interface (`templates/fm-gui.html`)

This document covers the **front-end functions** of FM: what happens in the
browser when you click, right-click, type, or run something. It is the
counterpart to `COMMAND.md`, which documents the C++ core's command
behavior. Everything here lives in `templates/fm-gui.html`'s inline
`<script>` block, and talks to the core only through the two endpoints in
`run.py` (`GET /api/list`, `POST /api/run`).

---

## 1. Overview of the UI

The interface has four regions, top to bottom:

1. **Topbar** — logo, breadcrumb path (`#crumbs`), an `⌘` button to open a
   raw `oscmd`, and a `log` button to open the activity drawer.
2. **Main grid** (`#grid`) — the current directory's contents, one card per
   entry, plus an empty-state message when the folder has nothing in it.
3. **Composer** — the strip at the bottom of the screen with the raw
   command input box, the **receipt** (the in-progress command being
   built), a `clear` button, and the `Run ▸` button.
4. **Overlays** — the context menu, the activity log drawer, and the
   confirm/info/error modal, all hidden until triggered.

Everything the GUI does ultimately becomes one string sent to `POST
/api/run` as `{ "command": "<fm command line>" }`, or a `GET /api/list`
call to refresh the grid.

---

## 2. Browsing

- **Double-click a folder card** → `openEntry()` calls `navigateTo(joinPath(state.path, entry.name))`,
  which updates `state.path` and calls `refresh()` (re-fetches `/api/list`).
- **Double-click a file card** → `openEntry()` runs `cat "<path>"` via
  `runAndShow()` and displays the file's contents in an info modal.
- **Click a breadcrumb segment** → `navigateTo()` jumps straight to that
  ancestor path; the current (last) segment is not clickable.
- **`Enter` on a focused card** (keyboard navigation) → same as
  double-click, via the card's `keydown` listener.

---

## 3. Right-click — Context Menus (open the command composer)

This is the primary way to act on a file or folder without typing a
command by hand. Right-clicking anywhere in the grid opens a context menu
next to the cursor; **most items in that menu don't run a command
immediately — instead they open (or partially fill in) the command
composer at the bottom of the screen, so you can review or edit the
command before running it.**

### 3.1 Right-click on a file or folder card

Handled by each card's own `contextmenu` listener → `openEntryContextMenu(e, entry)`.
`e.stopPropagation()` is called first so the click doesn't also trigger the
empty-area menu underneath it.

**Files** get:
| Menu item | What it does |
|---|---|
| Edit | Opens the composer pre-filled as `edit [ ] "<file>"`, with the editor-name field left blank and focused, ready to type (see §4). |
| Rename | Opens the composer pre-filled as `rn "<file>" [ ]`, with the new-name field focused. |
| Info | Runs `info "<path>"` immediately and shows the result in an info modal — does **not** touch the composer. |
| Delete | Opens a confirm modal (`Delete "<file>"? This can't be undone.`); confirming runs `rm "<path>" Y` immediately. |

**Folders** get the same layout, except the first item is:
| Menu item | What it does |
|---|---|
| Open (cd) | Navigates into the folder immediately (`navigateTo`), same as double-click. |

Rename / Info / Delete behave identically to the file case, operating on
the folder's path instead.

### 3.2 Right-click on empty grid space

Handled by a `contextmenu` listener on `#main`, which only fires if the
click target isn't inside `.entry` (so it doesn't double-fire when you
right-click a card) → `openEmptyContextMenu(e)`.

| Menu item | What it does |
|---|---|
| New folder | Opens the composer pre-filled as `mk dir [ ]`, name field focused. |
| New file | Opens the composer pre-filled as `mk file [ ]`, name field focused. |

### 3.3 How the menu is positioned and dismissed

- `showContextMenu(x, y, items)` builds the menu's buttons, places it at
  the cursor, then re-measures itself and clamps `x`/`y` so it never
  renders off-screen (`Math.min(x, window.innerWidth - rect.width - 8)`,
  same for `y`, floored at `8`).
- The menu closes on: clicking anywhere outside it, scrolling (any
  scroll event, capture-phase), pressing `Escape`, or selecting an item
  (`closeContextMenu()` runs right before the item's `action()`).

---

## 4. The Composer & the "Receipt" (command tab)

The composer is the part of the UI that the right-click menu feeds into.
`state.pending` holds the command currently being built:

```js
state.pending = { verb: 'rn', tokens: [ {label, value, removable}, {input:true, placeholder, value} ] }
```

- `startVerb(verb, tokens)` sets `state.pending` and calls
  `renderReceipt()`, which is what actually draws the chips in the
  `#receipt` row: a teal **verb chip** (`rn`, `edit`, `mk`, `oscmd`, …)
  followed by `›` arrows and either:
  - a **static chip** — a fixed argument (e.g. the file being renamed),
    optionally removable (an `✕` that clears the whole pending command), or
  - an **editable input** — a small inline text box for the part you still
    need to type (new name, editor name, mk target name, raw oscmd
    string). The first such input is auto-focused.
- **`Run ▸`** is disabled until every editable token has a non-empty,
  trimmed value (`syncRunEnabled()`), or until you've typed something in
  the raw command box instead.
- **`clear`** (only visible while a command is pending) discards
  `state.pending` and resets the receipt to its placeholder text
  (`right-click a file or folder to act on it`).
- Pressing **`Enter`** inside one of the receipt's inline inputs also
  triggers `attemptRun()`, so you don't have to reach for the mouse.

### Building the final command string

`buildCommandString()` joins the verb and each token's current value,
quoting values that need it (`quoteIfNeeded()` wraps anything with
characters outside `[A-Za-z0-9_./:-]` in single quotes, escaping embedded
`'`). It also re-qualifies bare names typed into `rn` / `mk` / `edit`
composer flows with the current directory (`entryPath()`), so typing just
`notes.txt` while inside `projects/2026` produces
`projects/2026/notes.txt` in the final command.

`oscmd` is special-cased in `attemptRun()`: instead of running straight
away, it opens a confirm modal showing the exact command that will be
executed, and only runs it if you confirm.

---

## 5. Raw Command Input

The `#rawCommandInput` box next to `Run ▸` is a second, independent way to
issue a command: type any full FM command line and press `Enter` or click
`Run ▸`. If there's text in this box, `attemptRun()` prioritizes it over
whatever is in the receipt, runs it via `runCommand`, shows the result in
an info modal, and clears the box afterward.

---

## 6. Running a Command — `executeCommand()`

Every path described above — context-menu actions, the composer's `Run`,
and the raw input box — funnels into `executeCommand(command, opts)`:

1. Disables `Run ▸` for the duration of the call.
2. Calls `API.runCommand(command)` → `POST /api/run`.
3. If the response text starts with `PATH:` (see `COMMAND.md`'s `cd`
   entry), updates `state.path` to the value after the prefix.
4. Logs the command and result to the activity drawer (`logEntry()`).
5. On failure (`result.ok === false`), opens an **error modal** with the
   response text.
6. On success, if `opts.showResult` was requested (used for `cat`, `info`,
   and the raw-input flow), opens an **info modal** with the response
   text.
7. Clears `state.pending` and re-renders the receipt back to its empty
   state.
8. Calls `refresh()` to reload the current directory's listing regardless
   of success or failure, so the grid always reflects the latest state.

---

## 7. Modal System

One shared modal (`#modalOverlay`) is reused for three purposes, chosen by
`openModal({ kind, title, message, onConfirm })`:

- **`confirm`** — shown before destructive/irreversible actions (`Delete`,
  raw `oscmd`). Shows **Cancel** and a red **Confirm** button; only
  `Confirm` calls `onConfirm()`.
- **`error`** — amber title, single **Dismiss** button.
- **`info`** (default) — single **Close** button, used for command output
  such as `cat`/`info` results or any raw-input command result.

The modal also closes on clicking its dark backdrop or pressing `Escape`.

---

## 8. Activity Log Drawer

Every command run through `executeCommand()` (context-menu actions,
composer runs, and raw input) is appended to the `#logBody` list via
`logEntry(command, ok, output)`, newest first, each entry showing the
command text, a `success`/`fail` status pill, and the raw output (if any).
The drawer itself (`#log`) slides in from the right when the `log` button
in the topbar is clicked, with a dimming scrim behind it; it closes via its
`✕` button or by clicking the scrim.

---

## 9. Raw `oscmd` Shortcut

The `⌘` button in the topbar is a shortcut into the same composer flow
used by the context menu: it calls `startVerb('oscmd', [{input:true,
placeholder:'raw OS command…', value:''}])`, giving you an inline field to
type any OS command. Submitting it always goes through the confirm modal
described in §4 before it's sent to `/api/run` — there is no way to fire
an `oscmd` from the GUI without that confirmation step, even though the
core itself (see `COMMAND.md`) no longer prompts for confirmation on its
own.

---

## 10. API Contract Recap

| Function | Endpoint | Request | Response used by GUI |
|---|---|---|---|
| `API.listDir(path)` | `GET /api/list?path=` | — | `{ path, entries: [{name, type}] }` — drives the grid and breadcrumbs |
| `API.runCommand(command)` | `POST /api/run` | `{ command }` | `{ ok, output/response }` — drives modals, the log drawer, and `PATH:` path updates |

Both throw a plain `Error` on a non-OK HTTP status, using `data.output` as
the message where available, which the calling function then turns into a
log entry and/or an error modal.
