[![Русский](https://img.shields.io/badge/Русский-%F0%9F%87%B7%F0%9F%87%BA-green?style=for-the-badge)](README_ru.md)

# JavaScript (V8) Language Module for Plugify

The Plugify JavaScript Language Module enables developers to write plugins in modern JavaScript (ECMAScript 2023) and run them using the high-performance [V8 engine](https://v8.dev/). With this module, you can harness the flexibility of JavaScript while fully integrating into the Plugify plugin ecosystem.

## Features

- **JavaScript-Powered Plugins**: Write your plugins entirely in JavaScript using familiar syntax and tools.
- **Seamless Integration**: Integrate JavaScript plugins smoothly into the Plugify system alongside other supported languages.
- **Cross-Language Communication**: Interact effortlessly with plugins written in C++, Python, Lua, and more.
- **Sandboxed Execution**: Run JavaScript code safely in a sandboxed V8 environment with controlled access to native APIs.
- **Valve Pulse Integration**: Access Counter-Strike 2's native Pulse (cs_script) scripting system directly from your Plugify plugins, enabling seamless integration with game entities, events, and functionality.

## Valve Pulse ([cs_script](https://developer.valvesoftware.com/wiki/Counter-Strike_2_Workshop_Tools/Scripting)) Integration
<details>
<summary>Spoiler</summary>

The V8 Language Module provides unique integration with Valve's **Pulse scripting system** (also known as `cs_script`) for Counter-Strike 2. Through the s2sdk plugin, you can dynamically import and use Pulse modules alongside your Plugify plugins, creating powerful hybrid solutions that leverage both systems.

### Key Capabilities

- **Dynamic Module Loading**: Import Pulse modules like `cs_script/point_script`, `cs_script/entities`, and `cs_script/events` at runtime
- **Full Pulse API Access**: Use Valve's native scripting APIs for game entities, events, and game logic
- **Hybrid Plugin Development**: Combine Plugify's multi-language ecosystem with Pulse's game-specific functionality
- **Seamless Integration**: Automatically detect when Pulse becomes available and load modules dynamically

### Quick Example

```javascript
import { Plugin } from 'plugify';
import * as s2sdk from ':s2sdk';

export class HybridPlugin extends Plugin {
    pluginStart() {
        s2sdk.OnEntityCreated_Register(HybridPlugin.OnEntityCreated);
    }

    static OnEntityCreated(entityHandle) {
        const className = s2sdk.GetEntityClassname(entityHandle);
        
        if (className === "point_script") {
            setTimeout(() => {
                // Dynamically import Pulse modules after point_script is created
                import("cs_script/point_script").then(point_script => {
                    const { Instance } = point_script;
                    
                    console.log("✓ Pulse system connected!");
                    Instance.Msg("✓ Pulse integration active!");
                    
                    // Use Pulse functionality
                    Instance.SetThink(() => {
                        Instance.Msg("Think function executing...");
                        return Instance.GetGameTime() + 5.0;
                    });
                    Instance.SetNextThink(Instance.GetGameTime());
                });
            }, 100);
        }
    }
}
```

**Note**: Pulse modules must be imported dynamically after the `point_script` entity is created, as Plugify plugins load earlier than Valve's scripting system becomes available.

For comprehensive documentation on Pulse integration, see the [Pulse System Integration Guide](https://untrustedmodders.github.io/s2sdk/pulse-integration).

Learn more about Valve's Pulse system:
- [Counter-Strike 2 Scripting API](https://developer.valvesoftware.com/wiki/Counter-Strike_2_Workshop_Tools/Scripting_API)
- [Hello Gordon Tutorial](https://developer.valvesoftware.com/wiki/Counter-Strike_2_Workshop_Tools/Scripting/Hello_Gordon)
</details>

## Getting Started

### Prerequisites

- JavaScript `ES6` is recommended.
- Plugify Framework Installed
- For Pulse integration: [s2sdk](https://github.com/untrustedmodders/plugify-plugin-s2sdk) plugin installed

### Installation

#### Option 1: Install via Plugify Plugin Manager

```bash
mamba install -n your_env_name -c https://untrustedmodders.github.io/plugify-module-v8/ plugify-module-v8
```

#### Option 2: Manual Installation

1. Install dependencies:  

   a. Windows  
   > Setting up [CMake tools with Visual Studio Installer](https://learn.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio#installation)

   b. Linux:  
   ```sh
   sudo apt-get install -y build-essential cmake ninja-build
   ```
   
   c. Mac:  
   ```sh
   brew install cmake ninja
   ```

2. Clone this repository:

   ```bash
   git clone https://github.com/untrustedmodders/plugify-module-v8.git --recursive
   ```

3. Build the JavaScript language module:

   ```bash
   mkdir build && cd build
   cmake ..
   cmake --build .
   ```

### Usage

1. **Integration with Plugify**

   Ensure that your JavaScript language module is available in the same directory as your Plugify setup.

2. **Write JavaScript Plugins**

   Develop your plugins in JavaScript using the Plugify V8 API. Refer to the [Plugify JavaScript Plugin Guide](https://untrustedmodders.github.io/languages/javascript/first-plugin) for detailed instructions.

3. **Build and Install Plugins**

   Put your JavaScript files in a directory accessible to the Plugify core.

4. **Run Plugify**

   Start the Plugify framework, and it will dynamically load your JavaScript plugins.

## Example

```javascript
import { Plugin } from 'plugify';

export class ExamplePlugin extends Plugin {
    pluginStart() {
        console.log('ExamplePlugin::pluginStart');
    }
    pluginUpdate(dt) {
        console.log('ExamplePlugin::pluginUpdate');
    }
    pluginEnd() {
        console.log('ExamplePlugin::pluginEnd');
    }
}
```

## API References

- [`assert`](src/builtin/assert.rs) Provides a set of assertion tests
    - `assert.ok()` Checks if a value is true.
    - `assert.equal()` Checks if two values are equal, using the equal operator (==).
    - `assert.notEqual()` Checks if two values are not equal, using the not equal operator (!=)
- [`buffer`](src/builtin/buffer.rs) To handle binary data
  - TODO
- [`console`](src/builtin/buffer.rs) Console object.
    - `console.log()` Outputs a message to the console.
    - `console.info()` Outputs a message to the console.
    - `console.warn()` Outputs a message to the console.
    - `console.error()` Outputs a message to the console.
    - `console.debug()` Outputs a message to the console.
- [`child_process`](src/builtin/child_process.rs) 	To run a child process
  - TODO
- [`cluster`](src/builtin/cluster.rs) To split a single Node process into multiple processes
  - TODO
- [`crypto`](src/builtin/crypto.rs) To handle OpenSSL cryptographic functions
  - TODO
- [`dgram`](src/builtin/dgram.rs) Provides implementation of UDP datagram sockets
  - TODO
- [`dns`](src/builtin/dns.rs) To do DNS lookups and name resolution functions
  - TODO
- [`events`](src/builtin/events.rs) To handle events
  - TODO
- [`fs`](src/builtin/fs.rs) To handle the file system  - `fs.rename()` rename the file.
    - `fs.exists()`
    - `fs.rmdir()` remove the dir.
    - `fs.mkdir()` create the dir.
    - `fs.stat()` get the stat of the given pathname.
    - `fs.readdir()` read the directory.
    - `fs.readFile()` read the file of the given pathname.
    - `fs.writeFile()` write the file of the given pathname and bytes/string.
    - `fs.appendFile()` append to the file of the given pathname and bytes/string.
- [`http`](src/builtin/http.rs) To make Node.js act as an HTTP server
  - TODO
- [`https`](src/builtin/https.rs) To make Node.js act as an HTTPS server.
  - TODO
- [`net`](src/builtin/net.rs) To create servers and clients
  - TODO
- [`os`](src/builtin/os.rs) Provides information about the operation system
    - `os.tmpdir()` Returns the operating system's default directory for temporary files.
    - `os.homedir()` Returns the operating system's home directory.
    - `os.endianness()` Returns the endianness of the CPU.
    - `os.type()` Returns the name of the operating system.
    - `os.platform()` Returns information about the operating system's platform.
    - `os.arch()` Returns the operating system CPU architecture.
- [`path`](src/builtin/path.rs) To handle file paths
    - `path.normalize()` normalize the path string.
    - `path.join()` Joins the specified paths into one.
    - `path.resolve()` Resolves the specified paths into an absolute path.
    - `path.isAbsolute()` return if a path is in absolute.
    - `path.relative()` normalize the path string.
    - `path.dirname()` return the directory name of the path.
    - `path.basename()` return the basename of the path.
    - `path.extname()` return the ext name.
- [`querystring`](src/builtin/querystring.rs) To handle URL query strings
  - TODO
- [`readlin`](src/builtin/readlin.rs) To handle readable streams one line at the time
  - TODO
- [`stream`](src/builtin/stream.rs) To handle streaming data
  - TODO
- [`string_decoder`](src/builtin/string_decoder.rs) To decode buffer objects into strings
  - TODO
- [`timers`](src/builtin/timers.rs) To execute a function after a given number of milliseconds
    - `timers.setTimeout()` Executes a given function after a given time (in milliseconds).
    - `timers.clearTimeout()` Cancels a Timeout object.
- [`tls`](src/builtin/tls.rs) To implement TLS and SSL protocols
  - TODO
- [`tty`](src/builtin/tty.rs) Provides classes used by a text terminal
  - TODO
- [`url`](src/builtin/url.rs) To parse URL strings
  - TODO
- [`util`](src/builtin/util.rs) To access utility functions
  - TODO
- [`v8`](src/builtin/v8.rs) To access information about V8 (the JavaScript engine)
  - TODO
- [`vm`](src/builtin/vm.rs) To compile JavaScript code in a virtual machine
  - TODO
- [`zlib`](src/builtin/zlib.rs) To compress or decompress files 
  - TODO

## Documentation

For comprehensive documentation on writing plugins in JavaScript using the Plugify framework, refer to the [Plugify Documentation](https://untrustedmodders.github.io).

## Contributing

Feel free to contribute by opening issues or submitting pull requests. We welcome your feedback and ideas!

## License

This JavaScript (V8) Language Module for Plugify is licensed under the [MIT License](LICENSE).
