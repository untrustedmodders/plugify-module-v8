#!/usr/bin/python3
import sys
import argparse
import os
import json
from enum import IntEnum


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


def gen_params(params: list[dict]) -> str:
    """Generate JavaScript function parameters as strings."""
    def gen_param(index: int, param: dict) -> str:
        return generate_name(param.get("name", f"p{index}"))

    return ', '.join(gen_param(i, p) for i, p in enumerate(params))


def gen_documentation(method: dict) -> str:
    """
    Generate a JavaScript function documentation string (JSDoc) from a JSON block.

    Args:
        method (Dict[str, Any]): The input JSON data describing the function.

    Returns:
        str: The generated JSDoc documentation string.
    """
    # Extract general details
    name = method.get('name', 'UnnamedFunction')
    description = method.get('description', 'No description provided.')
    param_types = method.get('paramTypes', [])
    ret_type = method.get('retType', {}).get('type', 'void')

    # Start building the JSDoc comment
    docstring = [f"/**\n * {description}\n *\n"]

    # Add parameters
    for param in param_types:
        param_name = param.get('name', 'UnnamedParam')
        param_type = param.get('type', 'Any')
        param_desc = param.get('description', 'No description available.')
        docstring.append(f" * @param {{{param_type}}} {param_name} - {param_desc}\n")

    # Add return type
    if ret_type.lower() != 'void':
        ret_desc = method.get('retType', {}).get('description', 'No description available.')
        docstring.append(f" * @returns {{{ret_type}}} - {ret_desc}\n")

    # Add callback prototype if present
    for param in param_types:
        if param.get('type') == 'function' and 'prototype' in param:
            prototype = param['prototype']
            proto_name = prototype.get('name', 'UnnamedCallback')
            proto_desc = prototype.get('description', 'No description provided.')
            proto_params = prototype.get('paramTypes', [])
            proto_ret = prototype.get('retType', {})

            docstring.append(f" *\n * @callback {{{proto_name}}} {param.get('name', '')}\n *   {proto_desc}\n *\n")
            for proto_param in proto_params:
                p_name = proto_param.get('name', 'UnnamedParam')
                p_type = proto_param.get('type', 'Any')
                p_desc = proto_param.get('description', 'No description available.')
                docstring.append(f" *   @param {{{p_type}}} {p_name} - {p_desc}\n")

            if proto_ret:
                proto_ret_type = proto_ret.get('type', 'void')
                proto_ret_desc = proto_ret.get('description', 'No description available.')
                docstring.append(f" *   @returns {{{proto_ret_type}}} - {proto_ret_desc}\n")

    # Close JSDoc comment
    docstring.append(" */")

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
    enum_name = enum.get('name', 'InvalidEnum')
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
        enum_code.append(f"/**\n * @enum {enum_name}\n * @description {enum_description}\n */")
    enum_code.append(f"const {enum_name} = {{")

    # Iterate over the enum values and generate corresponding JavaScript entries
    for i, value in enumerate(enum_values):
        name = value.get('name', f'InvalidName_{i}')
        enum_value = value.get('value', str(i))
        description = value.get('description', '')

        # Add JSDoc comment for each value
        if description:
            enum_code.append(f"  /** {description} */")
        enum_code.append(f"  {name}: {enum_value},")

    # Close the enum-like object
    enum_code.append("};")

    # Return the generated code as a string
    return '\n'.join(enum_code)


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
    for method in pplugin.get('exportedMethods', []):
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
        f'# Generated from {plugin_name}.pplugin by {link}\n\n']

    # Append enum definitions
    enums = set()
    content.append(generate_enum_code(pplugin, enums))

    # Append method stubs
    for method in pplugin.get('exportedMethods', []):
        method_name = method.get('name', 'UnnamedMethod')
        param_types = method.get('paramTypes', [])
        ret_type = method.get('retType', {})

        # Add the method signature and documentation
        content.append(gen_documentation(method))  # Use JS-specific doc generator
        content.append(f'function {method_name}({gen_params(param_types)}) {{')
        content.append('  // ...')
        content.append('}\n')

    return '\n'.join(content)


def main(manifest_path: str, output_dir: str, override: bool):
    """Main entry point for the script."""
    if not os.path.isfile(manifest_path):
        print(f'Manifest file does not exist: {manifest_path}')
        return 1
    if not os.path.isdir(output_dir):
        print(f'Output directory does not exist: {output_dir}')
        return 1

    plugin_name = os.path.basename(manifest_path).rsplit('.', 3)[0]
    output_path = os.path.join(output_dir, 'pps', f'{plugin_name}.mjs')
    os.makedirs(os.path.dirname(output_path), exist_ok=True)

    if os.path.isfile(output_path) and not override:
        print(f'Output file already exists: {output_path}. Use --override to overwrite existing file.')
        return 1

    try:
        with open(manifest_path, 'r', encoding='utf-8') as file:
            pplugin = json.load(file)

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