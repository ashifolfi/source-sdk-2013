# Engine Tool Development
Engine tools are very undocumented and thus extremely difficult to make, these notes aim to assist with this. These notes, alongside the sample code, detail everything I found that's required to create a barebones functional tool.

## Anatomy of a tool
A tool is of the `source_dll` project type, being based upon an `IAppSystem` interface on the basic level, and stored as a `.dll` file inside `bin/tools`.

**Every tool has the following within it:**
- exposed `IToolDictionary` interface implementation
- at least one `IToolSystem` implementation

The `IToolSystem` class is what contains the actual hooks and tool code, `IToolDicionary` is merely the *entry point* that allows the Source Engine to create and get every tool present within your DLL. DLLs can contain more than one tool as well.

## Tool System
A barebones implementation of `IToolSystem` *(found in `public/toolframework/itoolsystem.h`)* will look like this:

<details>
	<summary>Header</summary>

```cpp
// exampletool.h
class ExampleTool : public IToolSystem
{
public:
    // Name describing the tool
    virtual char const *GetToolName();

    // Called at end of engine startup
    virtual bool Init();
    // Called during RemoveTool or when engine is shutting down.
    virtual void Shutdown();

    // This tool is being activated
    virtual void OnToolActivate();
    // Another tool is being activated
    virtual void OnToolDeactivate();
};
```

</details>

<details>
	<summary>Source</summary>

```cpp
// exampletool.cpp
char const *ExampleTool::GetToolName() {
	return "Example Tool";
}

bool ExampleTool::Init() {
	Msg("Initialized tool");
	return true;
}

void ExampleTool::Shutdown() {
	Msg("Shutting down tool");
}

void ExampleTool::OnToolActivate() {
	Msg("Activating tool");
}

void ExampleTool::OnToolDeactivate() {
	Msg("Deactivating tool");
}
```

</details>
