#!/usr/bin/python3
import sys
import argparse
import os
import json
from enum import IntEnum


TYPES_MAP = {
    'void': 'void',
    'bool': 'boolean',
    'char8': 'string',
    'char16': 'string',
    'int8': 'number',
    'int16': 'number',
    'int32': 'number',
    'int64': 'number',
    'uint8': 'number',
    'uint16': 'number',
    'uint32': 'number',
    'uint64': 'bigint',
    'ptr64': 'bigint',
    'float': 'number',
    'double': 'number',
    'function': 'delegate',
    'string': 'string',
    'any': 'any',
    'bool[]': 'boolean[]',
    'char8[]': 'string[]',
    'char16[]': 'string[]',
    'int8[]': 'number[]',
    'int16[]': 'number[]',
    'int32[]': 'number[]',
    'int64[]': 'number[]',
    'uint8[]': 'number[]',
    'uint16[]': 'number[]',
    'uint32[]': 'number[]',
    'uint64[]': 'bigint[]',
    'ptr64[]': 'bigint[]',
    'float[]': 'number[]',
    'double[]': 'number[]',
    'string[]': 'string[]',
    'any[]': 'any[]',
    'vec2[]': 'Vector2[]',
    'vec3[]': 'Vector3[]',
    'vec4[]': 'Vector4[]',
    'mat4x4[]': 'Matrix4x4[]',
    'vec2': 'Vector2',
    'vec3': 'Vector3',
    'vec4': 'Vector4',
    'mat4x4': 'Matrix4x4'
}


INVALID_NAMES = {
    'await',       # Reserved for async/await
    'break',       # Flow control
    'case',        # Switch statement
    'catch',       # Exception handling
    'class',       # Class declaration
    'const',       # Constant declaration
    'continue',    # Loop control
    'debugger',    # Debugging keyword
    'default',     # Switch statement
    'delete',      # Delete operator
    'do',          # Loop control
    'else',        # Conditional
    'enum',        # Future reserved keyword
    'export',      # Module export
    'extends',     # Class inheritance
    'false',       # Boolean literal
    'finally',     # Exception handling
    'for',         # Loop control
    'function',    # Function declaration
    'if',          # Conditional
    'import',      # Module import
    'in',          # Loop operator
    'instanceof',  # Instance check
    'let',         # Variable declaration
    'new',         # Object instantiation
    'null',        # Null literal
    'return',      # Return statement
    'super',       # Superclass reference
    'switch',      # Conditional
    'this',        # Context reference
    'throw',       # Exception handling
    'true',        # Boolean literal
    'try',         # Exception handling
    'typeof',      # Type operator
    'var',         # Variable declaration
    'void',        # Void operator
    'while',       # Loop control
    'with',        # Deprecated
    'yield',       # Generator functions
}


def generate_name(name: str) -> str:
    """Generate a valid Python variable name."""
    return f'{name}_' if name in INVALID_NAMES else name


def convert_type(param: dict) -> str:
    """Convert a JSON-defined type to Python typing."""
    type_name = param.get('type', '')
    result = TYPES_MAP.get(type_name, '')
    #if not result:
    #    raise ValueError(f'Unsupported type: {type_name}')
    if 'delegate' in result and 'prototype' in param:
        return generate_name(param['prototype'].get('name', 'UnnamedDelegate'))
    elif 'enum' in param:
        if '[]' in type_name:
            return f'{generate_name(param["enum"].get("name", "UnnamedEnum"))}[]'
        else:
            return generate_name(param['enum'].get('name', 'UnnamedEnum'))
    return result


class ParamGen(IntEnum):
    """Enumeration for parameter generation modes."""
    Types = 1
    Names = 2
    TypesNames = 3


def gen_params(params: list[dict], param_gen: ParamGen) -> str:
    """Generate function parameters as strings."""
    def gen_param(index: int, param: dict) -> str:
        return convert_type(param) if param_gen == ParamGen.Types else f'{generate_name(param.get("name", f"p{index}"))}: {convert_type(param)}'

    result = []
    if params:
        for i, p in enumerate(params):
            result.append(gen_param(i, p))
    return ', '.join(result)


def gen_return(ret_type: dict, param_types: list[dict]) -> str:
    if not any('ref' in p and p['ref'] is True for p in param_types):
        return convert_type(ret_type)
    result = [convert_type(ret_type)]
    for p in param_types:
        if 'ref' in p and p['ref'] is True:
            result.append(convert_type(p))
    return f'[{",".join(result)}]'


def gen_documentation(method: dict, tab_level: int = 0) -> str:
    """
    Generate a JavaScript function documentation string (JSDoc) from a JSON block.

    Args:
        method (Dict[str, Any]): The input JSON data describing the function.
        tab_level (int): The level of tabulation for the generated comment.

    Returns:
        str: The generated JSDoc documentation string.
    """
    # Extract general details
    name = method.get('name', 'UnnamedFunction')
    description = method.get('description', 'No description provided.')
    param_types = method.get('paramTypes', [])
    ret_type = method.get('retType', {}).get('type', 'void')

    # Determine tabulation
    tab = '  ' * tab_level

    # Start building the JSDoc comment
    docstring = [f'{tab}/**\n{tab} * {description}\n{tab} *\n']

    # Add parameters
    for param in param_types:
        param_name = param.get('name', 'UnnamedParam')
        param_type = param.get('type', 'Any')
        param_desc = param.get('description', 'No description available.')
        docstring.append(f'{tab} * @param {{{param_type}}} {param_name} - {param_desc}\n')

    # Add return type
    if ret_type.lower() != 'void':
        ret_desc = method.get('retType', {}).get('description', 'No description available.')
        docstring.append(f'{tab} * @returns {{{ret_type}}} - {ret_desc}\n')

    # Close JSDoc comment
    docstring.append(f'{tab} */')

    return ''.join(docstring)


def gen_enum_body(enum: dict, enums: set[str]) -> str:
    """
    Generates a JavaScript enum-like object definition from the provided enum metadata.

    Args:
        enum (dict): The JSON dictionary describing the enum.
        enums (set): A set to track already defined enums to prevent duplicates.

    Returns:
        str: The generated JavaScript enum-like object code or an empty string if the enum already exists.
    """
    # Extract enum name and values
    enum_name = enum.get('name', 'UnnamedEnum')
    enum_description = enum.get('description', '')
    enum_values = enum.get('values', [])

    # Check for duplicate enums
    if enum_name in enums:
        return ''  # Skip if already generated

    # Add the enum name to the set
    enums.add(enum_name)

    # Start building the enum-like object definition
    enum_code = []
    if enum_description:
        enum_code.append(f'  /**\n   * @enum {enum_name}\n   * {enum_description}\n   */')
    enum_code.append(f'  export const enum {enum_name} {{')

    # Iterate over the enum values and generate corresponding JavaScript entries
    for i, value in enumerate(enum_values):
        name = value.get('name', f'InvalidName_{i}')
        enum_value = value.get('value', str(i))
        description = value.get('description', '')

        # Add JSDoc comment for each value
        if description:
            enum_code.append(f'    /** {description} */')
        enum_code.append(f'    {name} = {enum_value},')

    # Close the enum-like object
    enum_code.append('  }')

    # Return the generated code as a string
    return '\n'.join(enum_code)


def gen_delegate_body(prototype: dict, delegates: set[str]) -> str:
    """
    Generates a JavaScript delegate definition from the provided prototype.
    """
    # Check for duplicate delegates
    delegate_name = prototype.get('name', 'UnnamedDelegate')
    delegate_description = prototype.get('description', '')

    # Check for duplicate delegates
    if delegate_name in delegates:
        return ''  # Skip if already generated

    # Add the delegate name to the set
    delegates.add(delegate_name)

    # Get the return type and convert it
    param_types = prototype.get('paramTypes', [])
    ret_type = prototype.get('retType', {})

    # Start building the delegate definition
    delegate_code = []
    if delegate_description:
        delegate_code.append(gen_documentation(prototype, tab_level=1))
    param_list = gen_params(param_types, ParamGen.TypesNames)
    delegate_code.append(f'  export type {delegate_name} = ({param_list}) => {gen_return(ret_type, param_types)};')

    # Join the list into a single formatted string
    return '\n'.join(delegate_code)


def generate_delegate_code(pplugin: dict, delegates: set[str]) -> str:
    """
    Generate C# delegate code from a plugin definition.
    """
    # Container for all generated delegate code
    content = []

    def process_prototype(prototype: dict):
        """
        Generate delegate code from the given prototype if it hasn't been processed.
        """
        delegate_code = gen_delegate_body(prototype, delegates)
        if delegate_code:
            content.append(delegate_code)

    # Main loop: Process all exported methods in the plugin
    for method in pplugin.get('methods', []):
        # Check the return type for a delegate
        ret_type = method.get('retType', {})
        if 'prototype' in ret_type:
            process_prototype(ret_type['prototype'])

        # Check parameters for delegates
        for param in method.get('paramTypes', []):
            if 'prototype' in param:
                process_prototype(param['prototype'])

    content.append('\n\n')

    # Join all generated delegates into a single string
    return '\n'.join(content)


def generate_enum_code(pplugin: dict, enums: set[str]) -> str:
    """
    Generate  JavaScript enum-like object code from a plugin definition.
    """
    # Container for all generated enum code
    content = []

    def process_enum(enum_data: dict):
        """
        Generate enum code from the given enum data if it hasn't been processed.
        """
        enum_code = gen_enum_body(enum_data, enums)
        if enum_code:
            content.append(enum_code)
            content.append('\n')

    def process_prototype(prototype: dict):
        """
        Recursively process a function prototype for enums.
        """
        if 'enum' in prototype.get('retType', {}):
            process_enum(prototype['retType']['enum'])

        for param in prototype.get('paramTypes', []):
            if 'enum' in param:
                process_enum(param['enum'])
            if 'prototype' in param:  # Process nested prototypes
                process_prototype(param['prototype'])

    # Main loop: Process all exported methods in the plugin
    for method in pplugin.get('methods', []):
        if 'retType' in method and 'enum' in method['retType']:
            process_enum(method['retType']['enum'])

        for param in method.get('paramTypes', []):
            if 'enum' in param:
                process_enum(param['enum'])
            if 'prototype' in param:  # Handle nested function prototypes
                process_prototype(param['prototype'])

    # Join all generated enums into a single string
    return '\n'.join(content)

    
def generate_stub(plugin_name: str, pplugin: dict) -> str:
    """Generate JavaScript stub content."""
    link = 'https://github.com/untrustedmodders/plugify-module-v8/blob/main/generator/generator.py'
    content = [
        f'// Generated from {plugin_name}.pplugin by {link}\n\n'
        'declare module "plugify" {\n'
        
        '  /**\n'
        '   * Represents a plugin with metadata information.\n'
        '   */\n'
        '  type Plugin = {\n'
        '    /** Unique identifier for the plugin */\n'
        '    id: bigint;\n'
        '    /** Short name of the plugin */\n'
        '    name: string;\n'
        '    /** Full name of the plugin */\n'
        '    fullName: string;\n'
        '    /** Description of the plugin */\n'
        '    description: string;\n'
        '    /** Version of the plugin */\n'
        '    version: string;\n'
        '    /** Author of the plugin */\n'
        '    author: string;\n'
        '    /** Website of the plugin */\n'
        '    website: string;\n'
        '    /** Directory where the plugin is stored */\n'
        '    baseDir: string;\n'
        '    /** Directory where the plugin\'s configs is stored */\n'
        '    configsDir: string;\n'
        '    /** Directory where the plugin\'s data is stored */\n'
        '    dataDir: string;\n'
        '    /** Directory where the plugin\'s logs is stored */\n'
        '    logsDir: string;\n'
        '    /** List of dependencies required by the plugin */\n'
        '    dependencies: string[];\n'
        '  };\n\n'
        
        '  /**\n'
        '   * Represents a 2D vector with basic mathematical operations.\n'
        '   */\n'
        '  type Vector2 = {\n'
        '    /** X-coordinate of the vector */\n'
        '    x: number;\n'
        '    /** Y-coordinate of the vector */\n'
        '    y: number;\n'
        '    /** Adds another Vector2 to this vector */\n'
        '    add: (vector: Vector2) => Vector2;\n'
        '    /** Subtracts another Vector2 from this vector */\n'
        '    subtract: (vector: Vector2) => Vector2;\n'
        '    /** Scales this vector by a scalar */\n'
        '    scale: (scalar: number) => Vector2;\n'
        '    /** Returns the magnitude (length) of the vector */\n'
        '    magnitude: () => number;\n'
        '    /** Returns a normalized (unit length) version of this vector */\n'
        '    normalize: () => Vector2;\n'
        '    /** Returns the dot product with another Vector2 */\n'
        '    dot: (vector: Vector2) => number;\n'
        '    /** Computes the distance between this vector and another Vector2 */\n'
        '    distanceTo: (vector: Vector2) => number;\n'
        '  };\n\n'
        
        '  /**\n'
        '   * Represents a 3D vector with basic mathematical operations.\n'
        '   */\n'
        '  type Vector3 = {\n'
        '    /** X-coordinate of the vector */\n'
        '    x: number;\n'
        '    /** Y-coordinate of the vector */\n'
        '    y: number;\n'
        '    /** Z-coordinate of the vector */\n'
        '    z: number;\n'
        '    /** Adds another Vector3 to this vector */\n'
        '    add: (vector: Vector3) => Vector3;\n'
        '    /** Subtracts another Vector3 from this vector */\n'
        '    subtract: (vector: Vector3) => Vector3;\n'
        '    /** Scales this vector by a scalar */\n'
        '    scale: (scalar: number) => Vector3;\n'
        '    /** Returns the magnitude (length) of the vector */\n'
        '    magnitude: () => number;\n'
        '    /** Returns a normalized (unit length) version of this vector */\n'
        '    normalize: () => Vector3;\n'
        '    /** Returns the dot product with another Vector3 */\n'
        '    dot: (vector: Vector3) => number;\n'
        '    /** Computes the cross product with another Vector3 */\n'
        '    cross: (vector: Vector3) => Vector3;\n'
        '    /** Computes the distance between this vector and another Vector3 */\n'
        '    distanceTo: (vector: Vector3) => number;\n'
        '  };\n\n'
        
        '  /**\n'
        '   * Represents a 4D vector with basic mathematical operations.\n'
        '   */\n'
        '  type Vector4 = {\n'
        '    /** X-coordinate of the vector */\n'
        '    x: number;\n'
        '    /** Y-coordinate of the vector */\n'
        '    y: number;\n'
        '    /** Z-coordinate of the vector */\n'
        '    z: number;\n'
        '    /** W-coordinate of the vector */\n'
        '    w: number;\n'
        '    /** Adds another Vector4 to this vector */\n'
        '    add: (vector: Vector4) => Vector4;\n'
        '    /** Subtracts another Vector4 from this vector */\n'
        '    subtract: (vector: Vector4) => Vector4;\n'
        '    /** Scales this vector by a scalar */\n'
        '    scale: (scalar: number) => Vector4;\n'
        '    /** Returns the magnitude (length) of the vector */\n'
        '    magnitude: () => number;\n'
        '    /** Returns a normalized (unit length) version of this vector */\n'
        '    normalize: () => Vector4;\n'
        '    /** Returns the dot product with another Vector4 */\n'
        '    dot: (vector: Vector4) => number;\n'
        '    /** Computes the distance between this vector and another Vector4 */\n'
        '    distanceTo: (vector: Vector4) => number;\n'
        '  };\n\n'
        
        '  /**\n'
        '   * Represents a 4x4 matrix with operations for transformations.\n'
        '   */\n'
        '  type Matrix4x4 = {\n'
        '    /** Elements stored as a 2D array */\n'
        '    elements: number[][];\n'
        '    /** Adds another matrix to this matrix */\n'
        '    add: (matrix: Matrix4x4) => Matrix4x4;\n'
        '    /** Subtracts another matrix from this matrix */\n'
        '    subtract: (matrix: Matrix4x4) => Matrix4x4;\n'
        '    /** Multiplies this matrix with another matrix */\n'
        '    multiply: (matrix: Matrix4x4) => Matrix4x4;\n'
        '    /** Multiplies this matrix with a Vector4 */\n'
        '    multiplyVector: (vector: Vector4) => Vector4;\n'
        '    /** Returns the transpose of this matrix */\n'
        '    transpose: () => Matrix4x4;\n'
        '  };\n'
        '}\n\n'
        
        f'declare module ":{plugin_name}" {{\n'
        '  import { Vector2, Vector3, Vector4, Matrix4x4 } from "plugify";\n\n'
    ]

    # Append enum definitions
    enums = set()
    content.append(generate_enum_code(pplugin, enums))

    # Append delegate definitions
    delegates = set()
    content.append(generate_delegate_code(pplugin, delegates))

    # Append method stubs
    for method in pplugin.get('methods', []):
        method_name = method.get('name', 'UnnamedMethod')
        param_types = method.get('paramTypes', [])
        ret_type = method.get('retType', {})

        # Add the method signature and documentation
        content.append(gen_documentation(method, tab_level=1))  # Use JS-specific doc generator
        content.append(f'  export function {method_name}({gen_params(param_types, ParamGen.TypesNames)}): {gen_return(ret_type, param_types)};')

    content.append('}')

    return '\n'.join(content)


def main(manifest_path: str, output_dir: str, override: bool):
    """Main entry point for the script."""
    if not os.path.isfile(manifest_path):
        print(f'Manifest file does not exist: {manifest_path}')
        return 1
    if not os.path.isdir(output_dir):
        print(f'Output directory does not exist: {output_dir}')
        return 1

    try:
        with open(manifest_path, 'r', encoding='utf-8') as file:
            pplugin = json.load(file)

    except Exception as e:
        print(f'An error occurred: {e}')
        return 1

    plugin_name = pplugin.get('name', os.path.basename(manifest_path).rsplit('.', 3)[0])
    output_path = os.path.join(output_dir, f'{plugin_name}.d.ts')
    os.makedirs(os.path.dirname(output_path), exist_ok=True)

    if os.path.isfile(output_path) and not override:
        print(f'Output file already exists: {output_path}. Use --override to overwrite existing file.')
        return 1

    try:
        content = generate_stub(plugin_name, pplugin)
        
        with open(output_path, 'w', encoding='utf-8') as file:
            file.write(content)

    except Exception as e:
        print(f'An error occurred: {e}')
        return 1
    
    print(f'Stub generated at: {output_path}')
    return 0


def get_args():
    """Parse command-line arguments."""
    parser = argparse.ArgumentParser(description='Generate Javascript .mjs stub files for plugin manifests.')
    parser.add_argument('manifest', help='Path to the plugin manifest file')
    parser.add_argument('output', help='Output directory for the generated stub')
    parser.add_argument('--override', action='store_true', help='Override existing files')
    return parser.parse_args()


if __name__ == '__main__':
    args = get_args()
    sys.exit(main(args.manifest, args.output, args.override))