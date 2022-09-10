# bugs

- [ ] never word-wrap on non-whitespace characters; e.g. don't break up long
  ipfs://bafybei... URLs even if the markdown parser doesn't recognize them as
  URLs
- [ ] crash on ctrl-k sometimes
- [ ] moving checkbox items from current list to done list seems to be triggering
  even when it's not initially checked (control-k to convert bullet item)
- [ ] Document::saveResources(): come up with a better way to find them all

# todo

- [ ] don't change indent when converting bullet point to checkbox list item
- [ ] deal with nested lists (indented subtasks)
- [ ] make new lists when needed (when dragging to a column corresponding to an
  empty heading)
- [ ] metadata: columns that are kanban categories and/or those that aren't?
- [ ] metadata: tag the yaml class, layout or whatever (yaml: "layout: kanban"
  perhaps)
- [ ] metadata: yaml parsed separately before parsing markdown?
- [ ] saving to ipfs: strip yaml and convert to json-ld? (setting for that)
- [ ] use filesystem metadata too
- [ ] kanban creation wizard to select common columns and ordering
- [ ] kanban view in main window as alternative to QTextBrowser view (tabs?)
- [ ] come up with plugin architecture, make kanban-mode a plugin?
- [ ] transplant immediately when checked/unchecked
  (on_actionToggle_Checkbox_toggled isn't the best place because it doesn't
  change state until the cursor moves after clicking the checkbox)
- [ ] transactional undo
- [ ] fix paragraph spacing
- [ ] attempt quote styling (just draw lines in editor subclass?)
- [ ] todo.txt format support: should look like kanban with only 2 columns (todo
  and done); support DnD between both kinds of lists
- [ ] DnD to drop multiple files, in both reading and editing modes
- [ ] table navigation: tab between columns, insert new rows when necessary
- [ ] table editing features: insert and remove rows etc.
- [ ] mini-spreadsheet?
- [ ] dogfood the Qt 6 version
- [ ] IPFS features without KIO (?)
- [ ] link & anchor shapes
- [ ] support text ranges in fragment identifiers

# doing

- [ ] automatically move tasks to the done column when checked

# done

- [x] arbitrary columns in kanban view (treeview for every heading)
- [x] DnD between columns
- [x] DnD within columns to re-order
- [x] fix DnD to drop between existing items rather than onto them
- [x] DnD between boards (different md files)
- [x] kanban treeviews are missing edit notifications and can be temporarily out of
  sync
- [x] automatically check tasks when moved to the done column
- [x] saving resources to doc dir instead of the resource dir
- [x] SDI: multiple docs in multiple windows
- [x] task cut & paste loses the checkbox (QTBUG-103714)
- [x] SDI: open links in new windows

# research

- [ ] new ipns name for each document
- [ ] ipns links in each kanban task, to host a sub-document with details
- [ ] dependency graphing (dependencies in yaml / json-ld / fs metadata? some other
  way?)
  - [ ] tasks can have dependencies and entire projects can have dependencies
- [ ] plugins for rendering frames (code blocks in specific languages?)
- [ ] put metadata into Qt html export (at least html title tag already works but
  we need yaml or embedded html to get it from markdown, it seems. what about
  layout? export as body class or something? add new enum to
  QTextDocument::MetaInformation)
- [ ] use libgit2 to manage revisions if the file is in a git repo
  - [ ] but only if the yaml metadata says to do that, or only if it doesn't say
    not to?
- [ ] git on ipfs: follow the standard if there is one
- [ ] techtree: multiple kanban projects get organized into a long-term roadmap
- [ ] collaboration features: share the roadmap and the kanban boards, solicit help
  from others, encourage sharing and merging of roadmaps
- [ ] android and iOS versions: see a todo list in portrait mode, kanban columns in
  landscape mode (for easy DnD and reordering); tap on a task to open a little
  bottom panel to see the rest of elided text, to follow links, etc.
  - [ ] therefore needs better model-view separation
