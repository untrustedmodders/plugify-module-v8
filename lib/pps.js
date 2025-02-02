var global = global || (function () { return this; }());
(function (global) {
    "use strict";
    let pps = global.pps = global.pps || {};
    let modules = global.modules = global.modules || {};
    
    let sendRequestSync = pps.sendRequestSync;

    function normalize(name) {
        if ('./' === name.substr(0, 2)) {
            name = name.substr(2);
        }
        return name;
    }

    let evalScript = global.__plgjsEvalScript || function(script) {
        return eval(script);
    }
    global.__plgjsEvalScript = undefined;
    
    let loadModule = global.__plgjsLoadModule;
    global.__plgjsLoadModule = undefined;
    
    let searchModule = global.__plgjsSearchModule;
    global.__plgjsSearchModule = undefined;
    
    let org_require = global.require;
    
    let findModule = global.__plgjsFindModule;
    global.__plgjsFindModule = undefined;
    
    let tmpModuleStorage = [];
    
    function addModule(m) {
        for (var i = 0; i < tmpModuleStorage.length; i++) {
            if (!tmpModuleStorage[i]) {
                tmpModuleStorage[i] = m;
                return i;
            }
        }
        return tmpModuleStorage.push(m) - 1;
    }
    
    function getModuleBySID(id) {
        return tmpModuleStorage[id];
    }

    let moduleCache = Object.create(null);
    let buildinModule = Object.create(null);
    function executeModule(fullPath, script, sid, isESM, bytecode) {
        sid = (typeof sid == 'undefined') ? 0 : sid;
        let fullPathInJs = fullPath.replace(/\\/g, '\\\\');
        let fullDirInJs = (fullPath.indexOf('/') != -1) ? fullPath.substring(0, fullPath.lastIndexOf("/")) : fullPath.substring(0, fullPath.lastIndexOf("\\")).replace(/\\/g, '\\\\');
        let exports = {};
        let module = pps.getModuleBySID(sid);
        module.exports = exports;
        let wrapped = evalScript(
            // Wrap the script in the same way NodeJS does it. It is important since IDEs (VSCode) will use this wrapper pattern
            // to enable stepping through original source in-place.
            (isESM || bytecode) ? script : "(function (exports, require, module, __filename, __dirname) { " + script + "\n});", 
            isESM, fullPath, bytecode
        )
        if (isESM) return wrapped;
        wrapped(exports, pps.genRequire(fullDirInJs), module, fullPathInJs, fullDirInJs)
        return module.exports;
    }
    
    function getESMMain(script) {
        let packageConfigure = JSON.parse(script);
        return (packageConfigure && packageConfigure.type === "module") ? packageConfigure.main : undefined;
    }
    
    function getSourceLengthFromBytecode(buf, isESM) {
        let sourceHash = (new Uint32Array(buf))[2];
        //console.log(`sourceHash:${sourceHash}`);
        const kModuleFlagMask = (1 << 31);
        const mask = isESM ? kModuleFlagMask : 0;

        // Remove the mask to get the original length
        const length = sourceHash & ~mask;

        return length;
    }
    
    let baseString
    function generateEmptyCode(length) {
        if (baseString === undefined) {
            baseString = " ".repeat(128*1024);
        }
        if (length <= baseString.length) {
            return baseString.slice(0, length);
        } else {
            const fullString = baseString.repeat(Math.floor(length / baseString.length));
            const remainingLength = length % baseString.length;
            return fullString.concat(baseString.slice(0, remainingLength));
        }
    }
    
    function genRequire(requiringDir, outerIsESM) {
        let localModuleCache = Object.create(null);
        function require(moduleName) {
            if (org_require) {
                try {
                    return org_require(moduleName);
                } catch (e) {}
            }
            moduleName = normalize(moduleName);
            let forceReload = false;
            if ((moduleName in localModuleCache)) {
                let m = localModuleCache[moduleName];
                if (!m.__forceReload) {
                    return localModuleCache[moduleName].exports;
                } else {
                    forceReload = true;
                }
            }
            if (moduleName in buildinModule) return buildinModule[moduleName];
            let nativeModule = findModule(moduleName);
            if (nativeModule) {
                buildinModule[moduleName] = nativeModule;
                return nativeModule;
            }
            let fullPath = searchModule(moduleName, requiringDir);
            if (!fullPath) {
                throw new Error(`can not find ${moduleName} in ${requiringDir}`);
            }
            
            let key = fullPath;
            if ((key in moduleCache) && !forceReload) {
                localModuleCache[moduleName] = moduleCache[key];
                return localModuleCache[moduleName].exports;
            }
            let m = {"exports":{}};
            localModuleCache[moduleName] = m;
            moduleCache[key] = m;
            let sid = addModule(m);
            let isESM = outerIsESM === true || fullPath.endsWith(".mjs") || fullPath.endsWith(".mbc");
            if (fullPath.endsWith(".cjs") || fullPath.endsWith(".cbc")) isESM = false;
            let script = isESM ? undefined : loadModule(fullPath);
            let bytecode = undefined;
            if (fullPath.endsWith(".mbc") || fullPath.endsWith(".cbc")) {
                bytecode = script;
                script = generateEmptyCode(getSourceLengthFromBytecode(bytecode));
            }
            try {
                if (fullPath.endsWith(".json")) {
                    let packageConfigure = JSON.parse(script);
                    
                    if (fullPath.endsWith("package.json")) {
                        isESM = packageConfigure.type === "module"
                        let url = packageConfigure.main || "index.js";
                        if (isESM) {
                            let packageExports = packageConfigure.exports && packageConfigure.exports["."];
                            if (packageExports)
                                url =
                                    (packageExports["default"] && packageExports["default"]["require"]) ||
                                    (packageExports["require"] && packageExports["require"]["default"]) ||
                                    packageExports["require"];                        
                            if (!url) {
                                throw new Error("can not require a esm in cjs module!");
                            }
                        }
                        let fullDirInJs = (fullPath.indexOf('/') != -1) ? fullPath.substring(0, fullPath.lastIndexOf("/")) : fullPath.substring(0, fullPath.lastIndexOf("\\")).replace(/\\/g, '\\\\');
                        let tmpRequire = genRequire(fullDirInJs, isESM);
                        let r = tmpRequire(url);
                        
                        m.exports = r;
                    } else {
                        m.exports = packageConfigure;
                    }
                } else {
                    let r = executeModule(fullPath, script, sid, isESM, bytecode);
                    if (isESM) {
                        m.exports = r;
                    }
                }
            } catch(e) {
                localModuleCache[moduleName] = undefined;
                moduleCache[key] = undefined;
                throw e;
            } finally {
                tmpModuleStorage[sid] = undefined;
            }
            return m.exports;
        }

        return require;
    }
    
    function registerBuiltinModule(name, module) {
        buildinModule[name] = module;
    }
	
	function forceReload(reloadModuleKey) {
        if (reloadModuleKey) {
            reloadModuleKey = normalize(reloadModuleKey);
        }
        let reloaded = false;
        for(var moduleKey in moduleCache) {
            if (!reloadModuleKey || (reloadModuleKey == moduleKey)) {
                moduleCache[moduleKey].__forceReload = true;
                reloaded = true;
                if (reloadModuleKey) break;
            }
        }
        if (!reloaded && reloadModuleKey) {
            console.warn(`reload not loaded module: ${reloadModuleKey}!`);
        }
    }

    function getModuleByUrl(url) {
        if (url) {
            url = normalize(url);
            return moduleCache[url];
        }
    }
    
    registerBuiltinModule("pps", pps);
    registerBuiltinModule("assert", modules.assert);
	//registerBuiltinModule("buffer", modules.buffer);
    registerBuiltinModule("console", modules.console);
    registerBuiltinModule("crypto", modules.crypto);
    registerBuiltinModule("datagram", modules.datagram);
    registerBuiltinModule("dns", modules.dns);
	//registerBuiltinModule("events", modules.events);
    registerBuiltinModule("fs", modules.fs);
    registerBuiltinModule("http", modules.http);
    registerBuiltinModule("https", modules.https);
    registerBuiltinModule("net", modules.net);
    registerBuiltinModule("os", modules.os);
    registerBuiltinModule("path", modules.path);
    registerBuiltinModule("process", modules.process);
    registerBuiltinModule("querystring", modules.querystring);
    registerBuiltinModule("readline", modules.readline);
    registerBuiltinModule("repl", modules.repl);
    registerBuiltinModule("tls", modules.tls);
    registerBuiltinModule("url", modules.url);
	//registerBuiltinModule("util", modules.util);
	
    pps.genRequire = genRequire;
    
    pps.getESMMain = getESMMain;
    
    pps.__require = genRequire("");
    
    global.require = pps.__require;
    
    pps.getModuleBySID = getModuleBySID;
    
    pps.registerBuiltinModule = registerBuiltinModule;

    pps.loadModule = loadModule;
    
    pps.forceReload = forceReload;
    
    pps.getModuleByUrl = getModuleByUrl;
    
    pps.generateEmptyCode = generateEmptyCode;
}(global));