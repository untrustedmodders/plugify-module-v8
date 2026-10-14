#!/bin/bash
# build.sh - For Linux builds

set -ex

LIB_EXT="so"

mkdir -p $PREFIX/bin
mkdir -p $PREFIX/lib
mkdir -p $PREFIX/v8

cp bin/libplugify-module-v8.$LIB_EXT $PREFIX/bin/
cp -r lib/* $PREFIX/lib/
cp -r v8/* $PREFIX/v8/
cp plugify-module-v8.pmodule $PREFIX/

chmod 755 $PREFIX/bin/libplugify-module-v8.$LIB_EXT
chmod -R 755 $PREFIX/lib
chmod -R 755 $PREFIX/v8
chmod 644 $PREFIX/plugify-module-v8.pmodule

# Create activation scripts for proper library path
mkdir -p $PREFIX/etc/conda/activate.d
mkdir -p $PREFIX/etc/conda/deactivate.d

cat > $PREFIX/etc/conda/activate.d/plugify-module-v8.sh << EOF2
#!/bin/bash
export PLUGIFY_V8_MODULE_PATH="\${CONDA_PREFIX}:\${PLUGIFY_V8_MODULE_PATH}"
EOF2

cat > $PREFIX/etc/conda/deactivate.d/plugify-module-v8.sh << EOF2
#!/bin/bash
export PLUGIFY_V8_MODULE_PATH="\${PLUGIFY_V8_MODULE_PATH//\${CONDA_PREFIX}:/}"
EOF2

chmod +x $PREFIX/etc/conda/activate.d/plugify-module-v8.sh
chmod +x $PREFIX/etc/conda/deactivate.d/plugify-module-v8.sh
