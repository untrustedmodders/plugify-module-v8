// Generated from cross_call_master.pplugin

declare module "plugify" {
    /** Represents a plugin with metadata information. */
    type Plugin = {
        /** Unique identifier for the plugin */
        id: bigint;
        /** Name of the plugin */
        name: string;
        /** Description of the plugin */
        description: string;
        /** Version of the plugin */
        version: string;
        /** Author of the plugin */
        author: string;
        /** Website of the plugin */
        website: string;
        /** Software license of the plugin */
        license: string;
        /** Installation location of the plugin */
        location: string;
        /** List of plugin dependencies */
        dependencies: string[];
        /** Base directory where plugin files reside */
        base_dir: string;
        /** Directory for plugin extensions */
        extensions_dir: string;
        /** Directory for configuration files */
        configs_dir: string;
        /** Directory for plugin data files */
        data_dir: string;
        /** Directory for log files */
        logs_dir: string;
        /** Directory for cached files */
        cache_dir: string;
    };

    /** Represents a 2D vector with mathematical operations. */
    export type Vector2 = {
        /** X-coordinate of the vector */
        x: number;
        /** Y-coordinate of the vector */
        y: number;
        /** Adds another Vector2 to this vector */
        add(vector: Vector2): Vector2;
        /** Subtracts another Vector2 from this vector */
        subtract(vector: Vector2): Vector2;
        /** Scales this vector by a scalar */
        scale(scalar: number): Vector2;
        /** Returns the magnitude (length) of the vector */
        magnitude(): number;
        /** Returns a normalized (unit length) version of this vector */
        normalize(): Vector2;
        /** Returns the dot product with another Vector2 */
        dot(vector: Vector2): number;
        /** Computes the distance between this vector and another Vector2 */
        distanceTo(vector: Vector2): number;
        /** Converts vector to string */
        toString(): string;
    };

    export namespace Vector2 {
        /** Returns a zero vector (0, 0). */
        function zero(): Vector2;
        /** Returns a unit vector (1, 1). */
        function unit(): Vector2;
    }

    /** Represents a 3D vector with mathematical operations. */
    export type Vector3 = {
        /** X-coordinate of the vector */
        x: number;
        /** Y-coordinate of the vector */
        y: number;
        /** Z-coordinate of the vector */
        z: number;
        /** Adds another Vector3 to this vector */
        add(vector: Vector3): Vector3;
        /** Subtracts another Vector3 from this vector */
        subtract(vector: Vector3): Vector3;
        /** Scales this vector by a scalar */
        scale(scalar: number): Vector3;
        /** Returns the magnitude (length) of the vector */
        magnitude(): number;
        /** Returns a normalized (unit length) version of this vector */
        normalize(): Vector3;
        /** Returns the dot product with another Vector3 */
        dot(vector: Vector3): number;
        /** Computes the cross product with another Vector3 */
        cross(vector: Vector3): Vector3;
        /** Computes the distance between this vector and another Vector3 */
        distanceTo(vector: Vector3): number;
        /** Converts vector to string */
        toString(): string;
    };

    export namespace Vector3 {
        /** Returns a zero vector (0, 0, 0). */
        function zero(): Vector3;
        /** Returns a unit vector (1, 1, 1). */
        function unit(): Vector3;
    }

    /** Represents a 4D vector with mathematical operations. */
    export type Vector4 = {
        /** X-coordinate of the vector */
        x: number;
        /** Y-coordinate of the vector */
        y: number;
        /** Z-coordinate of the vector */
        z: number;
        /** W-coordinate of the vector */
        w: number;
        /** Adds another Vector4 to this vector */
        add(vector: Vector4): Vector4;
        /** Subtracts another Vector4 from this vector */
        subtract(vector: Vector4): Vector4;
        /** Scales this vector by a scalar */
        scale(scalar: number): Vector4;
        /** Returns the magnitude (length) of the vector */
        magnitude(): number;
        /** Returns a normalized (unit length) version of this vector */
        normalize(): Vector4;
        /** Returns the dot product with another Vector4 */
        dot(vector: Vector4): number;
        /** Computes the distance between this vector and another Vector4 */
        distanceTo(vector: Vector4): number;
        /** Converts vector to string */
        toString(): string;
    };

    export namespace Vector4 {
        /** Returns a zero vector (0, 0, 0, 0). */
        function zero(): Vector4;
        /** Returns a unit vector (1, 1, 1, 1). */
        function unit(): Vector4;
    }

    /** Represents a 4x4 matrix with transformation operations. */
    export type Matrix4x4 = {
        /** Matrix elements stored as a 2D array */
        m: number[][];
        /** Adds another matrix to this matrix */
        add(matrix: Matrix4x4): Matrix4x4;
        /** Subtracts another matrix from this matrix */
        subtract(matrix: Matrix4x4): Matrix4x4;
        /** Multiplies this matrix with another matrix */
        multiply(matrix: Matrix4x4): Matrix4x4;
        /** Multiplies this matrix with a Vector4 */
        multiplyVector(vector: Vector4): Vector4;
        /** Returns the transpose of this matrix */
        transpose(): Matrix4x4;
        /** Returns a string representation of the matrix */
        toString(): string;
    };

    export namespace Matrix4x4 {
        /** Returns an identity matrix. */
        function identity(): Matrix4x4;
        /** Returns a zero matrix (all values set to 0). */
        function zero(): Matrix4x4;
        /** Creates a scaling matrix with scale factors for each axis. */
        function scaling(sx: number, sy: number, sz: number): Matrix4x4;
        /** Creates a translation matrix using given translation values. */
        function translation(tx: number, ty: number, tz: number): Matrix4x4;
        /** Creates a rotation matrix around the X-axis. */
        function rotationX(angle: number): Matrix4x4;
        /** Creates a rotation matrix around the Y-axis. */
        function rotationY(angle: number): Matrix4x4;
        /** Creates a rotation matrix around the Z-axis. */
        function rotationZ(angle: number): Matrix4x4;
    }
}

declare module ":cross_call_master" {
    import { Vector2, Vector3, Vector4, Matrix4x4 } from "plugify";

    export const enum Example {
        First = 1,
        Second = 2,
        Third = 3,
        Forth = 4
    }


    type AliasBool = boolean;

    type AliasChar8 = number;

    type AliasChar16 = number;

    type AliasInt8 = number;

    type AliasInt16 = number;

    type AliasInt32 = number;

    type AliasInt64 = number;

    type AliasPtr = bigint;

    type AliasFloat = number;

    type AliasDouble = number;

    type AliasString = string;

    type AliasAny = any;

    type AliasVec2 = Vector2;

    type AliasVec3 = Vector3;

    type AliasVec4 = Vector4;

    type AliasMat4x4 = Matrix4x4;

    type AliasBoolVector = boolean[];

    type AliasChar8Vector = number[];

    type AliasChar16Vector = number[];

    type AliasInt8Vector = number[];

    type AliasInt16Vector = number[];

    type AliasInt32Vector = number[];

    type AliasInt64Vector = number[];

    type AliasPtrVector = bigint[];

    type AliasFloatVector = number[];

    type AliasDoubleVector = number[];

    type AliasStringVector = string[];

    type AliasAnyVector = any[];

    type AliasVec2Vector = Vector2[];

    type AliasVec3Vector = Vector3[];

    type AliasVec4Vector = Vector4[];

    type AliasUInt8 = number;

    type AliasUInt16 = number;

    type AliasUInt32 = number;

    type AliasUInt64 = bigint;

    type AliasFunction = bigint;

    type AliasUInt8Vector = number[];

    type AliasUInt16Vector = number[];

    type AliasUInt32Vector = number[];

    type AliasUInt64Vector = bigint[];

    type AliasMat4x4Vector = Matrix4x4[];


    export type NoParamReturnFunctionCallbackFunc = () => number;

    export type FuncVoid = () => void;

    export type FuncBool = () => boolean;

    export type FuncChar8 = () => number;

    export type FuncChar16 = () => number;

    export type FuncInt8 = () => number;

    export type FuncInt16 = () => number;

    export type FuncInt32 = () => number;

    export type FuncInt64 = () => number;

    export type FuncUInt8 = () => number;

    export type FuncUInt16 = () => number;

    export type FuncUInt32 = () => number;

    export type FuncUInt64 = () => bigint;

    export type FuncPtr = () => bigint;

    export type FuncFloat = () => number;

    export type FuncDouble = () => number;

    export type FuncString = () => string;

    export type FuncAny = () => any;

    export type FuncFunction = () => FuncFunctionInner;

    export type FuncFunctionInner = () => void;

    export type FuncBoolVector = () => boolean[];

    export type FuncChar8Vector = () => number[];

    export type FuncChar16Vector = () => number[];

    export type FuncInt8Vector = () => number[];

    export type FuncInt16Vector = () => number[];

    export type FuncInt32Vector = () => number[];

    export type FuncInt64Vector = () => number[];

    export type FuncUInt8Vector = () => number[];

    export type FuncUInt16Vector = () => number[];

    export type FuncUInt32Vector = () => number[];

    export type FuncUInt64Vector = () => bigint[];

    export type FuncPtrVector = () => bigint[];

    export type FuncFloatVector = () => number[];

    export type FuncDoubleVector = () => number[];

    export type FuncStringVector = () => string[];

    export type FuncAnyVector = () => any[];

    export type FuncVec2Vector = () => Vector2[];

    export type FuncVec3Vector = () => Vector3[];

    export type FuncVec4Vector = () => Vector4[];

    export type FuncMat4x4Vector = () => Matrix4x4[];

    export type FuncVec2 = () => Vector2;

    export type FuncVec3 = () => Vector3;

    export type FuncVec4 = () => Vector4;

    export type FuncMat4x4 = () => Matrix4x4;

    export type FuncAliasBool = () => AliasBool;

    export type FuncAliasChar8 = () => AliasChar8;

    export type FuncAliasChar16 = () => AliasChar16;

    export type FuncAliasInt8 = () => AliasInt8;

    export type FuncAliasInt16 = () => AliasInt16;

    export type FuncAliasInt32 = () => AliasInt32;

    export type FuncAliasInt64 = () => AliasInt64;

    export type FuncAliasUInt8 = () => AliasUInt8;

    export type FuncAliasUInt16 = () => AliasUInt16;

    export type FuncAliasUInt32 = () => AliasUInt32;

    export type FuncAliasUInt64 = () => AliasUInt64;

    export type FuncAliasPtr = () => AliasPtr;

    export type FuncAliasFloat = () => AliasFloat;

    export type FuncAliasDouble = () => AliasDouble;

    export type FuncAliasString = () => AliasString;

    export type FuncAliasAny = () => AliasAny;

    export type FuncAliasFunction = () => AliasFunction;

    export type FuncAliasFunctionInner = () => void;

    export type FuncAliasBoolVector = () => AliasBoolVector;

    export type FuncAliasChar8Vector = () => AliasChar8Vector;

    export type FuncAliasChar16Vector = () => AliasChar16Vector;

    export type FuncAliasInt8Vector = () => AliasInt8Vector;

    export type FuncAliasInt16Vector = () => AliasInt16Vector;

    export type FuncAliasInt32Vector = () => AliasInt32Vector;

    export type FuncAliasInt64Vector = () => AliasInt64Vector;

    export type FuncAliasUInt8Vector = () => AliasUInt8Vector;

    export type FuncAliasUInt16Vector = () => AliasUInt16Vector;

    export type FuncAliasUInt32Vector = () => AliasUInt32Vector;

    export type FuncAliasUInt64Vector = () => AliasUInt64Vector;

    export type FuncAliasPtrVector = () => AliasPtrVector;

    export type FuncAliasFloatVector = () => AliasFloatVector;

    export type FuncAliasDoubleVector = () => AliasDoubleVector;

    export type FuncAliasStringVector = () => AliasStringVector;

    export type FuncAliasAnyVector = () => AliasAnyVector;

    export type FuncAliasVec2Vector = () => AliasVec2Vector;

    export type FuncAliasVec3Vector = () => AliasVec3Vector;

    export type FuncAliasVec4Vector = () => AliasVec4Vector;

    export type FuncAliasMat4x4Vector = () => AliasMat4x4Vector;

    export type FuncAliasVec2 = () => AliasVec2;

    export type FuncAliasVec3 = () => AliasVec3;

    export type FuncAliasVec4 = () => AliasVec4;

    export type FuncAliasMat4x4 = () => AliasMat4x4;

    export type FuncAliasAll = (aBool: AliasBool, aChar8: AliasChar8, aChar16: AliasChar16, aInt8: AliasInt8, aInt16: AliasInt16, aInt32: AliasInt32, aInt64: AliasInt64, aPtr: AliasPtr, aFloat: AliasFloat, aDouble: AliasDouble, aString: AliasString, aAny: AliasAny, aVec2: AliasVec2, aVec3: AliasVec3, aVec4: AliasVec4, aMat4x4: AliasMat4x4, aBoolVec: AliasBoolVector, aChar8Vec: AliasChar8Vector, aChar16Vec: AliasChar16Vector, aInt8Vec: AliasInt8Vector, aInt16Vec: AliasInt16Vector, aInt32Vec: AliasInt32Vector, aInt64Vec: AliasInt64Vector, aPtrVec: AliasPtrVector, aFloatVec: AliasFloatVector, aDoubleVec: AliasDoubleVector, aStringVec: AliasStringVector, aAnyVec: AliasAnyVector, aVec2Vec: AliasVec2Vector, aVec3Vec: AliasVec3Vector, aVec4Vec: AliasVec4Vector) => string;

    export type Func1 = (a: Vector3) => number;

    export type Func2 = (a: number, b: number) => number;

    export type Func3 = (a: bigint, b: Vector4, c: string) => void;

    export type Func4 = (a: boolean, b: number, c: number, d: Matrix4x4) => Vector4;

    export type Func5 = (a: number, b: Vector2, c: bigint, d: number, e: bigint[]) => boolean;

    export type Func6 = (a: string, b: number, c: number[], d: number, e: number[], f: bigint) => number;

    export type Func7 = (vecC: number[], u16: number, ch16: number, vecU32: number[], vec4: Vector4, b: boolean, u64: bigint) => number;

    export type Func8 = (vec3: Vector3, vecU32: number[], i16: number, b: boolean, vec4: Vector4, vecC16: number[], ch16: number, i32: number) => Matrix4x4;

    export type Func9 = (f: number, vec2: Vector2, vecI8: number[], u64: bigint, b: boolean, str: string, vec4: Vector4, i16: number, ptr: bigint) => void;

    export type Func10 = (vec4: Vector4, mat: Matrix4x4, vecU32: number[], u64: bigint, vecC: number[], i32: number, b: boolean, vec2: Vector2, i64: number, d: number) => number;

    export type Func11 = (vecB: boolean[], ch16: number, u8: number, d: number, vec3: Vector3, vecI8: number[], i64: number, u16: number, f: number, vec2: Vector2, u32: number) => bigint;

    export type Func12 = (ptr: bigint, vecD: number[], u32: number, d: number, b: boolean, i32: number, i8: number, u64: bigint, f: number, vecPtr: bigint[], i64: number, ch: number) => boolean;

    export type Func13 = (i64: number, vecC: number[], d: number, f: number, b: boolean[], vec4: Vector4, str: string, int32: number, vec3: Vector3, ptr: bigint, vec2: Vector2, arr: number[], i16: number) => string;

    export type Func14 = (vecC: number[], vecU32: number[], mat: Matrix4x4, b: boolean, ch16: number, i32: number, vecF: number[], u16: number, vecU8: number[], i8: number, vec3: Vector3, vec4: Vector4, d: number, ptr: bigint) => string[];

    export type Func15 = (vecI16: number[], mat: Matrix4x4, vec4: Vector4, ptr: bigint, u64: bigint, vecU32: number[], b: boolean, f: number, vecC16: number[], u8: number, i32: number, vec2: Vector2, u16: number, d: number, vecU8: number[]) => number;

    export type Func16 = (vecB: boolean[], i16: number, vecI8: number[], vec4: Vector4, mat: Matrix4x4, vec2: Vector2, vecU64: bigint[], vecC: number[], str: string, i64: number, vecU32: number[], vec3: Vector3, f: number, d: number, i8: number, u16: number) => bigint;

    export type Func17 = (i32: number) => [void, number];

    export type Func18 = (i8: number, i16: number) => [Vector2, number, number];

    export type Func19 = (u32: number, vec3: Vector3, vecU32: number[]) => [void, number, Vector3, number[]];

    export type Func20 = (ch16: number, vec4: Vector4, vecU64: bigint[], ch: number) => [number, number, Vector4, bigint[], number];

    export type Func21 = (mat: Matrix4x4, vecI32: number[], vec2: Vector2, b: boolean, extraParam: number) => [number, Matrix4x4, number[], Vector2, boolean, number];

    export type Func22 = (ptr64Ref: bigint, uint32Ref: number, vectorDoubleRef: number[], int16Ref: number, plgStringRef: string, plgVector4Ref: Vector4) => [bigint, bigint, number, number[], number, string, Vector4];

    export type Func23 = (uint64Ref: bigint, plgVector2Ref: Vector2, vectorInt16Ref: number[], char16Ref: number, floatRef: number, int8Ref: number, vectorUInt8Ref: number[]) => [void, bigint, Vector2, number[], number, number, number, number[]];

    export type Func24 = (vectorCharRef: number[], int64Ref: number, vectorUInt8Ref: number[], plgVector4Ref: Vector4, uint64Ref: bigint, vectorptr64Ref: bigint[], doubleRef: number, vectorptr64Ref2: bigint[]) => [Matrix4x4, number[], number, number[], Vector4, bigint, bigint[], number, bigint[]];

    export type Func25 = (int32Ref: number, vectorptr64Ref: bigint[], boolRef: boolean, uint8Ref: number, plgStringRef: string, plgVector3Ref: Vector3, int64Ref: number, plgVector4Ref: Vector4, uint16Ref: number) => [number, number, bigint[], boolean, number, string, Vector3, number, Vector4, number];

    export type Func26 = (char16Ref: number, plgVector2Ref: Vector2, plgMatrix4x4Ref: Matrix4x4, vectorFloatRef: number[], int16Ref: number, uint64Ref: bigint, uint32Ref: number, vectorUInt16Ref: number[], ptr64Ref: bigint, boolRef: boolean) => [number, number, Vector2, Matrix4x4, number[], number, bigint, number, number[], bigint, boolean];

    export type Func27 = (floatRef: number, plgVector3Ref: Vector3, ptr64Ref: bigint, plgVector2Ref: Vector2, vectorInt16Ref: number[], plgMatrix4x4Ref: Matrix4x4, boolRef: boolean, plgVector4Ref: Vector4, int8Ref: number, int32Ref: number, vectorUInt8Ref: number[]) => [number, number, Vector3, bigint, Vector2, number[], Matrix4x4, boolean, Vector4, number, number, number[]];

    export type Func28 = (ptr64Ref: bigint, uint16Ref: number, vectorUInt32Ref: number[], plgMatrix4x4Ref: Matrix4x4, floatRef: number, plgVector4Ref: Vector4, plgStringRef: string, vectorUInt64Ref: bigint[], int64Ref: number, boolRef: boolean, plgVector3Ref: Vector3, vectorFloatRef: number[]) => [string, bigint, number, number[], Matrix4x4, number, Vector4, string, bigint[], number, boolean, Vector3, number[]];

    export type Func29 = (plgVector4Ref: Vector4, int32Ref: number, vectorInt8Ref: number[], doubleRef: number, boolRef: boolean, int8Ref: number, vectorUInt16Ref: number[], floatRef: number, plgStringRef: string, plgMatrix4x4Ref: Matrix4x4, uint64Ref: bigint, plgVector3Ref: Vector3, vectorInt64Ref: number[]) => [string[], Vector4, number, number[], number, boolean, number, number[], number, string, Matrix4x4, bigint, Vector3, number[]];

    export type Func30 = (ptr64Ref: bigint, plgVector4Ref: Vector4, int64Ref: number, vectorUInt32Ref: number[], boolRef: boolean, plgStringRef: string, plgVector3Ref: Vector3, vectorUInt8Ref: number[], floatRef: number, plgVector2Ref: Vector2, plgMatrix4x4Ref: Matrix4x4, int8Ref: number, vectorFloatRef: number[], doubleRef: number) => [number, bigint, Vector4, number, number[], boolean, string, Vector3, number[], number, Vector2, Matrix4x4, number, number[], number];

    export type Func31 = (charRef: number, uint32Ref: number, vectorUInt64Ref: bigint[], plgVector4Ref: Vector4, plgStringRef: string, boolRef: boolean, int64Ref: number, vec2Ref: Vector2, int8Ref: number, uint16Ref: number, vectorInt16Ref: number[], mat4x4Ref: Matrix4x4, vec3Ref: Vector3, floatRef: number, vectorDoubleRef: number[]) => [Vector3, number, number, bigint[], Vector4, string, boolean, number, Vector2, number, number, number[], Matrix4x4, Vector3, number, number[]];

    export type Func32 = (p1: number, p2: number, p3: number[], p4: Vector4, p5: bigint, p6: number[], p7: Matrix4x4, p8: bigint, p9: string, p10: number, p11: Vector2, p12: number[], p13: boolean, p14: Vector3, p15: number, p16: number[]) => [number, number, number, number[], Vector4, bigint, number[], Matrix4x4, bigint, string, number, Vector2, number[], boolean, Vector3, number, number[]];

    export type Func33 = (variant: any) => [void, any];

    export type FuncEnum = (p1: Example, p2: Example[]) => [Example[], Example[]];


    /**
     * @param returnString
     */
    export function ReverseReturn(returnString: string): void;

    /**
     */
    export function NoParamReturnVoidCallback(): void;

    /**
     */
    export function NoParamReturnBoolCallback(): boolean;

    /**
     */
    export function NoParamReturnChar8Callback(): number;

    /**
     */
    export function NoParamReturnChar16Callback(): number;

    /**
     */
    export function NoParamReturnInt8Callback(): number;

    /**
     */
    export function NoParamReturnInt16Callback(): number;

    /**
     */
    export function NoParamReturnInt32Callback(): number;

    /**
     */
    export function NoParamReturnInt64Callback(): number;

    /**
     */
    export function NoParamReturnUInt8Callback(): number;

    /**
     */
    export function NoParamReturnUInt16Callback(): number;

    /**
     */
    export function NoParamReturnUInt32Callback(): number;

    /**
     */
    export function NoParamReturnUInt64Callback(): bigint;

    /**
     */
    export function NoParamReturnPointerCallback(): bigint;

    /**
     */
    export function NoParamReturnFloatCallback(): number;

    /**
     */
    export function NoParamReturnDoubleCallback(): number;

    /**
     */
    export function NoParamReturnFunctionCallback(): NoParamReturnFunctionCallbackFunc;

    /**
     */
    export function NoParamReturnStringCallback(): string;

    /**
     */
    export function NoParamReturnAnyCallback(): any;

    /**
     */
    export function NoParamReturnArrayBoolCallback(): boolean[];

    /**
     */
    export function NoParamReturnArrayChar8Callback(): number[];

    /**
     */
    export function NoParamReturnArrayChar16Callback(): number[];

    /**
     */
    export function NoParamReturnArrayInt8Callback(): number[];

    /**
     */
    export function NoParamReturnArrayInt16Callback(): number[];

    /**
     */
    export function NoParamReturnArrayInt32Callback(): number[];

    /**
     */
    export function NoParamReturnArrayInt64Callback(): number[];

    /**
     */
    export function NoParamReturnArrayUInt8Callback(): number[];

    /**
     */
    export function NoParamReturnArrayUInt16Callback(): number[];

    /**
     */
    export function NoParamReturnArrayUInt32Callback(): number[];

    /**
     */
    export function NoParamReturnArrayUInt64Callback(): bigint[];

    /**
     */
    export function NoParamReturnArrayPointerCallback(): bigint[];

    /**
     */
    export function NoParamReturnArrayFloatCallback(): number[];

    /**
     */
    export function NoParamReturnArrayDoubleCallback(): number[];

    /**
     */
    export function NoParamReturnArrayStringCallback(): string[];

    /**
     */
    export function NoParamReturnArrayAnyCallback(): any[];

    /**
     */
    export function NoParamReturnArrayVector2Callback(): Vector2[];

    /**
     */
    export function NoParamReturnArrayVector3Callback(): Vector3[];

    /**
     */
    export function NoParamReturnArrayVector4Callback(): Vector4[];

    /**
     */
    export function NoParamReturnArrayMatrix4x4Callback(): Matrix4x4[];

    /**
     */
    export function NoParamReturnVector2Callback(): Vector2;

    /**
     */
    export function NoParamReturnVector3Callback(): Vector3;

    /**
     */
    export function NoParamReturnVector4Callback(): Vector4;

    /**
     */
    export function NoParamReturnMatrix4x4Callback(): Matrix4x4;

    /**
     * @param a
     */
    export function Param1Callback(a: number): void;

    /**
     * @param a
     * @param b
     */
    export function Param2Callback(a: number, b: number): void;

    /**
     * @param a
     * @param b
     * @param c
     */
    export function Param3Callback(a: number, b: number, c: number): void;

    /**
     * @param a
     * @param b
     * @param c
     * @param d
     */
    export function Param4Callback(a: number, b: number, c: number, d: Vector4): void;

    /**
     * @param a
     * @param b
     * @param c
     * @param d
     * @param e
     */
    export function Param5Callback(a: number, b: number, c: number, d: Vector4, e: number[]): void;

    /**
     * @param a
     * @param b
     * @param c
     * @param d
     * @param e
     * @param f
     */
    export function Param6Callback(a: number, b: number, c: number, d: Vector4, e: number[], f: number): void;

    /**
     * @param a
     * @param b
     * @param c
     * @param d
     * @param e
     * @param f
     * @param g
     */
    export function Param7Callback(a: number, b: number, c: number, d: Vector4, e: number[], f: number, g: string): void;

    /**
     * @param a
     * @param b
     * @param c
     * @param d
     * @param e
     * @param f
     * @param g
     * @param h
     */
    export function Param8Callback(a: number, b: number, c: number, d: Vector4, e: number[], f: number, g: string, h: number): void;

    /**
     * @param a
     * @param b
     * @param c
     * @param d
     * @param e
     * @param f
     * @param g
     * @param h
     * @param k
     */
    export function Param9Callback(a: number, b: number, c: number, d: Vector4, e: number[], f: number, g: string, h: number, k: number): void;

    /**
     * @param a
     * @param b
     * @param c
     * @param d
     * @param e
     * @param f
     * @param g
     * @param h
     * @param k
     * @param l
     */
    export function Param10Callback(a: number, b: number, c: number, d: Vector4, e: number[], f: number, g: string, h: number, k: number, l: bigint): void;

    /**
     * @param a
     */
    export function ParamRef1Callback(a: number): [void, number];

    /**
     * @param a
     * @param b
     */
    export function ParamRef2Callback(a: number, b: number): [void, number, number];

    /**
     * @param a
     * @param b
     * @param c
     */
    export function ParamRef3Callback(a: number, b: number, c: number): [void, number, number, number];

    /**
     * @param a
     * @param b
     * @param c
     * @param d
     */
    export function ParamRef4Callback(a: number, b: number, c: number, d: Vector4): [void, number, number, number, Vector4];

    /**
     * @param a
     * @param b
     * @param c
     * @param d
     * @param e
     */
    export function ParamRef5Callback(a: number, b: number, c: number, d: Vector4, e: number[]): [void, number, number, number, Vector4, number[]];

    /**
     * @param a
     * @param b
     * @param c
     * @param d
     * @param e
     * @param f
     */
    export function ParamRef6Callback(a: number, b: number, c: number, d: Vector4, e: number[], f: number): [void, number, number, number, Vector4, number[], number];

    /**
     * @param a
     * @param b
     * @param c
     * @param d
     * @param e
     * @param f
     * @param g
     */
    export function ParamRef7Callback(a: number, b: number, c: number, d: Vector4, e: number[], f: number, g: string): [void, number, number, number, Vector4, number[], number, string];

    /**
     * @param a
     * @param b
     * @param c
     * @param d
     * @param e
     * @param f
     * @param g
     * @param h
     */
    export function ParamRef8Callback(a: number, b: number, c: number, d: Vector4, e: number[], f: number, g: string, h: number): [void, number, number, number, Vector4, number[], number, string, number];

    /**
     * @param a
     * @param b
     * @param c
     * @param d
     * @param e
     * @param f
     * @param g
     * @param h
     * @param k
     */
    export function ParamRef9Callback(a: number, b: number, c: number, d: Vector4, e: number[], f: number, g: string, h: number, k: number): [void, number, number, number, Vector4, number[], number, string, number, number];

    /**
     * @param a
     * @param b
     * @param c
     * @param d
     * @param e
     * @param f
     * @param g
     * @param h
     * @param k
     * @param l
     */
    export function ParamRef10Callback(a: number, b: number, c: number, d: Vector4, e: number[], f: number, g: string, h: number, k: number, l: bigint): [void, number, number, number, Vector4, number[], number, string, number, number, bigint];

    /**
     * @param p1
     * @param p2
     * @param p3
     * @param p4
     * @param p5
     * @param p6
     * @param p7
     * @param p8
     * @param p9
     * @param p10
     * @param p11
     * @param p12
     * @param p13
     * @param p14
     * @param p15
     */
    export function ParamRefVectorsCallback(p1: boolean[], p2: number[], p3: number[], p4: number[], p5: number[], p6: number[], p7: number[], p8: number[], p9: number[], p10: number[], p11: bigint[], p12: bigint[], p13: number[], p14: number[], p15: string[]): [void, boolean[], number[], number[], number[], number[], number[], number[], number[], number[], number[], bigint[], bigint[], number[], number[], string[]];

    /**
     * @param p1
     * @param p2
     * @param p3
     * @param p4
     * @param p5
     * @param p6
     * @param p7
     * @param p8
     * @param p9
     * @param p10
     * @param p11
     * @param p12
     * @param p13
     * @param p14
     */
    export function ParamAllPrimitivesCallback(p1: boolean, p2: number, p3: number, p4: number, p5: number, p6: number, p7: number, p8: number, p9: number, p10: number, p11: bigint, p12: bigint, p13: number, p14: number): number;

    /**
     * @param aBool
     * @param aChar8
     * @param aChar16
     * @param aInt8
     * @param aInt16
     * @param aInt32
     * @param aInt64
     * @param aPtr
     * @param aFloat
     * @param aDouble
     * @param aString
     * @param aAny
     * @param aVec2
     * @param aVec3
     * @param aVec4
     * @param aMat4x4
     * @param aBoolVec
     * @param aChar8Vec
     * @param aChar16Vec
     * @param aInt8Vec
     * @param aInt16Vec
     * @param aInt32Vec
     * @param aInt64Vec
     * @param aPtrVec
     * @param aFloatVec
     * @param aDoubleVec
     * @param aStringVec
     * @param aAnyVec
     * @param aVec2Vec
     * @param aVec3Vec
     * @param aVec4Vec
     */
    export function ParamAllAliasesCallback(aBool: AliasBool, aChar8: AliasChar8, aChar16: AliasChar16, aInt8: AliasInt8, aInt16: AliasInt16, aInt32: AliasInt32, aInt64: AliasInt64, aPtr: AliasPtr, aFloat: AliasFloat, aDouble: AliasDouble, aString: AliasString, aAny: AliasAny, aVec2: AliasVec2, aVec3: AliasVec3, aVec4: AliasVec4, aMat4x4: AliasMat4x4, aBoolVec: AliasBoolVector, aChar8Vec: AliasChar8Vector, aChar16Vec: AliasChar16Vector, aInt8Vec: AliasInt8Vector, aInt16Vec: AliasInt16Vector, aInt32Vec: AliasInt32Vector, aInt64Vec: AliasInt64Vector, aPtrVec: AliasPtrVector, aFloatVec: AliasFloatVector, aDoubleVec: AliasDoubleVector, aStringVec: AliasStringVector, aAnyVec: AliasAnyVector, aVec2Vec: AliasVec2Vector, aVec3Vec: AliasVec3Vector, aVec4Vec: AliasVec4Vector): number;

    /**
     * @param aBool
     * @param aChar8
     * @param aChar16
     * @param aInt8
     * @param aInt16
     * @param aInt32
     * @param aInt64
     * @param aPtr
     * @param aFloat
     * @param aDouble
     * @param aString
     * @param aAny
     * @param aVec2
     * @param aVec3
     * @param aVec4
     * @param aMat4x4
     * @param aBoolVec
     * @param aChar8Vec
     * @param aChar16Vec
     * @param aInt8Vec
     * @param aInt16Vec
     * @param aInt32Vec
     * @param aInt64Vec
     * @param aPtrVec
     * @param aFloatVec
     * @param aDoubleVec
     * @param aStringVec
     * @param aAnyVec
     * @param aVec2Vec
     * @param aVec3Vec
     * @param aVec4Vec
     */
    export function ParamAllRefAliasesCallback(aBool: AliasBool, aChar8: AliasChar8, aChar16: AliasChar16, aInt8: AliasInt8, aInt16: AliasInt16, aInt32: AliasInt32, aInt64: AliasInt64, aPtr: AliasPtr, aFloat: AliasFloat, aDouble: AliasDouble, aString: AliasString, aAny: AliasAny, aVec2: AliasVec2, aVec3: AliasVec3, aVec4: AliasVec4, aMat4x4: AliasMat4x4, aBoolVec: AliasBoolVector, aChar8Vec: AliasChar8Vector, aChar16Vec: AliasChar16Vector, aInt8Vec: AliasInt8Vector, aInt16Vec: AliasInt16Vector, aInt32Vec: AliasInt32Vector, aInt64Vec: AliasInt64Vector, aPtrVec: AliasPtrVector, aFloatVec: AliasFloatVector, aDoubleVec: AliasDoubleVector, aStringVec: AliasStringVector, aAnyVec: AliasAnyVector, aVec2Vec: AliasVec2Vector, aVec3Vec: AliasVec3Vector, aVec4Vec: AliasVec4Vector): [number, AliasBool, AliasChar8, AliasChar16, AliasInt8, AliasInt16, AliasInt32, AliasInt64, AliasPtr, AliasFloat, AliasDouble, AliasString, AliasAny, AliasVec2, AliasVec3, AliasVec4, AliasMat4x4, AliasBoolVector, AliasChar8Vector, AliasChar16Vector, AliasInt8Vector, AliasInt16Vector, AliasInt32Vector, AliasInt64Vector, AliasPtrVector, AliasFloatVector, AliasDoubleVector, AliasStringVector, AliasAnyVector, AliasVec2Vector, AliasVec3Vector, AliasVec4Vector];

    /**
     * @param p1
     * @param p2
     */
    export function ParamEnumCallback(p1: Example, p2: Example[]): number;

    /**
     * @param p1
     * @param p2
     */
    export function ParamEnumRefCallback(p1: Example, p2: Example[]): [number, Example, Example[]];

    /**
     * @param p1
     * @param p2
     */
    export function ParamVariantCallback(p1: any, p2: any[]): void;

    /**
     * @param p1
     * @param p2
     */
    export function ParamVariantRefCallback(p1: any, p2: any[]): [void, any, any[]];

    /**
     * @param func
     */
    export function CallFuncVoidCallback(func: FuncVoid): void;

    /**
     * @param func
     */
    export function CallFuncBoolCallback(func: FuncBool): boolean;

    /**
     * @param func
     */
    export function CallFuncChar8Callback(func: FuncChar8): number;

    /**
     * @param func
     */
    export function CallFuncChar16Callback(func: FuncChar16): number;

    /**
     * @param func
     */
    export function CallFuncInt8Callback(func: FuncInt8): number;

    /**
     * @param func
     */
    export function CallFuncInt16Callback(func: FuncInt16): number;

    /**
     * @param func
     */
    export function CallFuncInt32Callback(func: FuncInt32): number;

    /**
     * @param func
     */
    export function CallFuncInt64Callback(func: FuncInt64): number;

    /**
     * @param func
     */
    export function CallFuncUInt8Callback(func: FuncUInt8): number;

    /**
     * @param func
     */
    export function CallFuncUInt16Callback(func: FuncUInt16): number;

    /**
     * @param func
     */
    export function CallFuncUInt32Callback(func: FuncUInt32): number;

    /**
     * @param func
     */
    export function CallFuncUInt64Callback(func: FuncUInt64): bigint;

    /**
     * @param func
     */
    export function CallFuncPtrCallback(func: FuncPtr): bigint;

    /**
     * @param func
     */
    export function CallFuncFloatCallback(func: FuncFloat): number;

    /**
     * @param func
     */
    export function CallFuncDoubleCallback(func: FuncDouble): number;

    /**
     * @param func
     */
    export function CallFuncStringCallback(func: FuncString): string;

    /**
     * @param func
     */
    export function CallFuncAnyCallback(func: FuncAny): any;

    /**
     * @param func
     */
    export function CallFuncFunctionCallback(func: FuncFunction): bigint;

    /**
     * @param func
     */
    export function CallFuncBoolVectorCallback(func: FuncBoolVector): boolean[];

    /**
     * @param func
     */
    export function CallFuncChar8VectorCallback(func: FuncChar8Vector): number[];

    /**
     * @param func
     */
    export function CallFuncChar16VectorCallback(func: FuncChar16Vector): number[];

    /**
     * @param func
     */
    export function CallFuncInt8VectorCallback(func: FuncInt8Vector): number[];

    /**
     * @param func
     */
    export function CallFuncInt16VectorCallback(func: FuncInt16Vector): number[];

    /**
     * @param func
     */
    export function CallFuncInt32VectorCallback(func: FuncInt32Vector): number[];

    /**
     * @param func
     */
    export function CallFuncInt64VectorCallback(func: FuncInt64Vector): number[];

    /**
     * @param func
     */
    export function CallFuncUInt8VectorCallback(func: FuncUInt8Vector): number[];

    /**
     * @param func
     */
    export function CallFuncUInt16VectorCallback(func: FuncUInt16Vector): number[];

    /**
     * @param func
     */
    export function CallFuncUInt32VectorCallback(func: FuncUInt32Vector): number[];

    /**
     * @param func
     */
    export function CallFuncUInt64VectorCallback(func: FuncUInt64Vector): bigint[];

    /**
     * @param func
     */
    export function CallFuncPtrVectorCallback(func: FuncPtrVector): bigint[];

    /**
     * @param func
     */
    export function CallFuncFloatVectorCallback(func: FuncFloatVector): number[];

    /**
     * @param func
     */
    export function CallFuncDoubleVectorCallback(func: FuncDoubleVector): number[];

    /**
     * @param func
     */
    export function CallFuncStringVectorCallback(func: FuncStringVector): string[];

    /**
     * @param func
     */
    export function CallFuncAnyVectorCallback(func: FuncAnyVector): any[];

    /**
     * @param func
     */
    export function CallFuncVec2VectorCallback(func: FuncVec2Vector): Vector2[];

    /**
     * @param func
     */
    export function CallFuncVec3VectorCallback(func: FuncVec3Vector): Vector3[];

    /**
     * @param func
     */
    export function CallFuncVec4VectorCallback(func: FuncVec4Vector): Vector4[];

    /**
     * @param func
     */
    export function CallFuncMat4x4VectorCallback(func: FuncMat4x4Vector): Matrix4x4[];

    /**
     * @param func
     */
    export function CallFuncVec2Callback(func: FuncVec2): Vector2;

    /**
     * @param func
     */
    export function CallFuncVec3Callback(func: FuncVec3): Vector3;

    /**
     * @param func
     */
    export function CallFuncVec4Callback(func: FuncVec4): Vector4;

    /**
     * @param func
     */
    export function CallFuncMat4x4Callback(func: FuncMat4x4): Matrix4x4;

    /**
     * @param func
     */
    export function CallFuncAliasBoolCallback(func: FuncAliasBool): AliasBool;

    /**
     * @param func
     */
    export function CallFuncAliasChar8Callback(func: FuncAliasChar8): AliasChar8;

    /**
     * @param func
     */
    export function CallFuncAliasChar16Callback(func: FuncAliasChar16): AliasChar16;

    /**
     * @param func
     */
    export function CallFuncAliasInt8Callback(func: FuncAliasInt8): AliasInt8;

    /**
     * @param func
     */
    export function CallFuncAliasInt16Callback(func: FuncAliasInt16): AliasInt16;

    /**
     * @param func
     */
    export function CallFuncAliasInt32Callback(func: FuncAliasInt32): AliasInt32;

    /**
     * @param func
     */
    export function CallFuncAliasInt64Callback(func: FuncAliasInt64): AliasInt64;

    /**
     * @param func
     */
    export function CallFuncAliasUInt8Callback(func: FuncAliasUInt8): AliasUInt8;

    /**
     * @param func
     */
    export function CallFuncAliasUInt16Callback(func: FuncAliasUInt16): AliasUInt16;

    /**
     * @param func
     */
    export function CallFuncAliasUInt32Callback(func: FuncAliasUInt32): AliasUInt32;

    /**
     * @param func
     */
    export function CallFuncAliasUInt64Callback(func: FuncAliasUInt64): AliasUInt64;

    /**
     * @param func
     */
    export function CallFuncAliasPtrCallback(func: FuncAliasPtr): AliasPtr;

    /**
     * @param func
     */
    export function CallFuncAliasFloatCallback(func: FuncAliasFloat): AliasFloat;

    /**
     * @param func
     */
    export function CallFuncAliasDoubleCallback(func: FuncAliasDouble): AliasDouble;

    /**
     * @param func
     */
    export function CallFuncAliasStringCallback(func: FuncAliasString): AliasString;

    /**
     * @param func
     */
    export function CallFuncAliasAnyCallback(func: FuncAliasAny): AliasAny;

    /**
     * @param func
     */
    export function CallFuncAliasFunctionCallback(func: FuncAliasFunction): AliasFunction;

    /**
     * @param func
     */
    export function CallFuncAliasBoolVectorCallback(func: FuncAliasBoolVector): AliasBoolVector;

    /**
     * @param func
     */
    export function CallFuncAliasChar8VectorCallback(func: FuncAliasChar8Vector): AliasChar8Vector;

    /**
     * @param func
     */
    export function CallFuncAliasChar16VectorCallback(func: FuncAliasChar16Vector): AliasChar16Vector;

    /**
     * @param func
     */
    export function CallFuncAliasInt8VectorCallback(func: FuncAliasInt8Vector): AliasInt8Vector;

    /**
     * @param func
     */
    export function CallFuncAliasInt16VectorCallback(func: FuncAliasInt16Vector): AliasInt16Vector;

    /**
     * @param func
     */
    export function CallFuncAliasInt32VectorCallback(func: FuncAliasInt32Vector): AliasInt32Vector;

    /**
     * @param func
     */
    export function CallFuncAliasInt64VectorCallback(func: FuncAliasInt64Vector): AliasInt64Vector;

    /**
     * @param func
     */
    export function CallFuncAliasUInt8VectorCallback(func: FuncAliasUInt8Vector): AliasUInt8Vector;

    /**
     * @param func
     */
    export function CallFuncAliasUInt16VectorCallback(func: FuncAliasUInt16Vector): AliasUInt16Vector;

    /**
     * @param func
     */
    export function CallFuncAliasUInt32VectorCallback(func: FuncAliasUInt32Vector): AliasUInt32Vector;

    /**
     * @param func
     */
    export function CallFuncAliasUInt64VectorCallback(func: FuncAliasUInt64Vector): AliasUInt64Vector;

    /**
     * @param func
     */
    export function CallFuncAliasPtrVectorCallback(func: FuncAliasPtrVector): AliasPtrVector;

    /**
     * @param func
     */
    export function CallFuncAliasFloatVectorCallback(func: FuncAliasFloatVector): AliasFloatVector;

    /**
     * @param func
     */
    export function CallFuncAliasDoubleVectorCallback(func: FuncAliasDoubleVector): AliasDoubleVector;

    /**
     * @param func
     */
    export function CallFuncAliasStringVectorCallback(func: FuncAliasStringVector): AliasStringVector;

    /**
     * @param func
     */
    export function CallFuncAliasAnyVectorCallback(func: FuncAliasAnyVector): AliasAnyVector;

    /**
     * @param func
     */
    export function CallFuncAliasVec2VectorCallback(func: FuncAliasVec2Vector): AliasVec2Vector;

    /**
     * @param func
     */
    export function CallFuncAliasVec3VectorCallback(func: FuncAliasVec3Vector): AliasVec3Vector;

    /**
     * @param func
     */
    export function CallFuncAliasVec4VectorCallback(func: FuncAliasVec4Vector): AliasVec4Vector;

    /**
     * @param func
     */
    export function CallFuncAliasMat4x4VectorCallback(func: FuncAliasMat4x4Vector): AliasMat4x4Vector;

    /**
     * @param func
     */
    export function CallFuncAliasVec2Callback(func: FuncAliasVec2): AliasVec2;

    /**
     * @param func
     */
    export function CallFuncAliasVec3Callback(func: FuncAliasVec3): AliasVec3;

    /**
     * @param func
     */
    export function CallFuncAliasVec4Callback(func: FuncAliasVec4): AliasVec4;

    /**
     * @param func
     */
    export function CallFuncAliasMat4x4Callback(func: FuncAliasMat4x4): AliasMat4x4;

    /**
     * @param func
     */
    export function CallFuncAliasAllCallback(func: FuncAliasAll): string;

    /**
     * @param func
     */
    export function CallFunc1Callback(func: Func1): number;

    /**
     * @param func
     */
    export function CallFunc2Callback(func: Func2): number;

    /**
     * @param func
     */
    export function CallFunc3Callback(func: Func3): void;

    /**
     * @param func
     */
    export function CallFunc4Callback(func: Func4): Vector4;

    /**
     * @param func
     */
    export function CallFunc5Callback(func: Func5): boolean;

    /**
     * @param func
     */
    export function CallFunc6Callback(func: Func6): number;

    /**
     * @param func
     */
    export function CallFunc7Callback(func: Func7): number;

    /**
     * @param func
     */
    export function CallFunc8Callback(func: Func8): Matrix4x4;

    /**
     * @param func
     */
    export function CallFunc9Callback(func: Func9): void;

    /**
     * @param func
     */
    export function CallFunc10Callback(func: Func10): number;

    /**
     * @param func
     */
    export function CallFunc11Callback(func: Func11): bigint;

    /**
     * @param func
     */
    export function CallFunc12Callback(func: Func12): boolean;

    /**
     * @param func
     */
    export function CallFunc13Callback(func: Func13): string;

    /**
     * @param func
     */
    export function CallFunc14Callback(func: Func14): string[];

    /**
     * @param func
     */
    export function CallFunc15Callback(func: Func15): number;

    /**
     * @param func
     */
    export function CallFunc16Callback(func: Func16): bigint;

    /**
     * @param func
     */
    export function CallFunc17Callback(func: Func17): string;

    /**
     * @param func
     */
    export function CallFunc18Callback(func: Func18): string;

    /**
     * @param func
     */
    export function CallFunc19Callback(func: Func19): string;

    /**
     * @param func
     */
    export function CallFunc20Callback(func: Func20): string;

    /**
     * @param func
     */
    export function CallFunc21Callback(func: Func21): string;

    /**
     * @param func
     */
    export function CallFunc22Callback(func: Func22): string;

    /**
     * @param func
     */
    export function CallFunc23Callback(func: Func23): string;

    /**
     * @param func
     */
    export function CallFunc24Callback(func: Func24): string;

    /**
     * @param func
     */
    export function CallFunc25Callback(func: Func25): string;

    /**
     * @param func
     */
    export function CallFunc26Callback(func: Func26): string;

    /**
     * @param func
     */
    export function CallFunc27Callback(func: Func27): string;

    /**
     * @param func
     */
    export function CallFunc28Callback(func: Func28): string;

    /**
     * @param func
     */
    export function CallFunc29Callback(func: Func29): string;

    /**
     * @param func
     */
    export function CallFunc30Callback(func: Func30): string;

    /**
     * @param func
     */
    export function CallFunc31Callback(func: Func31): string;

    /**
     * @param func
     */
    export function CallFunc32Callback(func: Func32): string;

    /**
     * @param func
     */
    export function CallFunc33Callback(func: Func33): string;

    /**
     * @param func
     */
    export function CallFuncEnumCallback(func: FuncEnum): string;

    /**
     * @param id
     * @param name
     */
    export function ResourceHandleCreate(id: number, name: string): bigint;

    /**
     */
    export function ResourceHandleCreateDefault(): bigint;

    /**
     * @param handle
     */
    export function ResourceHandleDestroy(handle: bigint): void;

    /**
     * @param handle
     */
    export function ResourceHandleGetId(handle: bigint): number;

    /**
     * @param handle
     */
    export function ResourceHandleGetName(handle: bigint): string;

    /**
     * @param handle
     * @param name
     */
    export function ResourceHandleSetName(handle: bigint, name: string): void;

    /**
     * @param handle
     */
    export function ResourceHandleIncrementCounter(handle: bigint): void;

    /**
     * @param handle
     */
    export function ResourceHandleGetCounter(handle: bigint): number;

    /**
     * @param handle
     * @param value
     */
    export function ResourceHandleAddData(handle: bigint, value: number): void;

    /**
     * @param handle
     */
    export function ResourceHandleGetData(handle: bigint): number[];

    /**
     */
    export function ResourceHandleGetAliveCount(): number;

    /**
     */
    export function ResourceHandleGetTotalCreated(): number;

    /**
     */
    export function ResourceHandleGetTotalDestroyed(): number;

    /**
     * @param initialValue
     */
    export function CounterCreate(initialValue: number): bigint;

    /**
     */
    export function CounterCreateZero(): bigint;

    /**
     * @param counter
     */
    export function CounterGetValue(counter: bigint): number;

    /**
     * @param counter
     * @param value
     */
    export function CounterSetValue(counter: bigint, value: number): void;

    /**
     * @param counter
     */
    export function CounterIncrement(counter: bigint): void;

    /**
     * @param counter
     */
    export function CounterDecrement(counter: bigint): void;

    /**
     * @param counter
     * @param amount
     */
    export function CounterAdd(counter: bigint, amount: number): void;

    /**
     * @param counter
     */
    export function CounterReset(counter: bigint): void;

    /**
     * @param counter
     */
    export function CounterIsPositive(counter: bigint): boolean;

    /**
     * @param value1
     * @param value2
     */
    export function CounterCompare(value1: number, value2: number): number;

    /**
     * @param values
     */
    export function CounterSum(values: number[]): number;

    /**
     * RAII wrapper for ResourceHandle pointer
     */
    export class ResourceHandle {
        /**
         * @param id
         * @param name
         */
        constructor(id: number, name: string);

        /**
         */
        constructor();

        /**
         * Check if the handle is valid.
         * @returns True if the handle is valid, false otherwise
         */
        valid(): boolean;

        /**
         * Get the raw handle value without transferring ownership.
         * @returns The underlying handle value
         */
        get(): bigint;

        /**
         * Release ownership of the handle and return it.
         * @returns The released handle value
         */
        release(): bigint;

        /**
         * Reset the handle by closing it.
         */
        reset(): void;

        /**
         * Close and destroy the handle if owned.
         */
        close(): void;

        /**
         */
        GetId(): number;

        /**
         */
        GetName(): string;

        /**
         * @param name
         */
        SetName(name: string): void;

        /**
         */
        IncrementCounter(): void;

        /**
         */
        GetCounter(): number;

        /**
         * @param value
         */
        AddData(value: number): void;

        /**
         */
        GetData(): number[];

        /**
         */
        static GetAliveCount(): number;

        /**
         */
        static GetTotalCreated(): number;

        /**
         */
        static GetTotalDestroyed(): number;

    }


    /**
     */
    export class Counter {
        /**
         * @param initialValue
         */
        constructor(initialValue: number);

        /**
         */
        constructor();

        /**
         * Check if the handle is valid.
         * @returns True if the handle is valid, false otherwise
         */
        valid(): boolean;

        /**
         * Get the raw handle value without transferring ownership.
         * @returns The underlying handle value
         */
        get(): bigint;

        /**
         * Release ownership of the handle and return it.
         * @returns The released handle value
         */
        release(): bigint;

        /**
         * Reset the handle by closing it.
         */
        reset(): void;

        /**
         */
        GetValue(): number;

        /**
         * @param value
         */
        SetValue(value: number): void;

        /**
         */
        Increment(): void;

        /**
         */
        Decrement(): void;

        /**
         * @param amount
         */
        Add(amount: number): void;

        /**
         */
        Reset(): void;

        /**
         */
        IsPositive(): boolean;

        /**
         * @param value1
         * @param value2
         */
        static Compare(value1: number, value2: number): number;

        /**
         * @param values
         */
        static Sum(values: number[]): number;

    }

}
