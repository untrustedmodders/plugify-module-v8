export class Plugin {
    constructor(id, name, fullName, description, version, author, website, baseDir, configsDir, dataDir, logsDir, dependencies) {
        this.id = id;
        this.name = name;
        this.fullName = fullName;
        this.description = description;
        this.version = version;
        this.author = author;
        this.website = website;
        this.baseDir = baseDir;
        this.configsDir = configsDir;
        this.dataDir = dataDir;
        this.logsDir = logsDir;
        this.dependencies = dependencies;
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
    return this.m
      .map(row => `[${row.join(", ")}]`)
      .join("\n");
  }
}
