#!/bin/bash
# build.sh - For Linux builds

set -ex

# Create the target directories
mkdir -p $PREFIX/bin
mkdir -p $PREFIX/lib
mkdir -p $PREFIX

# Copy the shared library and module file
cp bin/libplugify-module-v8.so $PREFIX/bin/
cp -r lib/* $PREFIX/lib/
cp plugify-module-v8.pmodule $PREFIX/

# Set proper permissions
chmod 755 $PREFIX/bin/libplugify-module-v8.so
chmod -R 755 $PREFIX/lib
chmod 644 $PREFIX/plugify-module-v8.pmodule

# Create activation scripts for proper library path
mkdir -p $PREFIX/etc/conda/activate.d
mkdir -p $PREFIX/etc/conda/deactivate.d

cat > $PREFIX/etc/conda/activate.d/plugify-module-v8.sh << EOF
#!/bin/bash
export PLUGIFY_V8_MODULE_PATH="\${CONDA_PREFIX}:\${PLUGIFY_V8_MODULE_PATH}"
EOF

cat > $PREFIX/etc/conda/deactivate.d/plugify-module-v8.sh << EOF
#!/bin/bash
export PLUGIFY_V8_MODULE_PATH="\${PLUGIFY_V8_MODULE_PATH//\${CONDA_PREFIX}:/}"
EOF

chmod +x $PREFIX/etc/conda/activate.d/plugify-module-v8.sh
chmod +x $PREFIX/etc/conda/deactivate.d/plugify-module-v8.sh