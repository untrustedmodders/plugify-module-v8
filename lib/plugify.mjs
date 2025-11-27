export class Plugin {
    constructor(id, name, description, version, author, website, license, location, dependencies, base_dir, extensions_dir, configs_dir, data_dir, logs_dir, cache_dir) {
        this.id = id;
        this.name = name;
        this.description = description;
        this.version = version;
        this.author = author;
        this.website = website;
        this.license = license;
        this.location = location;

        this.base_dir = base_dir
        this.extensions_dir = extensions_dir
        this.configs_dir = configs_dir
        this.data_dir = data_dir
        this.logs_dir = logs_dir
        this.cache_dir = cache_dir
    }
}

export class Vector2 {
  constructor(x = 0, y = 0) {
    this.x = x;
    this.y = y;
  }

  // Add another vector
  add(vector) {
    return new Vector2(this.x + vector.x, this.y + vector.y);
  }

  // Subtract another vector
  subtract(vector) {
    return new Vector2(this.x - vector.x, this.y - vector.y);
  }

  // Scale by a scalar
  scale(scalar) {
    return new Vector2(this.x * scalar, this.y * scalar);
  }

  // Calculate the magnitude (length) of the vector
  magnitude() {
    return Math.sqrt(this.x ** 2 + this.y ** 2);
  }

  // Normalize the vector to a unit vector
  normalize() {
    const magnitude = this.magnitude();
    return magnitude === 0 ? new Vector2(0, 0) : this.scale(1 / magnitude);
  }

  // Dot product with another vector
  dot(vector) {
    return this.x * vector.x + this.y * vector.y;
  }

  // Calculate the distance to another vector
  distanceTo(vector) {
    return Math.sqrt((this.x - vector.x) ** 2 + (this.y - vector.y) ** 2);
  }

  // Return a string representation
  toString() {
    return `Vector2(${this.x}, ${this.y})`;
  }

  // Static method to create a zero vector
  static zero() {
    return new Vector2(0, 0);
  }

  // Static method to create a unit vector
  static unit() {
    return new Vector2(1, 1);
  }
}

export class Vector3 {
  constructor(x = 0, y = 0, z = 0) {
    this.x = x;
    this.y = y;
    this.z = z;
  }

  // Add another vector
  add(vector) {
    return new Vector3(this.x + vector.x, this.y + vector.y, this.z + vector.z);
  }

  // Subtract another vector
  subtract(vector) {
    return new Vector3(this.x - vector.x, this.y - vector.y, this.z - vector.z);
  }

  // Scale by a scalar
  scale(scalar) {
    return new Vector3(this.x * scalar, this.y * scalar, this.z * scalar);
  }

  // Calculate the magnitude (length) of the vector
  magnitude() {
    return Math.sqrt(this.x ** 2 + this.y ** 2 + this.z ** 2);
  }

  // Normalize the vector to a unit vector
  normalize() {
    const magnitude = this.magnitude();
    return magnitude === 0 ? new Vector3(0, 0, 0) : this.scale(1 / magnitude);
  }

  // Dot product with another vector
  dot(vector) {
    return this.x * vector.x + this.y * vector.y + this.z * vector.z;
  }

  // Cross product with another vector
  cross(vector) {
    return new Vector3(
      this.y * vector.z - this.z * vector.y,
      this.z * vector.x - this.x * vector.z,
      this.x * vector.y - this.y * vector.x
    );
  }

  // Calculate the distance to another vector
  distanceTo(vector) {
    return Math.sqrt(
      (this.x - vector.x) ** 2 +
      (this.y - vector.y) ** 2 +
      (this.z - vector.z) ** 2
    );
  }

  // Return a string representation
  toString() {
    return `Vector3(${this.x}, ${this.y}, ${this.z})`;
  }

  // Static method to create a zero vector
  static zero() {
    return new Vector3(0, 0, 0);
  }

  // Static method to create a unit vector
  static unit() {
    return new Vector3(1, 1, 1);
  }
}

export class Vector4 {
  constructor(x = 0, y = 0, z = 0, w = 0) {
    this.x = x;
    this.y = y;
    this.z = z;
    this.w = w;
  }

  // Add another vector
  add(vector) {
    return new Vector4(this.x + vector.x, this.y + vector.y, this.z + vector.z, this.w + vector.w);
  }

  // Subtract another vector
  subtract(vector) {
    return new Vector4(this.x - vector.x, this.y - vector.y, this.z - vector.z, this.w - vector.w);
  }

  // Scale by a scalar
  scale(scalar) {
    return new Vector4(this.x * scalar, this.y * scalar, this.z * scalar, this.w * scalar);
  }

  // Calculate the magnitude (length) of the vector
  magnitude() {
    return Math.sqrt(this.x ** 2 + this.y ** 2 + this.z ** 2 + this.w ** 2);
  }

  // Normalize the vector to a unit vector
  normalize() {
    const magnitude = this.magnitude();
    return magnitude === 0 ? new Vector4(0, 0, 0, 0) : this.scale(1 / magnitude);
  }

  // Dot product with another vector
  dot(vector) {
    return this.x * vector.x + this.y * vector.y + this.z * vector.z + this.w * vector.w;
  }

  // Calculate the distance to another vector
  distanceTo(vector) {
    return Math.sqrt(
      (this.x - vector.x) ** 2 +
      (this.y - vector.y) ** 2 +
      (this.z - vector.z) ** 2 +
      (this.w - vector.w) ** 2
    );
  }

  // Return a string representation
  toString() {
    return `Vector4(${this.x}, ${this.y}, ${this.z}, ${this.w})`;
  }

  // Static method to create a zero vector
  static zero() {
    return new Vector4(0, 0, 0, 0);
  }

  // Static method to create a unit vector
  static unit() {
    return new Vector4(1, 1, 1, 1);
  }
}

export class Matrix4x4 {
  constructor(m = null) {
    if (m && Array.isArray(m)) {
      if (m.length === 16) {
        // Convert 16-length array into a 4x4 matrix
        this.m = [
          m.slice(0, 4),
          m.slice(4, 8),
          m.slice(8, 12),
          m.slice(12, 16),
        ];
      } else if (
        m.length === 4 &&
        m.every((row) => Array.isArray(row) && row.length === 4)
      ) {
        // Already a 4x4 matrix
        this.m = m;
      } else {
        throw new Error("Invalid m. Must be a 4x4 matrix or an array of 16 numbers.");
      }
    } else {
      // Default to identity matrix
      this.m = [
        [1, 0, 0, 0],
        [0, 1, 0, 0],
        [0, 0, 1, 0],
        [0, 0, 0, 1],
      ];
    }
  }

  // Add another matrix
  add(matrix) {
    const result = this.m.map((row, rowIndex) =>
      row.map((value, colIndex) => value + matrix.m[rowIndex][colIndex])
    );
    return new Matrix4x4(result);
  }

  // Subtract another matrix
  subtract(matrix) {
    const result = this.m.map((row, rowIndex) =>
      row.map((value, colIndex) => value - matrix.m[rowIndex][colIndex])
    );
    return new Matrix4x4(result);
  }

  // Multiply by another matrix
  multiply(matrix) {
    const result = Array.from({ length: 4 }, () => Array(4).fill(0));
    for (let row = 0; row < 4; row++) {
      for (let col = 0; col < 4; col++) {
        for (let k = 0; k < 4; k++) {
          result[row][col] += this.m[row][k] * matrix.m[k][col];
        }
      }
    }
    return new Matrix4x4(result);
  }

  // Multiply by a vector (Vector4)
  multiplyVector(vector) {
    const [x, y, z, w] = [
      this.m[0][0] * vector.x + this.m[0][1] * vector.y + this.m[0][2] * vector.z + this.m[0][3] * vector.w,
      this.m[1][0] * vector.x + this.m[1][1] * vector.y + this.m[1][2] * vector.z + this.m[1][3] * vector.w,
      this.m[2][0] * vector.x + this.m[2][1] * vector.y + this.m[2][2] * vector.z + this.m[2][3] * vector.w,
      this.m[3][0] * vector.x + this.m[3][1] * vector.y + this.m[3][2] * vector.z + this.m[3][3] * vector.w
    ];
    return new Vector4(x, y, z, w);
  }

  // Transpose the matrix
  transpose() {
    const result = Array.from({ length: 4 }, () => Array(4).fill(0));
    for (let row = 0; row < 4; row++) {
      for (let col = 0; col < 4; col++) {
        result[col][row] = this.m[row][col];
      }
    }
    return new Matrix4x4(result);
  }

  // Get the identity matrix
  static identity() {
    return new Matrix4x4([
      [1, 0, 0, 0],
      [0, 1, 0, 0],
      [0, 0, 1, 0],
      [0, 0, 0, 1]
    ]);
  }
  
   // Create a zero matrix
  static zero() {
    return new Matrix4x4([
      [0, 0, 0, 0],
      [0, 0, 0, 0],
      [0, 0, 0, 0],
      [0, 0, 0, 0],
    ]);
  }

  // Create a scaling matrix
  static scaling(sx, sy, sz) {
    return new Matrix4x4([
      [sx, 0, 0, 0],
      [0, sy, 0, 0],
      [0, 0, sz, 0],
      [0, 0, 0, 1]
    ]);
  }

  // Create a translation matrix
  static translation(tx, ty, tz) {
    return new Matrix4x4([
      [1, 0, 0, tx],
      [0, 1, 0, ty],
      [0, 0, 1, tz],
      [0, 0, 0, 1]
    ]);
  }

  // Create a rotation matrix around the X-axis
  static rotationX(angle) {
    const c = Math.cos(angle);
    const s = Math.sin(angle);
    return new Matrix4x4([
      [1, 0, 0, 0],
      [0, c, -s, 0],
      [0, s, c, 0],
      [0, 0, 0, 1]
    ]);
  }

  // Create a rotation matrix around the Y-axis
  static rotationY(angle) {
    const c = Math.cos(angle);
    const s = Math.sin(angle);
    return new Matrix4x4([
      [c, 0, s, 0],
      [0, 1, 0, 0],
      [-s, 0, c, 0],
      [0, 0, 0, 1]
    ]);
  }

  // Create a rotation matrix around the Z-axis
  static rotationZ(angle) {
    const c = Math.cos(angle);
    const s = Math.sin(angle);
    return new Matrix4x4([
      [c, -s, 0, 0],
      [s, c, 0, 0],
      [0, 0, 1, 0],
      [0, 0, 0, 1]
    ]);
  }

  // Print matrix as a formatted string
  toString() {
    const rows = this.m
      .map(r => `[${r.join(", ")}]`)
      .join(", ");
    return `Matrix4x4(${rows})`;
  }
}

const makeEnum = def => {
    const tag = Symbol(), E = { is: v => v?.[tag] };
    for (const k in def)
        E[k] = Object.freeze({ value: def[k], [tag]: true, valueOf() { return this.value; } });
    return Object.freeze(E);
};

// Ownership enum equivalent
const Ownership = makeEnum({
    OWNED: true,
    BORROWED: false
});

// Global finalizer registry for cleanup
const finalizerRegistry = new FinalizationRegistry((heldValue) => {
    const { handle, owned, destructor, invalidValue } = heldValue;
    if (handle !== invalidValue && owned === Ownership.OWNED) {
        destructor(handle);
    }
});

const classRegistry = {};

export function bindClassMethods(
    cls,
    constructors,
    destructor,
    methods,
    invalidValue = 0
) {
    const className = cls.name;

    // Store original constructor
    const OriginalClass = cls;

    // Create new constructor
    const EnhancedClass = function(...args) {
        // Initialize to invalid state
        this._handle = invalidValue;
        this._owned = Ownership.BORROWED;

        // Check if this is handle + ownership construction
        // Pattern: new ClassName(handle_value, Ownership.OWNED/BORROWED)
        if (args.length >= 2 && Ownership.is(args[1])) {
            this._handle = args[0];
            this._owned = args[1];

            // Register for finalization if owned
            if (this._owned === Ownership.OWNED && destructor) {
                finalizerRegistry.register(this, {
                    handle: this._handle,
                    owned: this._owned,
                    destructor: destructor,
                    invalidValue: invalidValue
                }, this);
            }
            return;
        }

        // Constructor call mode
        if (constructors.length === 0) {
            throw new Error(`${className} requires handle and ownership for construction`);
        }

        // Try constructors
        let lastError = null;
        for (const constructor of constructors) {
            try {
                this._handle = constructor(...args);
                this._owned = Ownership.OWNED;

                // Register for finalization
                if (destructor) {
                    finalizerRegistry.register(this, {
                        handle: this._handle,
                        owned: this._owned,
                        destructor: destructor,
                        invalidValue: invalidValue
                    }, this);
                }
                return; // Success!
            } catch (e) {
                lastError = e;
                continue;
            }
        }

        // If we get here, all constructors failed
        if (lastError) {
            throw lastError;
        } else {
            throw new Error(`No constructor matched the arguments for ${className}`);
        }
    };

    // Copy prototype and static properties
    EnhancedClass.prototype = Object.create(OriginalClass.prototype);
    EnhancedClass.prototype.constructor = EnhancedClass;
    Object.setPrototypeOf(EnhancedClass, OriginalClass);

    // Add close method
    EnhancedClass.prototype.close = function() {
        if (this._handle !== invalidValue && this._owned === Ownership.OWNED) {
            if (destructor) {
                destructor(this._handle);
            }
            // Unregister from finalizer
            finalizerRegistry.unregister(this);
        }
        this._handle = invalidValue;
        this._owned = Ownership.BORROWED;
    };

    // Add release method
    EnhancedClass.prototype.release = function() {
        if (this._owned === Ownership.OWNED) {
            finalizerRegistry.unregister(this);
        }
        const tmp = this._handle;
        this._handle = invalidValue;
        this._owned = Ownership.BORROWED;
        return tmp;
    };

    // Add reset method
    EnhancedClass.prototype.reset = function() {
        this.close();
    };

    // Add get method
    EnhancedClass.prototype.get = function() {
        return this._handle;
    };

    // Add valid method
    EnhancedClass.prototype.valid = function() {
        return this._handle !== invalidValue;
    };

    classRegistry[className] = EnhancedClass;

    // Helper to handle return value wrapping
    function wrapReturn(result, retAlias, invalidValue) {
        if (retAlias && retAlias.length >= 2) {
            const [retClassName, owner] = retAlias;
            const ownership = owner ? Ownership.OWNED : Ownership.BORROWED;

            let retClass = classRegistry[retClassName];
            if (!retClass) {
                // Try to find in global scope
                if (typeof globalThis[retClassName] !== 'undefined') {
                    retClass = globalThis[retClassName];
                    classRegistry[retClassName] = retClass;
                }
            }

            if (retClass && result !== invalidValue) {
                return new retClass(result, ownership);
            } else if (result === invalidValue) {
                return null;
            }
        }

        return result;
    }

    // Helper to process parameter aliases
    function processParamAliases(args, paramAliases) {
        const argsList = [...args];
        for (let i = 0; i < paramAliases.length && i < argsList.length; i++) {
            const aliasInfo = paramAliases[i];
            if (aliasInfo && aliasInfo.length >= 2 && argsList[i] != null) {
                const [aliasName, owner] = aliasInfo;

                if (aliasName && argsList[i] != null) {
                    const arg = argsList[i];
                    if (typeof arg.release === 'function' && typeof arg.get === 'function') {
                        if (owner) {
                            argsList[i] = arg.release();
                        } else {
                            argsList[i] = arg.get();
                        }
                    }
                }
            }
        }
        return argsList;
    }

    // Process methods
    for (const methodInfo of methods) {
        const [methodName, func, bindSelf, paramAliases, retAlias] = methodInfo;

        if (!bindSelf) {
            // Static method
            EnhancedClass[methodName] = function(...args) {
                const argsList = processParamAliases(args, paramAliases);
                const result = func(...argsList);
                return wrapReturn(result, retAlias, invalidValue);
            };
        } else {
            // Instance method
            EnhancedClass.prototype[methodName] = function(...args) {
                if (this._handle === invalidValue) {
                    throw new Error(`${className} handle is closed or not initialized`);
                }

                const argsList = processParamAliases(args, paramAliases);
                const result = func(this._handle, ...argsList);
                return wrapReturn(result, retAlias, invalidValue);
            };
        }
    }

    return EnhancedClass;
}