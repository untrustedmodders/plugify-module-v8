import { Plugin, Vector2, Vector3, Vector4, Matrix4x4 } from 'plugify';
import * as master from ':cross_call_master';

const boolStr = (b) => {
    return typeof b === 'boolean' ? `${b}`.toLowerCase() : '<wrong value>';
};

const ptrStr = (v) => {
    return `0x${BigInt.asUintN(64, v).toString(16)}`;
};

const ordZero = (ch) => {
    return ch.length === 0 ? 0 : ch.charCodeAt(0);
};

const stripZero = (fl) => {
    let s = `${parseFloat(fl)}`;
    if (s.endsWith('.0')) {
        s = s.slice(0, -2);
    }
    return s;
};

const floatStr = (v, aq = true) => {
    return stripZero(aq ? `${v.toFixed(4)}` : `${v.toFixed(6)}`);
};

const quoteStr = (s) => {
    return `'${s}'`;
};

const plainStr = (s) => {
    return `${s}`;
};

const char8Str = (ch) => {
    return `${ch}`;
};

const char16Str = (ch) => {
    return `${ordZero(ch)}`;
};

const enumStr = (ch) => {
    return `${parseInt(ch, 10)}`;
};

const vectorToString = (array, f = (v) => `${v}`) => {
    return `{${array.map(f).join(', ')}}`;
};

const podToString = (pod) => {
    if (pod instanceof Vector2) {
        return `{${floatStr(pod.x)}, ${floatStr(pod.y)}}`;
    }
    if (pod instanceof Vector3) {
        return `{${floatStr(pod.x)}, ${floatStr(pod.y)}, ${floatStr(pod.z)}}`;
    }
    if (pod instanceof Vector4) {
        return `{${floatStr(pod.x)}, ${floatStr(pod.y)}, ${floatStr(pod.z)}, ${floatStr(pod.w)}}`;
    }
    if (pod instanceof Matrix4x4) {
        const formattedRows = pod.m.map(row => `{${row.map(m => floatStr(m)).join(', ')}}`);
        return `{${formattedRows.join(', ')}}`;
    }
    throw new TypeError('Non POD type');
};

// <<< Test part >>>

export class CrossCallWorker extends Plugin {
}

export const noParamReturnVoid = () => {
    // Do nothing
};

export const noParamReturnBool = () => true;

export const noParamReturnChar8 = () => '\x7f';

export const noParamReturnChar16 = () => '\uffff';

export const noParamReturnInt8 = () => 0x7f;

export const noParamReturnInt16 = () => 0x7fff;

export const noParamReturnInt32 = () => 0x7fffffff;

export const noParamReturnInt64 = () => 0x7fffffffffffffff;

export const noParamReturnUInt8 = () => 0xff;

export const noParamReturnUInt16 = () => 0xffff;

export const noParamReturnUInt32 = () => 0xffffffff;

export const noParamReturnUInt64 = () => 0xFFFFFFFFFFFFFFFFn & 0xFFFFFFFFFFFFFFFFn;

export const noParamReturnPointer = () => 0x1;

export const noParamReturnFloat = () => 3.4028235e38;

export const noParamReturnDouble = () => Number.MAX_VALUE;

export const noParamReturnFunction = () => null;

export const noParamReturnString = () => 'Hello World';

export const noParamReturnAny = () => [1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0];

export const noParamReturnArrayBool = () => [true, false];

export const noParamReturnArrayChar8 = () => ['a', 'b', 'c', 'd'];

export const noParamReturnArrayChar16 = () => ['a', 'b', 'c', 'd'];

export const noParamReturnArrayInt8 = () => [-3, -2, -1, 0, 1];

export const noParamReturnArrayInt16 = () => [-4, -3, -2, -1, 0, 1];

export const noParamReturnArrayInt32 = () => [-5, -4, -3, -2, -1, 0, 1];

export const noParamReturnArrayInt64 = () => [-6, -5, -4, -3, -2, -1, 0, 1];

export const noParamReturnArrayUInt8 = () => [0, 1, 2, 3, 4, 5, 6, 7, 8];

export const noParamReturnArrayUInt16 = () => [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];

export const noParamReturnArrayUInt32 = () => [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10];

export const noParamReturnArrayUInt64 = () => [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11];

export const noParamReturnArrayPointer = () => [0, 1, 2, 3];

export const noParamReturnArrayFloat = () => [-12.34, 0.0, 12.34];

export const noParamReturnArrayDouble = () => [-12.345, 0.0, 12.345];

export const noParamReturnArrayString = () => [
    '1st string',
    '2nd string',
    '3rd element string (Should be big enough to avoid small string optimization)'
];

export const noParamReturnArrayAny = () => [
    1.0, 2.0, 
    "3rd element string (Should be big enough to avoid small string optimization)", 
    ["lolek", "and", "bolek"], 
    1
];

export const noParamReturnArrayVector2 = () => [
    new Vector2(1.1, 2.2),
    new Vector2(-3.3, 4.4),
    new Vector2(5.5, -6.6),
    new Vector2(7.7, 8.8),
    new Vector2(0.0, 0.0),
];

export const noParamReturnArrayVector3 = () => [
    new Vector3(1.1, 2.2, 3.3),
    new Vector3(-4.4, 5.5, -6.6),
    new Vector3(7.7, 8.8, 9.9),
    new Vector3(0.0, 0.0, 0.0),
    new Vector3(10.1, -11.2, 12.3),
];

export const noParamReturnArrayVector4 = () => [
    new Vector4(1.1, 2.2, 3.3, 4.4),
    new Vector4(-5.5, 6.6, -7.7, 8.8),
    new Vector4(9.9, 0.0, -1.1, 2.2),
    new Vector4(3.3, 4.4, 5.5, 6.6),
    new Vector4(-7.7, -8.8, 9.9, -10.1),
];

export const noParamReturnArrayMatrix4x4 = () => [
    // Identity matrix
    new Matrix4x4(),
    // Example random matrix #1
    new Matrix4x4([
        [2.0, 3.0, 4.0, 5.0],
        [6.0, 7.0, 8.0, 9.0],
        [10.0, 11.0, 12.0, 13.0],
        [14.0, 15.0, 16.0, 17.0],
    ]),
    // Negative matrix
    new Matrix4x4([
        [-1.0, -2.0, -3.0, -4.0],
        [-5.0, -6.0, -7.0, -8.0],
        [-9.0, -10.0, -11.0, -12.0],
        [-13.0, -14.0, -15.0, -16.0],
    ])
];

export const noParamReturnVector2 = () => new Vector2(1.0, 2.0);

export const noParamReturnVector3 = () => new Vector3(1.0, 2.0, 3.0);

export const noParamReturnVector4 = () => new Vector4(1.0, 2.0, 3.0, 4.0);

export const noParamReturnMatrix4x4 = () => new Matrix4x4([
    [1.0, 2.0, 3.0, 4.0],
    [5.0, 6.0, 7.0, 8.0],
    [9.0, 10.0, 11.0, 12.0],
    [13.0, 14.0, 15.0, 16.0]
]);

export const param1 = (a) => {
    const buffer = `${a}`;
};

export const param2 = (a, b) => {
    const buffer = `${a}${b}`;
};

export const param3 = (a, b, c) => {
    const buffer = `${a}${b}${c}`;
};

export const param4 = (a, b, c, d) => {
    const buffer = `${a}${b}${c}${d}`;
};

export const param5 = (a, b, c, d, e) => {
    const buffer = `${a}${b}${c}${d}${e}`;
};

export const param6 = (a, b, c, d, e, f) => {
    const buffer = `${a}${b}${c}${d}${e}${f}`;
};

export const param7 = (a, b, c, d, e, f, g) => {
    const buffer = `${a}${b}${c}${d}${e}${f}${g}`;
};

export const param8 = (a, b, c, d, e, f, g, h) => {
    const buffer = `${a}${b}${c}${d}${e}${f}${g}${h}`;
};

export const param9 = (a, b, c, d, e, f, g, h, k) => {
    const buffer = `${a}${b}${c}${d}${e}${f}${g}${h}${k}`;
};

export const param10 = (a, b, c, d, e, f, g, h, k, l) => {
    const buffer = `${a}${b}${c}${d}${e}${f}${g}${h}${k}${l}`;
};

export const paramRef1 = (a) => {
    return [null, 42];
};

export const paramRef2 = (a, b) => {
    return [null, 10, 3.14];
};

export const paramRef3 = (a, b, c) => {
    return [null, -20, 2.718, 3.14159];
};

export const paramRef4 = (a, b, c, d) => {
    return [null, 100, -5.55, 1.618, new Vector4(1.0, 2.0, 3.0, 4.0)];
};

export const paramRef5 = (a, b, c, d, e) => {
    return [null, 500, -10.5, 2.71828, new Vector4(-1.0, -2.0, -3.0, -4.0), [-6, -5, -4, -3, -2, -1, 0, 1]];
};

export const paramRef6 = (a, b, c, d, e, f) => {
    return [null, 750, 20.0, 1.23456, new Vector4(10.0, 20.0, 30.0, 40.0), [-6, -5, -4], 'Z'];
};

export const paramRef7 = (a, b, c, d, e, f, g) => {
    return [null, -1000, 3.0, -1.0, new Vector4(100.0, 200.0, 300.0, 400.0), [-6, -5, -4, -3], 'Y', 'Hello, World!'];
};

export const paramRef8 = (a, b, c, d, e, f, g, h) => {
    return [null, 999, -7.5, 0.123456, new Vector4(-100.0, -200.0, -300.0, -400.0), [-6, -5, -4, -3, -2, -1], 'X', 'Goodbye, World!', 'A'];
};

export const paramRef9 = (a, b, c, d, e, f, g, h, k) => {
    return [null, -1234, 123.45, -678.9, new Vector4(987.65, 432.1, 123.456, 789.123), [-6, -5, -4, -3, -2, -1, 0, 1, 5, 9], 'W', 'Testing, 1 2 3', 'B', 42];
};

export const paramRef10 = (a, b, c, d, e, f, g, h, k, l) => {
    return [null, 987, -0.123, 456.789, new Vector4(-123.456, 0.987, 654.321, -789.123), [-6, -5, -4, -3, -2, -1, 0, 1, 5, 9, 4, -7], 'V', 'Another string', 'C', -444, 0x12345678];
};

export const paramRefVectors = (p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15) => {
    return [null, [true], ['a', 'b', 'c'], ['d', 'e', 'f'], [-3, -2, -1, 0, 1, 2, 3], [-4, -3, -2, -1, 0, 1, 2, 3, 4], [-5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5], [-6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6], [0, 1, 2, 3, 4, 5, 6, 7], [0, 1, 2, 3, 4, 5, 6, 7, 8], [0, 1, 2, 3, 4, 5, 6, 7, 8, 9], [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10], [0, 1, 2], [-12.34, 0.0, 12.34], [-12.345, 0.0, 12.345], ['1', '12', '123', '1234', '12345', '123456']];
};

export const paramAllPrimitives = (p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14) => {
    const buffer = `${p1}${p2}${p3}${p4}${p5}${p6}${p7}${p8}${p9}${p10}${p11}${p12}${p13}${p14}`;
    return 56;
};

export const paramEnum = (p1, p2) => {
    return p1 + p2.reduce((a, b) => a + b, 0);
};

export const paramEnumRef = (p1, p2) => {
    const e = master.Example; // Assuming Example is an object containing enum-like properties
    p1 = e.Forth;
    p2 = [e.First, e.Second, e.Third];
	let sum = p2.reduce((a, b) => a + b, 0n)
    return [p1 + sum, p1, p2];
};

export const paramVariant = (p1, p2) => {
    const buffer = `${String(p1)}|${String(p2)}`;
};

export const paramVariantRef = (p1, p2) => {
    return [null, 'Z', [false, 6.28, [1.0, 2.0, 3.0], 0x0, 123456789]];
};

export const callFuncVoid = (func) => {
    func();
};

export const callFuncBool = (func) => {
    return func();
};

export const callFuncChar8 = (func) => {
    return func();
};

export const callFuncChar16 = (func) => {
    return func();
};

export const callFuncInt8 = (func) => {
    return func();
};

export const callFuncInt16 = (func) => {
    return func();
};

export const callFuncInt32 = (func) => {
    return func();
};

export const callFuncInt64 = (func) => {
    return func();
};

export const callFuncUInt8 = (func) => {
    return func();
};

export const callFuncUInt16 = (func) => {
    return func();
};

export const callFuncUInt32 = (func) => {
    return func();
};

export const callFuncUInt64 = (func) => {
    return func();
};

export const callFuncPtr = (func) => {
    return func();
};

export const callFuncFloat = (func) => {
    return func();
};

export const callFuncDouble = (func) => {
    return func();
};

export const callFuncFunction = (func) => {
    return func();
};

export const callFuncString = (func) => {
    return func();
};

export const callFuncAny = (func) => {
    return func();
};

export const callFuncBoolVector = (func) => {
    return func();
};

export const callFuncChar8Vector = (func) => {
    return func();
};

export const callFuncChar16Vector = (func) => {
    return func();
};

export const callFuncInt8Vector = (func) => {
    return func();
};

export const callFuncInt16Vector = (func) => {
    return func();
};

export const callFuncInt32Vector = (func) => {
    return func();
};

export const callFuncInt64Vector = (func) => {
    return func();
};

export const callFuncUInt8Vector = (func) => {
    return func();
};

export const callFuncUInt16Vector = (func) => {
    return func();
};

export const callFuncUInt32Vector = (func) => {
    return func();
};

export const callFuncUInt64Vector = (func) => {
    return func();
};

export const callFuncPtrVector = (func) => {
    return func();
};

export const callFuncFloatVector = (func) => {
    return func();
};

export const callFuncDoubleVector = (func) => {
    return func();
};

export const callFuncStringVector = (func) => {
    return func();
};

export const callFuncAnyVector = (func) => {
    return func();
};

export const callFuncVec2Vector = (func) => {
    return func();
};

export const callFuncVec3Vector = (func) => {
    return func();
};

export const callFuncVec4Vector = (func) => {
    return func();
};

export const callFuncMat4x4Vector = (func) => {
    return func();
};

export const callFuncVec2 = (func) => {
    return func();
};

export const callFuncVec3 = (func) => {
    return func();
};

export const callFuncVec4 = (func) => {
    return func();
};

export const callFuncMat4x4 = (func) => {
    return func();
};

// Assuming Vector2, Vector3, Vector4, and Matrix4x4 are classes that need to be imported or defined elsewhere in your project
// import { Vector2, Vector3, Vector4, Matrix4x4 } from './path-to-classes';

export const callFunc1 = (func) => {
    const vec = new Vector3(4.5, 5.6, 6.7);
    return func(vec);
};

export const callFunc2 = (func) => {
    const f = 2.71;
    const i64 = 200;
    return func(f, i64);
};

export const callFunc3 = (func) => {
    const ptr = 12345;
    const vec4 = new Vector4(7.8, 8.9, 9.1, 10.2);
    const strValue = 'RandomString';
    return func(ptr, vec4, strValue);
};

export const callFunc4 = (func) => {
    const b = false;
    const i32 = 42;
    const ch16 = 'B';
    const mat = Matrix4x4.zero();
    return func(b, i32, ch16, mat);
};

export const callFunc5 = (func) => {
    const i8 = 10;
    const vec2 = new Vector2(3.4, 5.6);
    const ptr = 67890;
    const d = 1.618;
    const vec64 = [4, 5, 6];
    return func(i8, vec2, ptr, d, vec64);
};

export const callFunc6 = (func) => {
    const strValue = 'AnotherString';
    const f = 4.56;
    const vecF = [4.0, 5.0, 6.0];
    const i16 = 30;
    const vecU8 = [3, 4, 5];
    const ptr = 24680;
    return func(strValue, f, vecF, i16, vecU8, ptr);
};

export const callFunc7 = (func) => {
    const vecC = ['X', 'Y', 'Z'];
    const u16 = 20;
    const ch16 = 'C';
    const vecU32 = [4, 5, 6];
    const vec4 = new Vector4(4.5, 5.6, 6.7, 7.8);
    const b = false;
    const u64 = 200;
    return func(vecC, u16, ch16, vecU32, vec4, b, u64);
};

export const callFunc8 = (func) => {
    const vec3 = new Vector3(4.0, 5.0, 6.0);
    const vecU32 = [4, 5, 6];
    const i16 = 30;
    const b = false;
    const vec4 = new Vector4(4.5, 5.6, 6.7, 7.8);
    const vecC16 = ['D', 'E'];
    const ch16 = 'B';
    const i32 = 50;
    return func(vec3, vecU32, i16, b, vec4, vecC16, ch16, i32);
};

export const callFunc9 = (func) => {
    const f = 2.71;
    const vec2 = new Vector2(3.4, 5.6);
    const vecI8 = [4, 5, 6];
    const u64 = 250;
    const b = false;
    const s = "Random";
    const vec4 = new Vector4(4.5, 5.6, 6.7, 7.8);
    const i16 = 30;
    const ptr = 13579;
    return func(f, vec2, vecI8, u64, b, s, vec4, i16, ptr);
};

export const callFunc10 = (func) => {
    const vec4 = new Vector4(5.6, 7.8, 8.9, 9.0);
    const mat = Matrix4x4.zero();
    const vecU32 = [4, 5, 6];
    const u64 = 150;
    const vecC = ['X', 'Y', 'Z'];
    const i32 = 60;
    const b = false;
    const vec2 = new Vector2(3.4, 5.6);
    const i64 = 75;
    const d = 2.71;
    return func(vec4, mat, vecU32, u64, vecC, i32, b, vec2, i64, d);
};

export const callFunc11 = (func) => {
    const vecB = [false, true, false];
    const ch16 = 'C';
    const u8 = 10;
    const d = 2.71;
    const vec3 = new Vector3(4.0, 5.0, 6.0);
    const vecI8 = [3, 4, 5];
    const i64 = 150;
    const u16 = 20;
    const f = 2.0;
    const vec2 = new Vector2(4.5, 6.7);
    const u32 = 30;
    return func(vecB, ch16, u8, d, vec3, vecI8, i64, u16, f, vec2, u32);
};

export const callFunc12 = (func) => {
    const ptr = 98765;
    const vecD = [4.0, 5.0, 6.0];
    const u32 = 30;
    const d = 1.41;
    const b = false;
    const i32 = 25;
    const i8 = 10;
    const u64 = 300;
    const f = 2.72;
    const vecPtr = [2, 3, 4];
    const i64 = 200;
    const ch = 'B';
    return func(ptr, vecD, u32, d, b, i32, i8, u64, f, vecPtr, i64, ch);
};

export const callFunc13 = (func) => {
    const i64 = 75;
    const vecC = ['D', 'E', 'F'];
    const u16 = 20;
    const f = 2.71;
    const vecB = [false, true, false];
    const vec4 = new Vector4(5.6, 7.8, 9.0, 10.1);
    const s = 'RandomString';
    const i32 = 30;
    const vec3 = new Vector3(4.0, 5.0, 6.0);
    const ptr = 13579;
    const vec2 = new Vector2(4.5, 6.7);
    const vecU8 = [2, 3, 4];
    const i16 = 20;
    return func(i64, vecC, u16, f, vecB, vec4, s, i32, vec3, ptr, vec2, vecU8, i16);
};

export const callFunc14 = (func) => {
    const vecC = ['D', 'E', 'F'];
    const vecU32 = [4, 5, 6];
    const mat = Matrix4x4.zero();
    const b = false;
    const ch16 = 'B';
    const i32 = 25;
    const vecF = [4.0, 5.0, 6.0];
    const u16 = 30;
    const vecU8 = [3, 4, 5];
    const i8 = 10;
    const vec3 = new Vector3(4.0, 5.0, 6.0);
    const vec4 = new Vector4(5.6, 7.8, 9.0, 10.1);
    const d = 2.72;
    const ptr = 54321;
    return func(vecC, vecU32, mat, b, ch16, i32, vecF, u16, vecU8, i8, vec3, vec4, d, ptr);
};

export const callFunc15 = (func) => {
    const vecI16 = [4, 5, 6];
    const mat = Matrix4x4.zero();
    const vec4 = new Vector4(7.8, 8.9, 9.0, 10.1);
    const ptr = 12345;
    const u64 = 200;
    const vecU32 = [5, 6, 7];
    const b = false;
    const f = 3.14;
    const vecC16 = ['D', 'E'];
    const u8 = 6;
    const i32 = 25;
    const vec2 = new Vector2(5.6, 7.8);
    const u16 = 40;
    const d = 2.71;
    const vecU8 = [1, 3, 5];
    return func(vecI16, mat, vec4, ptr, u64, vecU32, b, f, vecC16, u8, i32, vec2, u16, d, vecU8);
};

export const callFunc16 = (func) => {
    const vecB = [true, true, false];
    const i16 = 20;
    const vecI8 = [2, 3, 4];
    const vec4 = new Vector4(7.8, 8.9, 9.0, 10.1);
    const mat = Matrix4x4.zero();
    const vec2 = new Vector2(5.6, 7.8);
    const vecU64 = [5, 6, 7];
    const vecC = ['D', 'E', 'F'];
    const s = 'DifferentString';
    const i64 = 300;
    const vecU32 = [6, 7, 8];
    const vec3 = new Vector3(5.0, 6.0, 7.0);
    const f = 3.14;
    const d = 2.718;
    const i8 = 6;
    const u16 = 30;
    return func(vecB, i16, vecI8, vec4, mat, vec2, vecU64, vecC, s, i64, vecU32, vec3, f, d, i8, u16);
};

export const callFunc17 = (func) => {
    let i32 = 42;
    [, i32] = func(i32);
    return `${i32}`;
};

export const callFunc18 = (func) => {
    let i8 = 9;
    let i16 = 25;
    const [ret, newI8, newI16] = func(i8, i16);
    i8 = newI8;
    i16 = newI16;
    return `${podToString(ret)}|${i8}|${i16}`;
};

export const callFunc19 = (func) => {
    let u32 = 75;
    let vec3 = new Vector3(4.0, 5.0, 6.0);
    let vecU32 = [4, 5, 6];
    [, u32, vec3, vecU32] = func(u32, vec3, vecU32);
    return `${u32}|${podToString(vec3)}|${vectorToString(vecU32)}`;
};

export const callFunc20 = (func) => {
    let ch16 = 'Z';
    let vec4 = new Vector4(5.0, 6.0, 7.0, 8.0);
    let vecU64 = [4, 5, 6];
    let ch = 'X';
    const [ret, newCh16, newVec4, newVecU64, newCh] = func(ch16, vec4, vecU64, ch);
    ch16 = newCh16;
    vec4 = newVec4;
    vecU64 = newVecU64;
    ch = newCh;
    return `${ret}|${ordZero(ch16)}|${podToString(vec4)}|${vectorToString(vecU64)}|${ch}`;
};

export const callFunc21 = (func) => {
    let mat = Matrix4x4.zero();
    let vecI32 = [4, 5, 6];
    let vec2 = new Vector2(3.0, 4.0);
    let b = false;
    let d = 6.28;
    const [ret, newMat, newVecI32, newVec2, newB, newD] = func(mat, vecI32, vec2, b, d);
    mat = newMat;
    vecI32 = newVecI32;
    vec2 = newVec2;
    b = newB;
    d = newD;
    return `${floatStr(ret)}|${podToString(mat)}|${vectorToString(vecI32)}|${podToString(vec2)}|${boolStr(b)}|${floatStr(d)}`;
};

export const callFunc22 = (func) => {
    let ptr = 1;
    let u32 = 20;
    let vecD = [4.0, 5.0, 6.0];
    let i16 = 15;
    let strParam = 'Updated Test';
    let vec4 = new Vector4(5.0, 6.0, 7.0, 8.0);
    const [ret, newPtr, newU32, newVecD, newI16, newStrParam, newVec4] = func(ptr, u32, vecD, i16, strParam, vec4);
    ptr = newPtr;
    u32 = newU32;
    vecD = newVecD;
    i16 = newI16;
    strParam = newStrParam;
    vec4 = newVec4;
    return `${ret}|${ptrStr(ptr)}|${u32}|${vectorToString(vecD)}|${i16}|${strParam}|${podToString(vec4)}`;
};

export const callFunc23 = (func) => {
    let u64 = 200;
    let vec2 = new Vector2(3.0, 4.0);
    let vecI16 = [4, 5, 6];
    let ch16 = 'Y';
    let f = 2.34;
    let i8 = 10;
    let vecU8 = [3, 4, 5];
    [, u64, vec2, vecI16, ch16, f, i8, vecU8] = func(u64, vec2, vecI16, ch16, f, i8, vecU8);
    return `${u64}|${podToString(vec2)}|${vectorToString(vecI16)}|${ordZero(ch16)}|${floatStr(f)}|${i8}|${vectorToString(vecU8)}`;
};

export const callFunc24 = (func) => {
    let vecC = ['D', 'E', 'F'];
    let i64 = 100;
    let vecU8 = [3, 4, 5];
    let vec4 = new Vector4(5.0, 6.0, 7.0, 8.0);
    let u64 = 200;
    let vecPtr = [3, 4, 5];
    let d = 6.28;
    let vecPtr2 = [4, 5, 6, 7];
    const [ret, newVecC, newI64, newVecU8, newVec4, newU64, newVecPtr, newD, newVecPtr2] = func(vecC, i64, vecU8, vec4, u64, vecPtr, d, vecPtr2);
    vecC = newVecC;
    i64 = newI64;
    vecU8 = newVecU8;
    vec4 = newVec4;
    u64 = newU64;
    vecPtr = newVecPtr;
    d = newD;
    vecPtr2 = newVecPtr2;
    return `${podToString(ret)}|${vectorToString(vecC, char8Str)}|${i64}|${vectorToString(vecU8)}|${podToString(vec4)}|${u64}|${vectorToString(vecPtr, ptrStr)}|${floatStr(d)}|${vectorToString(vecPtr2, ptrStr)}`;
};

export const callFunc25 = (func) => {
    let i32 = 50;
    let vecPtr = [3, 4, 5];
    let b = false;
    let u8 = 10;
    let strVal = 'Updated Test String';
    let vec3 = new Vector3(4.0, 5.0, 6.0);
    let i64 = 100;
    let vec4 = new Vector4(5.0, 6.0, 7.0, 8.0);
    let u16 = 20;
    const [ret, newI32, newVecPtr, newB, newU8, newStrVal, newVec3, newI64, newVec4, newU16] = func(i32, vecPtr, b, u8, strVal, vec3, i64, vec4, u16);
    i32 = newI32;
    vecPtr = newVecPtr;
    b = newB;
    u8 = newU8;
    strVal = newStrVal;
    vec3 = newVec3;
    i64 = newI64;
    vec4 = newVec4;
    u16 = newU16;
    return `${floatStr(ret)}|${i32}|${vectorToString(vecPtr, ptrStr)}|${boolStr(b)}|${u8}|${strVal}|${podToString(vec3)}|${i64}|${podToString(vec4)}|${u16}`;
};

export const callFunc26 = (func) => {
    let ch16 = 'B';
    let vec2 = new Vector2(3.0, 4.0);
    let mat = Matrix4x4.zero();
    let vecF = [4.0, 5.0, 6.0];
    let i16 = 20;
    let u64 = 200;
    let u32 = 20;
    let vecU16 = [3, 4, 5];
    let ptr = 0xDEADBEAFDEADBEAF;
    let b = false;
    const [ret, newCh16, newVec2, newMat, newVecF, newI16, newU64, newU32, newVecU16, newPtr, newB] = func(ch16, vec2, mat, vecF, i16, u64, u32, vecU16, ptr, b);
    ch16 = newCh16;
    vec2 = newVec2;
    mat = newMat;
    vecF = newVecF;
    i16 = newI16;
    u64 = newU64;
    u32 = newU32;
    vecU16 = newVecU16;
    ptr = newPtr;
    b = newB;
    return `${ret}|${ordZero(ch16)}|${podToString(vec2)}|${podToString(mat)}|${vectorToString(vecF, floatStr)}|${u64}|${u32}|${vectorToString(vecU16)}|${ptrStr(ptr)}|${boolStr(b)}`;
};

export const callFunc27 = (func) => {
    let f = 2.56;
    let vec3 = new Vector3(4.0, 5.0, 6.0);
    let ptr = 0;
    let vec2 = new Vector2(3.0, 4.0);
    let vecI16 = [4, 5, 6];
    let mat = Matrix4x4.zero();
    let b = false;
    let vec4 = new Vector4(5.0, 6.0, 7.0, 8.0);
    let i8 = 10;
    let i32 = 40;
    let vecU8 = [3, 4, 5];
    const [ret, newF, newVec3, newPtr, newVec2, newVecI16, newMat, newB, newVec4, newI8, newI32, newVecU8] = func(f, vec3, ptr, vec2, vecI16, mat, b, vec4, i8, i32, vecU8);
    f = newF;
    vec3 = newVec3;
    ptr = newPtr;
    vec2 = newVec2;
    vecI16 = newVecI16;
    mat = newMat;
    b = newB;
    vec4 = newVec4;
    i8 = newI8;
    i32 = newI32;
    vecU8 = newVecU8;
    return `${ret}|${floatStr(f)}|${podToString(vec3)}|${ptrStr(ptr)}|${podToString(vec2)}|${vectorToString(vecI16)}|${podToString(mat)}|${boolStr(b)}|${podToString(vec4)}|${i8}|${i32}|${vectorToString(vecU8)}`;
};

export const callFunc28 = (func) => {
    let ptr = 1;
    let u16 = 20;
    let vecU32 = [4, 5, 6];
    let mat = Matrix4x4.zero();
    let f = 2.71;
    let vec4 = new Vector4(5.0, 6.0, 7.0, 8.0);
    let strVal = 'New example string';
    let vecU64 = [400, 500, 600];
    let i64 = 987654321;
    let b = false;
    let vec3 = new Vector3(4.0, 5.0, 6.0);
    let vecF = [4.0, 5.0, 6.0];
    const [ret, newPtr, newU16, newVecU32, newMat, newF, newVec4, newStrVal, newVecU64, newI64, newB, newVec3, newVecF] = func(ptr, u16, vecU32, mat, f, vec4, strVal, vecU64, i64, b, vec3, vecF);
    ptr = newPtr;
    u16 = newU16;
    vecU32 = newVecU32;
    mat = newMat;
    f = newF;
    vec4 = newVec4;
    strVal = newStrVal;
    vecU64 = newVecU64;
    i64 = newI64;
    b = newB;
    vec3 = newVec3;
    vecF = newVecF;
    return `${ret}|${ptrStr(ptr)}|${u16}|${vectorToString(vecU32)}|${podToString(mat)}|${floatStr(f)}|${podToString(vec4)}|${strVal}|${vectorToString(vecU64)}|${i64}|${boolStr(b)}|${podToString(vec3)}|${vectorToString(vecF, floatStr)}`;
};

export const callFunc29 = (func) => {
    let vec4 = new Vector4(2.0, 3.0, 4.0, 5.0);
    let i32 = 99;
    let vecI8 = [4, 5, 6];
    let d = 2.71;
    let b = false;
    let i8 = 10;
    let vecU16 = [4, 5, 6];
    let f = 3.21;
    let strVal = 'Yet another example string';
    let mat = Matrix4x4.zero();
    let u64 = 200;
    let vec3 = new Vector3(5.0, 6.0, 7.0);
    let vecI64 = [2000, 3000, 4000];
    const [ret, newVec4, newI32, newVecI8, newD, newB, newI8, newVecU16, newF, newStrVal, newMat, newU64, newVec3, newVecI64] = func(vec4, i32, vecI8, d, b, i8, vecU16, f, strVal, mat, u64, vec3, vecI64);
    vec4 = newVec4;
    i32 = newI32;
    vecI8 = newVecI8;
    d = newD;
    b = newB;
    i8 = newI8;
    vecU16 = newVecU16;
    f = newF;
    strVal = newStrVal;
    mat = newMat;
    u64 = newU64;
    vec3 = newVec3;
    vecI64 = newVecI64;
    return `${vectorToString(ret, quoteStr)}|${podToString(vec4)}|${i32}|${vectorToString(vecI8)}|${floatStr(d)}|${boolStr(b)}|${i8}|${vectorToString(vecU16)}|${floatStr(f)}|${strVal}|${podToString(mat)}|${u64}|${podToString(vec3)}|${vectorToString(vecI64)}`;
};

export const callFunc30 = (func) => {
    let ptr = 1;
    let vec4 = new Vector4(2.0, 3.0, 4.0, 5.0);
    let i64 = 987654321;
    let vecU32 = [4, 5, 6];
    let b = false;
    let strVal = 'Updated String for Func30';
    let vec3 = new Vector3(5.0, 6.0, 7.0);
    let vecU8 = [1, 2, 3];
    let f = 5.67;
    let vec2 = new Vector2(3.0, 4.0);
    let mat = Matrix4x4.zero();
    let i8 = 10;
    let vecF = [4.0, 5.0, 6.0];
    let d = 8.90;
    const [ret, newPtr, newVec4, newI64, newVecU32, newB, newStrVal, newVec3, newVecU8, newF, newVec2, newMat, newI8, newVecF, newD] = func(ptr, vec4, i64, vecU32, b, strVal, vec3, vecU8, f, vec2, mat, i8, vecF, d);
    ptr = newPtr;
    vec4 = newVec4;
    i64 = newI64;
    vecU32 = newVecU32;
    b = newB;
    strVal = newStrVal;
    vec3 = newVec3;
    vecU8 = newVecU8;
    f = newF;
    vec2 = newVec2;
    mat = newMat;
    i8 = newI8;
    vecF = newVecF;
    d = newD;
    return `${ret}|${ptrStr(ptr)}|${podToString(vec4)}|${i64}|${vectorToString(vecU32)}|${boolStr(b)}|${strVal}|${podToString(vec3)}|${vectorToString(vecU8)}|${floatStr(f)}|${podToString(vec2)}|${podToString(mat)}|${i8}|${vectorToString(vecF, floatStr)}|${floatStr(d, false)}`;
};

export const callFunc31 = (func) => {
    let ch = 'B';
    let u32 = 200;
    let vecU64 = [4, 5, 6];
    let vec4 = new Vector4(2.0, 3.0, 4.0, 5.0);
    let strVal = 'Updated String for Func31';
    let b = true;
    let i64 = 987654321;
    let vec2 = new Vector2(3.0, 4.0);
    let i8 = 10;
    let u16 = 20;
    let vecI16 = [4, 5, 6];
    let mat = Matrix4x4.zero();
    let vec3 = new Vector3(4.0, 5.0, 6.0);
    let f = 5.67;
    let vecD = [4.0, 5.0, 6.0];
    const [ret, newCh, newU32, newVecU64, newVec4, newStrVal, newB, newI64, newVec2, newI8, newU16, newVecI16, newMat, newVec3, newF, newVecD] = func(ch, u32, vecU64, vec4, strVal, b, i64, vec2, i8, u16, vecI16, mat, vec3, f, vecD);
    ch = newCh;
    u32 = newU32;
    vecU64 = newVecU64;
    vec4 = newVec4;
    strVal = newStrVal;
    b = newB;
    i64 = newI64;
    vec2 = newVec2;
    i8 = newI8;
    u16 = newU16;
    vecI16 = newVecI16;
    mat = newMat;
    vec3 = newVec3;
    f = newF;
    vecD = newVecD;
    return `${podToString(ret)}|${ch}|${u32}|${vectorToString(vecU64)}|${podToString(vec4)}|${strVal}|${boolStr(b)}|${i64}|${podToString(vec2)}|${i8}|${u16}|${vectorToString(vecI16)}|${podToString(mat)}|${podToString(vec3)}|${floatStr(f)}|${vectorToString(vecD)}`;
};

export const callFunc32 = (func) => {
    let i32 = 30;
    let u16 = 20;
    let vecI8 = [4, 5, 6];
    let vec4 = new Vector4(2.0, 3.0, 4.0, 5.0);
    let ptr = 1;
    let vecU32 = [4, 5, 6];
    let mat = Matrix4x4.zero();
    let u64 = 200;
    let strVal = 'Updated String for Func32';
    let i64 = 987654321;
    let vec2 = new Vector2(3.0, 4.0);
    let vecI8_2 = [7, 8, 9];
    let b = false;
    let vec3 = new Vector3(4.0, 5.0, 6.0);
    let u8 = 128;
    let vecC16 = ['D', 'E', 'F'];
    [, i32, u16, vecI8, vec4, ptr, vecU32, mat, u64, strVal, i64, vec2, vecI8_2, b, vec3, u8, vecC16] = func(i32, u16, vecI8, vec4, ptr, vecU32, mat, u64, strVal, i64, vec2, vecI8_2, b, vec3, u8, vecC16);
    return `${i32}|${u16}|${vectorToString(vecI8)}|${podToString(vec4)}|${ptrStr(ptr)}|${vectorToString(vecU32)}|${podToString(mat)}|${u64}|${strVal}|${i64}|${podToString(vec2)}|${vectorToString(vecI8_2)}|${boolStr(b)}|${podToString(vec3)}|${u8}|${vectorToString(vecC16, char16Str)}`;
};

export const callFunc33 = (func) => {
    let variant = 30;
    const [, variantRef] = func(variant);
    return variantRef;
};

export const callFuncEnum = (func) => {
    const e = master.Example;
    const [ret, p2] = func(e.Forth, []);
    return `${vectorToString(ret, enumStr)}|${vectorToString(p2, enumStr)}`;
};

export const reverseNoParamReturnVoid = () => {
    master.NoParamReturnVoidCallback();
};

export const reverseNoParamReturnBool = () => {
    const result = master.NoParamReturnBoolCallback();
    return boolStr(result);
};

export const reverseNoParamReturnChar8 = () => {
    const result = master.NoParamReturnChar8Callback();
    return char8Str(result);
};

export const reverseNoParamReturnChar16 = () => {
    const result = master.NoParamReturnChar16Callback();
    return char16Str(result);
};

export const reverseNoParamReturnInt8 = () => {
    const result = master.NoParamReturnInt8Callback();
    return `${result}`;
};

export const reverseNoParamReturnInt16 = () => {
    const result = master.NoParamReturnInt16Callback();
    return `${result}`;
};

export const reverseNoParamReturnInt32 = () => {
    const result = master.NoParamReturnInt32Callback();
    return `${result}`;
};

export const reverseNoParamReturnInt64 = () => {
    const result = master.NoParamReturnInt64Callback();
    return `${result}`;
};

export const reverseNoParamReturnUInt8 = () => {
    const result = master.NoParamReturnUInt8Callback();
    return `${result}`;
};

export const reverseNoParamReturnUInt16 = () => {
    const result = master.NoParamReturnUInt16Callback();
    return `${result}`;
};

export const reverseNoParamReturnUInt32 = () => {
    const result = master.NoParamReturnUInt32Callback();
    return `${result}`;
};

export const reverseNoParamReturnUInt64 = () => {
    const result = master.NoParamReturnUInt64Callback();
    return `${result}`;
};

export const reverseNoParamReturnPointer = () => {
    const result = master.NoParamReturnPointerCallback();
    return ptrStr(result);
};

export const reverseNoParamReturnFloat = () => {
    const result = master.NoParamReturnFloatCallback();
    return floatStr(result);
};

export const reverseNoParamReturnDouble = () => {
    const result = master.NoParamReturnDoubleCallback();
    return `${result}`;
};

export const reverseNoParamReturnFunction = () => {
    const result = master.NoParamReturnFunctionCallback();
    return result ? `${result()}` : '<null function pointer>';
};

export const reverseNoParamReturnString = () => {
    const result = master.NoParamReturnStringCallback();
    return result;
};

export const reverseNoParamReturnAny = () => {
    const result = master.NoParamReturnAnyCallback();
    return result;
};

export const reverseNoParamReturnArrayBool = () => {
    const result = master.NoParamReturnArrayBoolCallback();
    return vectorToString(result, boolStr);
};

export const reverseNoParamReturnArrayChar8 = () => {
    const result = master.NoParamReturnArrayChar8Callback();
    return vectorToString(result, char8Str);
};

export const reverseNoParamReturnArrayChar16 = () => {
    const result = master.NoParamReturnArrayChar16Callback();
    return vectorToString(result, char16Str);
};

export const reverseNoParamReturnArrayInt8 = () => {
    const result = master.NoParamReturnArrayInt8Callback();
    return vectorToString(result);
};

export const reverseNoParamReturnArrayInt16 = () => {
    const result = master.NoParamReturnArrayInt16Callback();
    return vectorToString(result);
};

export const reverseNoParamReturnArrayInt32 = () => {
    const result = master.NoParamReturnArrayInt32Callback();
    return vectorToString(result);
};

export const reverseNoParamReturnArrayInt64 = () => {
    const result = master.NoParamReturnArrayInt64Callback();
    return vectorToString(result);
};

export const reverseNoParamReturnArrayUInt8 = () => {
    const result = master.NoParamReturnArrayUInt8Callback();
    return vectorToString(result);
};

export const reverseNoParamReturnArrayUInt16 = () => {
    const result = master.NoParamReturnArrayUInt16Callback();
    return vectorToString(result);
};

export const reverseNoParamReturnArrayUInt32 = () => {
    const result = master.NoParamReturnArrayUInt32Callback();
    return vectorToString(result);
};

export const reverseNoParamReturnArrayUInt64 = () => {
    const result = master.NoParamReturnArrayUInt64Callback();
    return vectorToString(result);
};

export const reverseNoParamReturnArrayPointer = () => {
    const result = master.NoParamReturnArrayPointerCallback();
    return vectorToString(result, ptrStr);
};

export const reverseNoParamReturnArrayFloat = () => {
    const result = master.NoParamReturnArrayFloatCallback();
    return vectorToString(result, floatStr);
};

export const reverseNoParamReturnArrayDouble = () => {
    const result = master.NoParamReturnArrayDoubleCallback();
    return vectorToString(result);
};

export const reverseNoParamReturnArrayString = () => {
    const result = master.NoParamReturnArrayStringCallback();
    return vectorToString(result, quoteStr);
};

export const reverseNoParamReturnArrayAny = () => {
    const result = master.NoParamReturnArrayAnyCallback();
    return vectorToString(result, plainStr);
};

export const reverseNoParamReturnArrayVec2 = () => {
    const result = master.NoParamReturnArrayVec2Callback();
    return vectorToString(result);
};

export const reverseNoParamReturnArrayVec3 = () => {
    const result = master.NoParamReturnArrayVec3Callback();
    return vectorToString(result);
};

export const reverseNoParamReturnArrayVec4 = () => {
    const result = master.NoParamReturnArrayVec4Callback();
    return vectorToString(result);
};

export const reverseNoParamReturnArrayMat4x4 = () => {
    const result = master.NoParamReturnArrayMat4x4Callback();
    return vectorToString(result);
};

export const reverseNoParamReturnVector2 = () => {
    const result = master.NoParamReturnVector2Callback();
    return podToString(result);
};

export const reverseNoParamReturnVector3 = () => {
    const result = master.NoParamReturnVector3Callback();
    return podToString(result);
};

export const reverseNoParamReturnVector4 = () => {
    const result = master.NoParamReturnVector4Callback();
    return podToString(result);
};

export const reverseNoParamReturnMatrix4x4 = () => {
    const result = master.NoParamReturnMatrix4x4Callback();
    return podToString(result);
};

export const reverseParam1 = () => {
    master.Param1Callback(999);
};

export const reverseParam2 = () => {
    master.Param2Callback(888, 9.9);
};

export const reverseParam3 = () => {
    master.Param3Callback(777, 8.8, 9.8765);
};

export const reverseParam4 = () => {
    master.Param4Callback(666, 7.7, 8.7659, new Vector4(100.1, 200.2, 300.3, 400.4));
};

export const reverseParam5 = () => {
    master.Param5Callback(555, 6.6, 7.6598, new Vector4(-105.1, -205.2, -305.3, -405.4), []);
};

export const reverseParam6 = () => {
    master.Param6Callback(444, 5.5, 6.5987, new Vector4(110.1, 210.2, 310.3, 410.4), [90000, -100, 20000], 'A');
};

export const reverseParam7 = () => {
    master.Param7Callback(333, 4.4, 5.9876, new Vector4(-115.1, -215.2, -315.3, -415.4), [800000, 30000, -4000000], 'B', 'red gold');
};

export const reverseParam8 = () => {
    master.Param8Callback(222, 3.3, 1.2345, new Vector4(120.1, 220.2, 320.3, 420.4), [7000000, 5000000, -600000000], 'C', 'blue ice', 'Z');
};

export const reverseParam9 = () => {
    master.Param9Callback(111, 2.2, 5.1234, new Vector4(-125.1, -225.2, -325.3, -425.4), [60000000, -700000000, 80000000000], 'D', 'pink metal', 'Y', -100);
};

export const reverseParam10 = () => {
    master.Param10Callback(1234, 1.1, 4.5123, new Vector4(130.1, 230.2, 330.3, 430.4), [500000000, 90000000000, 1000000000000], 'E', 'green wood', 'X', -200, 0xabeba);
};

export const reverseParamRef1 = () => {
    const [, a] = master.ParamRef1Callback(0);
    return `${a}`;
};

export const reverseParamRef2 = () => {
    const [, a, b] = master.ParamRef2Callback(0, 0.0);
    return `${a}|${floatStr(b)}`;
};

export const reverseParamRef3 = () => {
    const [, a, b, c] = master.ParamRef3Callback(0, 0.0, 0.0);
    return `${a}|${floatStr(b)}|${c}`;
};

export const reverseParamRef4 = () => {
    const [, a, b, c, d] = master.ParamRef4Callback(0, 0.0, 0.0, new Vector4());
    return `${a}|${floatStr(b)}|${c}|${podToString(d)}`;
};

export const reverseParamRef5 = () => {
    const [, a, b, c, d, e] = master.ParamRef5Callback(0, 0.0, 0.0, new Vector4(), []);
    return `${a}|${floatStr(b)}|${c}|${podToString(d)}|${vectorToString(e)}`;
};

export const reverseParamRef6 = () => {
    const [, a, b, c, d, e, f] = master.ParamRef6Callback(0, 0.0, 0.0, new Vector4(), [], '');
    return `${a}|${floatStr(b)}|${c}|${podToString(d)}|${vectorToString(e)}|${ordZero(f)}`;
};

export const reverseParamRef7 = () => {
    const [, a, b, c, d, e, f, g] = master.ParamRef7Callback(0, 0.0, 0.0, new Vector4(), [], '', '');
    return `${a}|${floatStr(b)}|${c}|${podToString(d)}|${vectorToString(e)}|${ordZero(f)}|${g}`;
};

export const reverseParamRef8 = () => {
    const [, a, b, c, d, e, f, g, h] = master.ParamRef8Callback(0, 0.0, 0.0, new Vector4(), [], '', '', '');
    return `${a}|${floatStr(b)}|${c}|${podToString(d)}|${vectorToString(e)}|${ordZero(f)}|${g}|${ordZero(h)}`;
};

export const reverseParamRef9 = () => {
    const [, a, b, c, d, e, f, g, h, k] = master.ParamRef9Callback(0, 0.0, 0.0, new Vector4(), [], '', '', '', 0);
    return `${a}|${floatStr(b)}|${c}|${podToString(d)}|${vectorToString(e)}|${ordZero(f)}|${g}|${ordZero(h)}|${k}`;
};

export const reverseParamRef10 = () => {
    const [, a, b, c, d, e, f, g, h, k, l] = master.ParamRef10Callback(0, 0.0, 0.0, new Vector4(), [], '', '', '', 0, 0);
    return `${a}|${floatStr(b)}|${c}|${podToString(d)}|${vectorToString(e)}|${ordZero(f)}|${g}|${ordZero(h)}|${k}|${ptrStr(l)}`;
};

export const reverseParamRefVectors = () => {
    const [, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15] = master.ParamRefVectorsCallback(
        [true], ['A'], ['A'], [-1], [-1], [-1], [-1], [0], [0], [0], [0], [0], [1.0], [1.0], ['Hi']
    );
    const p15Formatted = p15.map(v => `'${v}'`).join(', ');
    return `${vectorToString(p1, boolStr)}|${vectorToString(p2, char8Str)}|${vectorToString(p3, char16Str)}|` +
           `${vectorToString(p4)}|${vectorToString(p5)}|${vectorToString(p6)}|${vectorToString(p7)}|` +
           `${vectorToString(p8)}|${vectorToString(p9)}|${vectorToString(p10)}|${vectorToString(p11)}|` +
           `${vectorToString(p12, ptrStr)}|${vectorToString(p13, floatStr)}|${vectorToString(p14)}|` +
           `${vectorToString(p15, quoteStr)}`;
};

export const reverseParamAllPrimitives = () => {
    const result = master.ParamAllPrimitivesCallback(true, '%', '☢', -1, -1000, -1000000, -1000000000000,
        200, 50000, 3000000000, 9999999999, 0xfedcbaabcdef, 0.001, 987654.456789);
    return `${result}`;
};

export const reverseParamEnum = () => {
    const e = master.Example;
    const result = master.ParamEnumCallback(e.Forth, [e.First, e.Second, e.Third]);
    return `${result}`;
};

export const reverseParamEnumRef = () => {
    const e = master.Example;
    const [result, p1, p2] = master.ParamEnumRefCallback(e.First, [e.First, e.First, e.Second]);
    return `${result}|${enumStr(p1)}|${vectorToString(p2, enumStr)}`;
};

export const reverseParamVariant = () => {
    const p1 = 'my custom string with enough chars';
    const p2 = ['X', '☢', -1, -1000, -1000000, -1000000000000, 200, 50000, 3000000000, 9999999999, 0xfedcbaabcdef, 0.001, 987654.456789];
    master.ParamVariantCallback(p1, p2);
};

export const reverseParamVariantRef = () => {
    const [, p1, p2] = master.ParamVariantRefCallback('my custom string with enough chars', 
        ['X', '☢', -1, -1000, -1000000, -1000000000000, 200, 50000, 3000000000, 9999999999, 0xfedcbaabcdef, 0.001, 987654.456789]);
    return `${vectorToString(p1)}|{${boolStr(p2[0])}, ${floatStr(p2[1])}, ${p2[2]}}`;
};

export class CallbackHolder {
    static mockVoid() {
        // No operation
    }

    static mockBool() {
        return true;
    }

    static mockChar8() {
        return 'A';
    }

    static mockChar16() {
        return 'Z';
    }

    static mockInt8() {
        return 10;
    }

    static mockInt16() {
        return 100;
    }

    static mockInt32() {
        return 1000;
    }

    static mockInt64() {
        return 10000;
    }

    static mockUint8() {
        return 20;
    }

    static mockUint16() {
        return 200;
    }

    static mockUint32() {
        return 2000;
    }

    static mockUint64() {
        return 20000;
    }

    static mockPtr() {
        return 0;
    }

    static mockFloat() {
        return 3.14;
    }

    static mockDouble() {
        return 6.28;
    }

    static mockFunction() {
        return 2;
    }

    static mockString() {
        return 'Test string';
    }

    static mockAny() {
        return 'A';
    }

    static mockBoolArray() {
        return [true, false];
    }

    static mockChar8Array() {
        return ['A', 'B'];
    }

    static mockChar16Array() {
        return ['A', 'B'];
    }

    static mockInt8Array() {
        return [10, 20];
    }

    static mockInt16Array() {
        return [100, 200];
    }

    static mockInt32Array() {
        return [1000, 2000];
    }

    static mockInt64Array() {
        return [10000, 20000];
    }

    static mockUInt8Array() {
        return [20, 30];
    }

    static mockUInt16Array() {
        return [200, 300];
    }

    static mockUInt32Array() {
        return [2000, 3000];
    }

    static mockUInt64Array() {
        return [20000, 30000];
    }

    static mockPtrArray() {
        return [0, 1];
    }

    static mockFloatArray() {
        return [1.1, 2.2];
    }

    static mockDoubleArray() {
        return [3.3, 4.4];
    }

    static mockStringArray() {
        return ['Hello', 'World'];
    }

    static mockAnyArray() {
        return ['Hello', 3.14, 6.28, 1, 0xdeadbeaf];
    }

    static mockVec2Array() {
        return [
            new Vector2(0.5, -1.2),
            new Vector2(3.4, 7.8),
            new Vector2(-6.7, 2.3),
            new Vector2(8.9, -4.5),
            new Vector2(0.0, 0.0)
        ];
    }

    static mockVec3Array() {
        return [
            new Vector3(2.1, 3.2, 4.3),
            new Vector3(-5.4, 6.5, -7.6),
            new Vector3(8.7, 9.8, 0.1),
            new Vector3(1.2, -3.3, 4.4),
            new Vector3(-5.5, 6.6, -7.7)
        ];
    }

    static mockVec4Array() {
        return [
            new Vector4(0.1, 1.2, 2.3, 3.4),
            new Vector4(-4.5, 5.6, 6.7, -7.8),
            new Vector4(8.9, -9.0, 10.1, -11.2),
            new Vector4(12.3, 13.4, 14.5, 15.6),
            new Vector4(-16.7, 17.8, 18.9, -19.0)
        ];
    }

    static mockMat4x4Array() {
        return [
            // Identity matrix
            new Matrix4x4(),
            // Random matrix #1
            new Matrix4x4([
                [0.5, 1.0, 1.5, 2.0],
                [2.5, 3.0, 3.5, 4.0],
                [4.5, 5.0, 5.5, 6.0],
                [6.5, 7.0, 7.5, 8.0]
            ]),
            // Random matrix #2
            new Matrix4x4([
                [-1.0, -2.0, -3.0, -4.0],
                [-5.0, -6.0, -7.0, -8.0],
                [-9.0, -10.0, -11.0, -12.0],
                [-13.0, -14.0, -15.0, -16.0]
            ]),
            // Random matrix #3
            new Matrix4x4([
                [1.1, 2.2, 3.3, 4.4],
                [5.5, 6.6, 7.7, 8.8],
                [9.9, 10.0, 11.1, 12.2],
                [13.3, 14.4, 15.5, 16.6]
            ])
        ];
    }

    static mockVec2() {
        return new Vector2(1.0, 2.0);
    }

    static mockVec3() {
        return new Vector3(1.0, 2.0, 3.0);
    }

    static mockVec4() {
        return new Vector4(1.0, 2.0, 3.0, 4.0);
    }

    static mockMat4x4() {
        const mat = Matrix4x4.zero();
        mat.m[0][0] = 1.0;
        return mat;
    }

    static mockFunc1(vec3) {
        return Math.floor(vec3.x + vec3.y + vec3.z);
    }

    static mockFunc2(a, b) {
        return '&';
    }

    static mockFunc3(p, v, s) {
        // No operation
    }

    static mockFunc4(flag, u, c, m) {
        return new Vector4(1.0, 2.0, 3.0, 4.0);
    }

    static mockFunc5(i, v, p, d, vec) {
        return true;
    }

    static mockFunc6(s, f, vec, i, u_vec, p) {
        return Math.floor(f + i);
    }

    static mockFunc7(vec, u, c, u_vec, v, flag, l) {
        return 3.14;
    }

    static mockFunc8(v, u_vec, i, flag, v4, c_vec, c, a) {
        return Matrix4x4.zero();
    }

    static mockFunc9(f, v, i_vec, l, flag, s, v4, i, p) {
        // No operation
    }

    static mockFunc10(v4, m, u_vec, l, c_vec, a, flag, v, i, d) {
        return 42;
    }

    static mockFunc11(b_vec, c, u, d, v3, i_vec, i, u16, f, v, u32) {
        return 0;
    }

    static mockFunc12(p, d_vec, u, d, flag, a, i, l, f, p_vec, i64, c) {
        return false;
    }

    static mockFunc13(i64, c_vec, u16, f, b_vec, v4, s, a, v3, p, v2, u8_vec, i16) {
        return 'Dummy String';
    }

    static mockFunc14(c_vec, u_vec, m, flag, c, a, f_vec, u16, u8_vec, i8, v3, v4, d, p) {
        return ['String1', 'String2'];
    }

    static mockFunc15(i_vec, m, v4, p, l, u_vec, flag, f, c_vec, u, a, v2, u16, d, u8_vec) {
        return 257;
    }

    static mockFunc16(b_vec, i16, i_vec, v4, m, v2, u_vec, c_vec, s, i64, u32_vec, v3, f, d, i8, u16) {
        return 0;
    }

    static mockFunc17(ref_val) {
        ref_val += 10;
        return [null, ref_val];
    }

    static mockFunc18(i8, i16) {
        i8 = 5;
        i16 = 10;
        return [new Vector2(5.0, 10.0), i8, i16];
    }

    static mockFunc19(u32, v3, u_vec) {
        u32 = 42;
        v3 = new Vector3(1.0, 2.0, 3.0);
        u_vec = [1, 2, 3];
        return [null, u32, v3, u_vec];
    }

    static mockFunc20(c, v4, u_vec, ch) {
        c = 't';
        v4 = new Vector4(1.0, 2.0, 3.0, 4.0);
        u_vec = [100, 200];
        ch = 'F';
        return [0, c, v4, u_vec, ch];
    }

    static mockFunc21(m, i_vec, v2, flag, d) {
        flag = true;
        d = 3.14;
        v2 = new Vector2(1.0, 2.0);
        m = new Matrix4x4([
            [1.3, 0.6, 0.8, 0.5],
            [0.7, 1.1, 0.2, 0.4],
            [0.9, 0.3, 1.2, 0.7],
            [0.2, 0.8, 0.5, 1.0]
        ]);
        i_vec = [1, 2, 3];
        return [0.0, m, i_vec, v2, flag, d];
    }

    static mockFunc22(p, u32, d_vec, i16, s, v4) {
        p = 0;
        u32 = 99;
        i16 = 123;
        s = 'Hello';
        v4 = new Vector4(1.0, 2.0, 3.0, 4.0);
        d_vec = [1.1, 2.2, 3.3];
        return [0, p, u32, d_vec, i16, s, v4];
    }

    static mockFunc23(u64, v2, i_vec, c, f, i8, u8_vec) {
        u64 = 50;
        f = 1.5;
        i8 = -1;
        v2 = new Vector2(3.0, 4.0);
        u8_vec = [1, 2, 3];
        c = 'Ⅴ';
        i_vec = [1, 2, 3, 4];
        return [5, u64, v2, i_vec, c, f, i8, u8_vec];
    }

    static mockFunc24(c_vec, i64, u8_vec, v4, u64, p_vec, d, v_vec) {
        i64 = 64;
        d = 2.71;
        v4 = new Vector4(1.0, 2.0, 3.0, 4.0);
        c_vec = ['a', 'b', 'c'];
        u8_vec = [5, 6, 7];
        p_vec = [0];
        v_vec = [1, 1, 2, 2];
        u64 = 0xffffffff;
        return [Matrix4x4.zero(), c_vec, i64, u8_vec, v4, u64, p_vec, d, v_vec];
    }

    static mockFunc25(i32, p_vec, flag, u8, s, v3, i64, v4, u16) {
        flag = false;
        i32 = 100;
        u8 = 250;
        v3 = new Vector3(1.0, 2.0, 3.0);
        v4 = new Vector4(4.0, 5.0, 6.0, 7.0);
        s = 'MockFunc25';
        p_vec = [0];
        i64 = 1337;
        u16 = 64222;
        return [0.0, i32, p_vec, flag, u8, s, v3, i64, v4, u16];
    }

    static mockFunc26(c, v2, m, f_vec, i16, u64, u32, u16_vec, p, flag) {
        c = 'Z';
        flag = true;
        v2 = new Vector2(2.0, 3.0);
        m = new Matrix4x4([
            0.9, 0.2, 0.4, 0.8,
            0.1, 1.0, 0.6, 0.3,
            0.7, 0.5, 0.2, 0.9,
            0.3, 0.4, 1.5, 0.1
        ]);
        f_vec = [1.1, 2.2];
        u64 = 64;
        u32 = 32;
        u16_vec = [100, 200];
        i16 = 332;
        p = 0xDEADBEAFDEADBEAF;
        return ['A', c, v2, m, f_vec, i16, u64, u32, u16_vec, p, flag];
    }

    static mockFunc27(f, v3, p, v2, i16_vec, m, flag, v4, i8, i32, u8_vec) {
        f = 1.0;
        v3 = new Vector3(-1.0, -2.0, -3.0);
        p = 0xDEADBEAFDEADBEAF;
        v2 = new Vector2(-111.0, 111.0);
        i16_vec = [1, 2, 3, 4];
        m = new Matrix4x4([
            1.0, 0.5, 0.3, 0.7,
            0.8, 1.2, 0.6, 0.9,
            1.5, 1.1, 0.4, 0.2,
            0.3, 0.9, 0.7, 1.0
        ]);
        flag = true;
        v4 = new Vector4(1.0, 2.0, 3.0, 4.0);
        i8 = 111;
        i32 = 30;
        u8_vec = [0, 0, 0, 0, 0, 0, 1, 0];
        return [0, f, v3, p, v2, i16_vec, m, flag, v4, i8, i32, u8_vec];
    }

    static mockFunc28(ptr, u16, u32_vec, m, f, v4, s, u64_vec, i64, b, vec3, f_vec) {
        ptr = 0;
        u16 = 65500;
        u32_vec = [1, 2, 3, 4, 5, 7];
        m = new Matrix4x4([
            1.4, 0.7, 0.2, 0.5,
            0.3, 1.1, 0.6, 0.8,
            0.9, 0.4, 1.3, 0.1,
            0.6, 0.2, 0.7, 1.0
        ]);
        f = 5.5;
        v4 = new Vector4(1.0, 2.0, 3.0, 4.0);
        u64_vec = [1, 2];
        i64 = 834748377834;
        b = true;
        vec3 = new Vector3(10.0, 20.0, 30.0);
        s = 'MockFunc28';
        f_vec = [1.0, -1000.0, 2000.0];
        return [s, ptr, u16, u32_vec, m, f, v4, s, u64_vec, i64, b, vec3, f_vec];
    }

    static mockFunc29(v4, i32, i_vec, d, flag, i8, u16_vec, f, s, m, u64, v3, i64_vec) {
        i32 = 30;
        flag = true;
        v4 = new Vector4(1.0, 2.0, 3.0, 4.0);
        d = 3.14;
        i8 = 8;
        u16_vec = [100, 200];
        f = 1.5;
        s = 'MockFunc29';
        m = new Matrix4x4([
            [0.4, 1.0, 0.6, 0.3],
            [1.2, 0.8, 0.5, 0.9],
            [0.7, 0.3, 1.4, 0.6],
            [0.1, 0.9, 0.8, 1.3]
        ]);
        u64 = 64;
        v3 = new Vector3(1.0, 2.0, 3.0);
        i64_vec = [1, 2, 3];
        i_vec = [127, 126, 125];
        return [['Example', 'MockFunc29'], v4, i32, i_vec, d, flag, i8, u16_vec, f, s, m, u64, v3, i64_vec];
    }

    static mockFunc30(p, v4, i64, u_vec, flag, s, v3, u8_vec, f, v2, m, i8, v_vec, d) {
        flag = false;
        f = 1.1;
        i64 = 1000;
        v2 = new Vector2(3.0, 4.0);
        v4 = new Vector4(1.0, 2.0, 3.0, 4.0);
        s = 'MockFunc30';
        p = 0;
        u_vec = [100, 200];
        m = new Matrix4x4([
            [0.5, 0.3, 1.0, 0.7],
            [1.1, 0.9, 0.6, 0.4],
            [0.2, 0.8, 1.5, 0.3],
            [0.7, 0.4, 0.9, 1.0]
        ]);
        i8 = 8;
        v_vec = [1.0, 1.0, 2.0, 2.0];
        d = 2.718;
        v3 = new Vector3(1.0, 2.0, 3.0);
        u8_vec = [255, 0, 255, 200, 100, 200];
        return [42, p, v4, i64, u_vec, flag, s, v3, u8_vec, f, v2, m, i8, v_vec, d];
    }

    static mockFunc31(c, u32, u_vec, v4, s, flag, i64, v2, i8, u16, i_vec, m, v3, f, v4_vec) {
        u32 = 12345;
        flag = true;
        v3 = new Vector3(1.0, 2.0, 3.0);
        s = 'MockFunc31';
        v2 = new Vector2(5.0, 6.0);
        i8 = 7;
        u16 = 255;
        m = new Matrix4x4([
            [0.8, 0.5, 1.2, 0.3],
            [1.0, 0.7, 0.4, 0.6],
            [0.9, 0.2, 0.5, 1.4],
            [0.6, 0.8, 1.1, 0.7]
        ]);
        i_vec = [1, 2];
        v4 = new Vector4(1.0, 2.0, 3.0, 4.0);
        i64 = 123456789;
        c = 'C';
        v4_vec = [1.0, 1.0, 1.0, 1.0, 2.0, 2.0, 2.0, 2.0];
        u_vec = [1, 2, 3, 4, 5];
        f = -1.0;
        return [new Vector3(1.0, 2.0, 3.0), c, u32, u_vec, v4, s, flag, i64, v2, i8, u16, i_vec, m, v3, f, v4_vec];
    }

    static mockFunc32(i32, u16, i_vec, v4, p, u_vec, m, u64, s, i64, v2, u8_vec, flag, v3, u8, c_vec) {
        i32 = 42;
        u16 = 255;
        flag = false;
        v2 = new Vector2(2.5, 3.5);
        u8_vec = [1, 2, 3, 4, 5, 9];
        v4 = new Vector4(4.0, 5.0, 6.0, 7.0);
        s = 'MockFunc32';
        p = 0;
        m = new Matrix4x4([
            [1.0, 0.4, 0.3, 0.9],
            [0.7, 1.2, 0.5, 0.8],
            [0.2, 0.6, 1.1, 0.4],
            [0.9, 0.3, 0.8, 1.5]
        ]);
        u64 = 123456789;
        u_vec = [100, 200];
        i64 = 1000;
        v3 = new Vector3(0.0, 0.0, 0.0);
        u8 = 8;
        c_vec = ['a', 'b', 'c'];
        i_vec = [0, 1];
        return [1.0, i32, u16, i_vec, v4, p, u_vec, m, u64, s, i64, v2, u8_vec, flag, v3, u8, c_vec];
    }

    static mockFunc33(variant) {
        variant = "MockFunc33";
        return [null, variant];
    }

    static mockFuncEnum(p1, p2) {
        const e = master.Example;
        p2 = [e.First, e.Second, e.Third];
        return [[p1, e.Forth], p2];
    }
}

export const reverseCallFuncVoid = () => {
    master.CallFuncVoidCallback(CallbackHolder.mockVoid);
    return '';
};

export const reverseCallFuncBool = () => {
    const result = master.CallFuncBoolCallback(CallbackHolder.mockBool);
    return boolStr(result);
};

export const reverseCallFuncChar8 = () => {
    const result = master.CallFuncChar8Callback(CallbackHolder.mockChar8);
    return `${ordZero(result)}`;
};

export const reverseCallFuncChar16 = () => {
    const result = master.CallFuncChar16Callback(CallbackHolder.mockChar16);
    return `${ordZero(result)}`;
};

export const reverseCallFuncInt8 = () => {
    const result = master.CallFuncInt8Callback(CallbackHolder.mockInt8);
    return String(result);
};

export const reverseCallFuncInt16 = () => {
    const result = master.CallFuncInt16Callback(CallbackHolder.mockInt16);
    return String(result);
};

export const reverseCallFuncInt32 = () => {
    const result = master.CallFuncInt32Callback(CallbackHolder.mockInt32);
    return String(result);
};

export const reverseCallFuncInt64 = () => {
    const result = master.CallFuncInt64Callback(CallbackHolder.mockInt64);
    return String(result);
};

export const reverseCallFuncUint8 = () => {
    const result = master.CallFuncUInt8Callback(CallbackHolder.mockUint8);
    return String(result);
};

export const reverseCallFuncUint16 = () => {
    const result = master.CallFuncUInt16Callback(CallbackHolder.mockUint16);
    return String(result);
};

export const reverseCallFuncUint32 = () => {
    const result = master.CallFuncUInt32Callback(CallbackHolder.mockUint32);
    return String(result);
};

export const reverseCallFuncUint64 = () => {
    const result = master.CallFuncUInt64Callback(CallbackHolder.mockUint64);
    return String(result);
};

export const reverseCallFuncPtr = () => {
    const result = master.CallFuncPtrCallback(CallbackHolder.mockPtr);
    return ptrStr(result);
};

export const reverseCallFuncFloat = () => {
    const result = master.CallFuncFloatCallback(CallbackHolder.mockFloat);
    return floatStr(result);
};

export const reverseCallFuncDouble = () => {
    const result = master.CallFuncDoubleCallback(CallbackHolder.mockDouble);
    return String(result);
};

export const reverseCallFuncString = () => {
    const result = master.CallFuncStringCallback(CallbackHolder.mockString);
    return result;
};

export const reverseCallFuncAny = () => {
    const result = master.CallFuncAnyCallback(CallbackHolder.mockAny);
    return result;
};

export const reverseCallFuncBoolVector = () => {
    const result = master.CallFuncBoolVectorCallback(CallbackHolder.mockBoolArray);
    return vectorToString(result, boolStr);
};

export const reverseCallFuncChar8Vector = () => {
    const result = master.CallFuncChar8VectorCallback(CallbackHolder.mockChar8Array);
    return vectorToString(result, char8Str);
};

export const reverseCallFuncChar16Vector = () => {
    const result = master.CallFuncChar16VectorCallback(CallbackHolder.mockChar16Array);
    return vectorToString(result, char16Str);
};

export const reverseCallFuncInt8Vector = () => {
    const result = master.CallFuncInt8VectorCallback(CallbackHolder.mockInt8Array);
    return vectorToString(result);
};

export const reverseCallFuncInt16Vector = () => {
    const result = master.CallFuncInt16VectorCallback(CallbackHolder.mockInt16Array);
    return vectorToString(result);
};

export const reverseCallFuncInt32Vector = () => {
    const result = master.CallFuncInt32VectorCallback(CallbackHolder.mockInt32Array);
    return vectorToString(result);
};

export const reverseCallFuncInt64Vector = () => {
    const result = master.CallFuncInt64VectorCallback(CallbackHolder.mockInt64Array);
    return vectorToString(result);
};

export const reverseCallFuncUInt8Vector = () => {
    const result = master.CallFuncUInt8VectorCallback(CallbackHolder.mockUInt8Array);
    return vectorToString(result);
};

export const reverseCallFuncUInt16Vector = () => {
    const result = master.CallFuncUInt16VectorCallback(CallbackHolder.mockUInt16Array);
    return vectorToString(result);
};

export const reverseCallFuncUInt32Vector = () => {
    const result = master.CallFuncUInt32VectorCallback(CallbackHolder.mockUInt32Array);
    return vectorToString(result);
};

export const reverseCallFuncUInt64Vector = () => {
    const result = master.CallFuncUInt64VectorCallback(CallbackHolder.mockUInt64Array);
    return vectorToString(result);
};

export const reverseCallFuncPtrVector = () => {
    const result = master.CallFuncPtrVectorCallback(CallbackHolder.mockPtrArray);
    return vectorToString(result, ptrStr);
};

export const reverseCallFuncFloatVector = () => {
    const result = master.CallFuncFloatVectorCallback(CallbackHolder.mockFloatArray);
    return vectorToString(result, floatStr);
};

export const reverseCallFuncDoubleVector = () => {
    const result = master.CallFuncDoubleVectorCallback(CallbackHolder.mockDoubleArray);
    return vectorToString(result);
};

export const reverseCallFuncStringVector = () => {
    const result = master.CallFuncStringVectorCallback(CallbackHolder.mockStringArray);
    return vectorToString(result, quoteStr);
};

export const reverseCallFuncAnyVector = () => {
    const result = master.CallFuncAnyVectorCallback(CallbackHolder.mockAnyArray);
    return vectorToString(result, plainStr);
};

export const reverseCallFuncVec2Vector = () => {
    const result = master.CallFuncVec2VectorCallback(CallbackHolder.mockVec2Array);
    return vectorToString(result, podToString);
};

export const reverseCallFuncVec3Vector = () => {
    const result = master.CallFuncVec3VectorCallback(CallbackHolder.mockVec3Array);
    return vectorToString(result, podToString);
};

export const reverseCallFuncVec4Vector = () => {
    const result = master.CallFuncVec4VectorCallback(CallbackHolder.mockVec4Array);
    return vectorToString(result, podToString);
};

export const reverseCallFuncMat4x4Vector = () => {
    const result = master.CallFuncMat4x4VectorCallback(CallbackHolder.mockMat4x4Array);
    return vectorToString(result, podToString);
};

export const reverseCallFuncVec2 = () => {
    const result = master.CallFuncVec2Callback(CallbackHolder.mockVec2);
    return podToString(result);
};

export const reverseCallFuncVec3 = () => {
    const result = master.CallFuncVec3Callback(CallbackHolder.mockVec3);
    return podToString(result);
};

export const reverseCallFuncVec4 = () => {
    const result = master.CallFuncVec4Callback(CallbackHolder.mockVec4);
    return podToString(result);
};

export const reverseCallFuncMat4x4 = () => {
    const result = master.CallFuncMat4x4Callback(CallbackHolder.mockMat4x4);
    return podToString(result);
};

export const reverseCallFunc1 = () => {
    const result = master.CallFunc1Callback(CallbackHolder.mockFunc1);
    return String(result);
};

export const reverseCallFunc2 = () => {
    const result = master.CallFunc2Callback(CallbackHolder.mockFunc2);
    return char8Str(result);
};

export const reverseCallFunc3 = () => {
    master.CallFunc3Callback(CallbackHolder.mockFunc3);
    return '';
};

export const reverseCallFunc4 = () => {
    const result = master.CallFunc4Callback(CallbackHolder.mockFunc4);
    return podToString(result);
};

export const reverseCallFunc5 = () => {
    const result = master.CallFunc5Callback(CallbackHolder.mockFunc5);
    return boolStr(result);
};

export const reverseCallFunc6 = () => {
    const result = master.CallFunc6Callback(CallbackHolder.mockFunc6);
    return String(result);
};

export const reverseCallFunc7 = () => {
    const result = master.CallFunc7Callback(CallbackHolder.mockFunc7);
    return String(result);
};

export const reverseCallFunc8 = () => {
    const result = master.CallFunc8Callback(CallbackHolder.mockFunc8);
    return podToString(result);
};

export const reverseCallFunc9 = () => {
    master.CallFunc9Callback(CallbackHolder.mockFunc9);
    return '';
};

export const reverseCallFunc10 = () => {
    const result = master.CallFunc10Callback(CallbackHolder.mockFunc10);
    return String(result);
};

export const reverseCallFunc11 = () => {
    const result = master.CallFunc11Callback(CallbackHolder.mockFunc11);
    return ptrStr(result);
};

export const reverseCallFunc12 = () => {
    const result = master.CallFunc12Callback(CallbackHolder.mockFunc12);
    return boolStr(result);
};

export const reverseCallFunc13 = () => {
    const result = master.CallFunc13Callback(CallbackHolder.mockFunc13);
    return result;
};

export const reverseCallFunc14 = () => {
    const result = master.CallFunc14Callback(CallbackHolder.mockFunc14);
    return vectorToString(result, quoteStr);
};

export const reverseCallFunc15 = () => {
    const result = master.CallFunc15Callback(CallbackHolder.mockFunc15);
    return String(result);
};

export const reverseCallFunc16 = () => {
    const result = master.CallFunc16Callback(CallbackHolder.mockFunc16);
    return ptrStr(result);
};

export const reverseCallFunc17 = () => {
    const result = master.CallFunc17Callback(CallbackHolder.mockFunc17);
    return result;
};

export const reverseCallFunc18 = () => {
    const result = master.CallFunc18Callback(CallbackHolder.mockFunc18);
    return result;
};

export const reverseCallFunc19 = () => {
    const result = master.CallFunc19Callback(CallbackHolder.mockFunc19);
    return result;
};

export const reverseCallFunc20 = () => {
    const result = master.CallFunc20Callback(CallbackHolder.mockFunc20);
    return result;
};

export const reverseCallFunc21 = () => {
    const result = master.CallFunc21Callback(CallbackHolder.mockFunc21);
    return result;
};

export const reverseCallFunc22 = () => {
    const result = master.CallFunc22Callback(CallbackHolder.mockFunc22);
    return result;
};

export const reverseCallFunc23 = () => {
    const result = master.CallFunc23Callback(CallbackHolder.mockFunc23);
    return result;
};

export const reverseCallFunc24 = () => {
    const result = master.CallFunc24Callback(CallbackHolder.mockFunc24);
    return result;
};

export const reverseCallFunc25 = () => {
    const result = master.CallFunc25Callback(CallbackHolder.mockFunc25);
    return result;
};

export const reverseCallFunc26 = () => {
    const result = master.CallFunc26Callback(CallbackHolder.mockFunc26);
    return result;
};

export const reverseCallFunc27 = () => {
    const result = master.CallFunc27Callback(CallbackHolder.mockFunc27);
    return result;
};

export const reverseCallFunc28 = () => {
    const result = master.CallFunc28Callback(CallbackHolder.mockFunc28);
    return result;
};

export const reverseCallFunc29 = () => {
    const result = master.CallFunc29Callback(CallbackHolder.mockFunc29);
    return result;
};

export const reverseCallFunc30 = () => {
    const result = master.CallFunc30Callback(CallbackHolder.mockFunc30);
    return result;
};

export const reverseCallFunc31 = () => {
    const result = master.CallFunc31Callback(CallbackHolder.mockFunc31);
    return result;
};

export const reverseCallFunc32 = () => {
    const result = master.CallFunc32Callback(CallbackHolder.mockFunc32);
    return result;
};

export const reverseCallFunc33 = () => {
    const result = master.CallFunc33Callback(CallbackHolder.mockFunc33);
    return result;
};

export const reverseCallFuncEnum = () => {
    const result = master.CallFuncEnumCallback(CallbackHolder.mockFuncEnum);
    return result;
};

function log(message) {
    // Only logs in debug mode
    const VERBOSE = false;
    if (VERBOSE) {
        console.log(message);
    }
}

export const basicLifecycle = () => {
    log("TEST 1: Basic Lifecycle");
    log("───────────────────────");

    const initialAlive = master.ResourceHandle.GetAliveCount();
    const initialCreated = master.ResourceHandle.GetTotalCreated();

    // JavaScript doesn't have context managers, so we use try/finally
    const resource = new master.ResourceHandle(1, "Test1");
    try {
        log(`v Created ResourceHandle ID: ${resource.GetId()}`);
        log(`v Alive count increased: ${master.ResourceHandle.GetAliveCount()}`);
    } finally {
        // Manually trigger cleanup if needed
        // In JS, the object will be GC'd when no longer referenced
        resource.close();
    }

    const finalAlive = master.ResourceHandle.GetAliveCount();
    const finalCreated = master.ResourceHandle.GetTotalCreated();
    const finalDestroyed = master.ResourceHandle.GetTotalDestroyed();

    log(`v Destructor called, alive count: ${finalAlive}`);
    log(`v Total created: ${finalCreated - initialCreated}`);
    log(`v Total destroyed: ${finalDestroyed}`);

    if (finalAlive === initialAlive && finalCreated === finalDestroyed) {
        log("v TEST 1 PASSED: Lifecycle working correctly\n");
        return "true";
    } else {
        log("x TEST 1 FAILED: Lifecycle mismatch!\n");
        return "false";
    }
}

export const stateManagement = () => {
    log("TEST 2: State Management");
    log("────────────────────────");

    const resource = new master.ResourceHandle(2, "StateTest");

    resource.IncrementCounter();
    resource.IncrementCounter();
    resource.IncrementCounter();
    const counter = resource.GetCounter();
    log(`v Counter incremented 3 times: ${counter}`);

    resource.SetName("StateTestModified");
    const newName = resource.GetName();
    log(`v Name changed to: ${newName}`);

    resource.AddData(1.1);
    resource.AddData(2.2);
    resource.AddData(3.3);
    const data = resource.GetData();
    log(`v Added ${data.length} data points`);

    if (counter === 3 && newName === "StateTestModified" && data.length === 3) {
        log("v TEST 2 PASSED: State management working\n");
        return "true";
    } else {
        log("x TEST 2 FAILED: State not preserved!\n");
        return "false";
    }
}

export const multipleInstances = () => {
    log("TEST 3: Multiple Instances");
    log("──────────────────────────");

    const beforeAlive = master.ResourceHandle.GetAliveCount();

    const r1 = new master.ResourceHandle(10, "Instance1");
    const r2 = new master.ResourceHandle(20, "Instance2");
    const r3 = new master.ResourceHandle();

    const duringAlive = master.ResourceHandle.GetAliveCount();
    log(`v Created 3 instances, alive: ${duringAlive}`);
    log(`v R1 ID: ${r1.GetId()}, R2 ID: ${r2.GetId()}, R3 ID: ${r3.GetId()}`);

    if (duringAlive - beforeAlive === 3) {
        log("v All 3 instances tracked correctly");
    }

    // Let them go out of scope / trigger GC
    // In a real scenario, they'd be garbage collected
    r1.close();
    r2.close();
    r3.close();

    const afterAlive = master.ResourceHandle.GetAliveCount();

    if (afterAlive === beforeAlive) {
        log("v TEST 3 PASSED: All instances destroyed properly\n");
        return "true";
    } else {
        log(`x TEST 3 FAILED: Leak detected! Before: ${beforeAlive}, After: ${afterAlive}\n`);
        return "false";
    }
}

export const counterWithoutDestructor = () => {
    log("TEST 4: Counter (No Destructor)");
    log("────────────────────────────────");

    const counter = new master.Counter(100);
    log(`v Created Counter with value: ${counter.GetValue()}`);

    counter.Increment();
    counter.Increment();
    counter.Add(50);
    const value = counter.GetValue();
    log(`v After operations, value: ${value}`);

    const isPositive = counter.IsPositive();
    log(`v Is positive: ${isPositive}`);

    if (value === 152n && isPositive) {
        log("v TEST 4 PASSED: Counter operations working\n");
        return "true";
    } else {
        log("x TEST 4 FAILED: Counter operations incorrect\n");
        return "false";
    }
}

export const staticMethods = () => {
    log("TEST 5: Static Methods");
    log("──────────────────────");

    const alive = master.ResourceHandle.GetAliveCount();
    const created = master.ResourceHandle.GetTotalCreated();
    const destroyed = master.ResourceHandle.GetTotalDestroyed();
    log(`v ResourceHandle stats - Alive: ${alive}, Created: ${created}, Destroyed: ${destroyed}`);

    const cmp1 = master.Counter.Compare(100, 50);
    const cmp2 = master.Counter.Compare(50, 100);
    const cmp3 = master.Counter.Compare(50, 50);
    log(`v Counter.Compare(100, 50) = ${cmp1} (expected 1)`);
    log(`v Counter.Compare(50, 100) = ${cmp2} (expected -1)`);
    log(`v Counter.Compare(50, 50) = ${cmp3} (expected 0)`);

    const sumResult = master.Counter.Sum([1, 2, 3, 4, 5]);
    log(`v Counter.Sum([1,2,3,4,5]) = ${sumResult} (expected 15)`);

    if (cmp1 === 1 && cmp2 === -1 && cmp3 === 0 && sumResult === 15n) {
        log("v TEST 5 PASSED: Static methods working\n");
        return "true";
    } else {
        log("x TEST 5 FAILED: Static methods incorrect\n");
        return "false";
    }
}

export const memoryLeakDetection = () => {
    log("TEST 6: Memory Leak Detection");
    log("──────────────────────────────");

    const beforeAlive = master.ResourceHandle.GetAliveCount();

    let leaked = new master.ResourceHandle(999, "IntentionalLeak");
    log(`v Created resource ID: ${leaked.GetId()}`);
    leaked = null;

    // Force garbage collection if available
    if (global.gc) {
        global.gc();
    }

    const afterAlive = master.ResourceHandle.GetAliveCount();

    log(`v Before leak test: ${beforeAlive} alive`);
    log(`v After GC: ${afterAlive} alive`);

    if (afterAlive === beforeAlive) {
        log("v TEST 6 PASSED: Finalizer cleaned up leaked resource\n");
        return "true";
    } else {
        log("x TEST 6 FAILED: Resource still alive (will be cleaned at plugin shutdown)\n");
        return "false";
    }
}

export const exceptionHandling = () => {
    log("TEST 7: Exception Handling");
    log("──────────────────────────");

    const resource = new master.ResourceHandle(777, "ExceptionTest");

    // Simulate manual cleanup/invalidation
    resource.close();

    try {
        resource.GetId();
        log("x TEST 7 FAILED: No exception thrown!\n");
        return "false";
    } catch (ex) {
        log(`v Caught expected exception: ${ex.constructor.name}`);
        log("v TEST 7 PASSED: Exception handling working\n");
        return "true";
    }
}

export const ownershipTransfer = () => {
    log("TEST 8: Ownership Transfer (get + release)");
    log("─────────────────────────────────────────");

    const initialAlive = master.ResourceHandle.GetAliveCount();
    const initialCreated = master.ResourceHandle.GetTotalCreated();

    const resource = new master.ResourceHandle(42, "OwnershipTest");
    log(`v Created ResourceHandle ID: ${resource.GetId()}`);

    // Get internal wrapper (simulate internal pointer access)
    const wrapper = resource.get();
    log(`v get() returned internal wrapper: ${wrapper}`);

    // Release ownership
    const handle = resource.release();
    log(`v release() returned handle: ${handle}`);

    if (wrapper !== handle) {
        log(`x TEST 8 FAILED: get() did not return internal wrapper, got ${typeof wrapper}`);
        return "false";
    }

    try {
        resource.GetId();
        log("x TEST 8 FAILED: ResourceHandle still accessible after release()");
        return "false";
    } catch (ex) {
        log("v ResourceHandle is invalid after release()");
    }

    // Check that handle is now owned externally and alive count updated correctly
    const aliveAfterRelease = master.ResourceHandle.GetAliveCount();
    if (aliveAfterRelease !== initialAlive + 1) {
        log(`x TEST 8 FAILED: Alive count mismatch after release. Expected ${initialAlive + 1}, got ${aliveAfterRelease}`);
        return "false";
    }

    master.ResourceHandleDestroy(handle);

    log("v TEST 8 PASSED: Ownership transfer working correctly\n");
    return "true";
}

const reverseTest = {
	'NoParamReturnVoid': reverseNoParamReturnVoid,
	'NoParamReturnBool': reverseNoParamReturnBool,
	'NoParamReturnChar8': reverseNoParamReturnChar8,
	'NoParamReturnChar16': reverseNoParamReturnChar16,
	'NoParamReturnInt8': reverseNoParamReturnInt8,
	'NoParamReturnInt16': reverseNoParamReturnInt16,
	'NoParamReturnInt32': reverseNoParamReturnInt32,
	'NoParamReturnInt64': reverseNoParamReturnInt64,
	'NoParamReturnUInt8': reverseNoParamReturnUInt8,
	'NoParamReturnUInt16': reverseNoParamReturnUInt16,
	'NoParamReturnUInt32': reverseNoParamReturnUInt32,
	'NoParamReturnUInt64': reverseNoParamReturnUInt64,
	'NoParamReturnPointer': reverseNoParamReturnPointer,
	'NoParamReturnFloat': reverseNoParamReturnFloat,
	'NoParamReturnDouble': reverseNoParamReturnDouble,
	'NoParamReturnFunction': reverseNoParamReturnFunction,
	'NoParamReturnString': reverseNoParamReturnString,
	'NoParamReturnAny': reverseNoParamReturnAny,
	'NoParamReturnArrayBool': reverseNoParamReturnArrayBool,
	'NoParamReturnArrayChar8': reverseNoParamReturnArrayChar8,
	'NoParamReturnArrayChar16': reverseNoParamReturnArrayChar16,
	'NoParamReturnArrayInt8': reverseNoParamReturnArrayInt8,
	'NoParamReturnArrayInt16': reverseNoParamReturnArrayInt16,
	'NoParamReturnArrayInt32': reverseNoParamReturnArrayInt32,
	'NoParamReturnArrayInt64': reverseNoParamReturnArrayInt64,
	'NoParamReturnArrayUInt8': reverseNoParamReturnArrayUInt8,
	'NoParamReturnArrayUInt16': reverseNoParamReturnArrayUInt16,
	'NoParamReturnArrayUInt32': reverseNoParamReturnArrayUInt32,
	'NoParamReturnArrayUInt64': reverseNoParamReturnArrayUInt64,
	'NoParamReturnArrayPointer': reverseNoParamReturnArrayPointer,
	'NoParamReturnArrayFloat': reverseNoParamReturnArrayFloat,
	'NoParamReturnArrayDouble': reverseNoParamReturnArrayDouble,
	'NoParamReturnArrayString': reverseNoParamReturnArrayString,
	'NoParamReturnArrayAny': reverseNoParamReturnArrayAny,
	'NoParamReturnVector2': reverseNoParamReturnVector2,
	'NoParamReturnVector3': reverseNoParamReturnVector3,
	'NoParamReturnVector4': reverseNoParamReturnVector4,
	'NoParamReturnMatrix4x4': reverseNoParamReturnMatrix4x4,
	'Param1': reverseParam1,
	'Param2': reverseParam2,
	'Param3': reverseParam3,
	'Param4': reverseParam4,
	'Param5': reverseParam5,
	'Param6': reverseParam6,
	'Param7': reverseParam7,
	'Param8': reverseParam8,
	'Param9': reverseParam9,
	'Param10': reverseParam10,
	'ParamRef1': reverseParamRef1,
	'ParamRef2': reverseParamRef2,
	'ParamRef3': reverseParamRef3,
	'ParamRef4': reverseParamRef4,
	'ParamRef5': reverseParamRef5,
	'ParamRef6': reverseParamRef6,
	'ParamRef7': reverseParamRef7,
	'ParamRef8': reverseParamRef8,
	'ParamRef9': reverseParamRef9,
	'ParamRef10': reverseParamRef10,
	'ParamRefArrays': reverseParamRefVectors,
	'ParamAllPrimitives': reverseParamAllPrimitives,
	'ParamEnum': reverseParamEnum,
	'ParamEnumRef': reverseParamEnumRef,
	'ParamVariant': reverseParamVariant,
	'ParamVariantRef': reverseParamVariantRef,
	'CallFuncVoid': reverseCallFuncVoid,
	'CallFuncBool': reverseCallFuncBool,
	'CallFuncChar8': reverseCallFuncChar8,
	'CallFuncChar16': reverseCallFuncChar16,
	'CallFuncInt8': reverseCallFuncInt8,
	'CallFuncInt16': reverseCallFuncInt16,
	'CallFuncInt32': reverseCallFuncInt32,
	'CallFuncInt64': reverseCallFuncInt64,
	'CallFuncUInt8': reverseCallFuncUint8,
	'CallFuncUInt16': reverseCallFuncUint16,
	'CallFuncUInt32': reverseCallFuncUint32,
	'CallFuncUInt64': reverseCallFuncUint64,
	'CallFuncPtr': reverseCallFuncPtr,
	'CallFuncFloat': reverseCallFuncFloat,
	'CallFuncDouble': reverseCallFuncDouble,
	'CallFuncString': reverseCallFuncString,
	'CallFuncAny': reverseCallFuncAny,
	'CallFuncBoolVector': reverseCallFuncBoolVector,
	'CallFuncChar8Vector': reverseCallFuncChar8Vector,
	'CallFuncChar16Vector': reverseCallFuncChar16Vector,
	'CallFuncInt8Vector': reverseCallFuncInt8Vector,
	'CallFuncInt16Vector': reverseCallFuncInt16Vector,
	'CallFuncInt32Vector': reverseCallFuncInt32Vector,
	'CallFuncInt64Vector': reverseCallFuncInt64Vector,
	'CallFuncUInt8Vector': reverseCallFuncUInt8Vector,
	'CallFuncUInt16Vector': reverseCallFuncUInt16Vector,
	'CallFuncUInt32Vector': reverseCallFuncUInt32Vector,
	'CallFuncUInt64Vector': reverseCallFuncUInt64Vector,
	'CallFuncPtrVector': reverseCallFuncPtrVector,
	'CallFuncFloatVector': reverseCallFuncFloatVector,
	'CallFuncDoubleVector': reverseCallFuncDoubleVector,
	'CallFuncStringVector': reverseCallFuncStringVector,
	'CallFuncAnyVector': reverseCallFuncAnyVector,
	'CallFuncVec2Vector': reverseCallFuncVec2Vector,
	'CallFuncVec3Vector': reverseCallFuncVec3Vector,
	'CallFuncVec4Vector': reverseCallFuncVec4Vector,
	'CallFuncMat4x4Vector': reverseCallFuncMat4x4Vector,
	'CallFuncVec2': reverseCallFuncVec2,
	'CallFuncVec3': reverseCallFuncVec3,
	'CallFuncVec4': reverseCallFuncVec4,
	'CallFuncMat4x4': reverseCallFuncMat4x4,
	'CallFunc1': reverseCallFunc1,
	'CallFunc2': reverseCallFunc2,
	'CallFunc3': reverseCallFunc3,
	'CallFunc4': reverseCallFunc4,
	'CallFunc5': reverseCallFunc5,
	'CallFunc6': reverseCallFunc6,
	'CallFunc7': reverseCallFunc7,
	'CallFunc8': reverseCallFunc8,
	'CallFunc9': reverseCallFunc9,
	'CallFunc10': reverseCallFunc10,
	'CallFunc11': reverseCallFunc11,
	'CallFunc12': reverseCallFunc12,
	'CallFunc13': reverseCallFunc13,
	'CallFunc14': reverseCallFunc14,
	'CallFunc15': reverseCallFunc15,
	'CallFunc16': reverseCallFunc16,
	'CallFunc17': reverseCallFunc17,
	'CallFunc18': reverseCallFunc18,
	'CallFunc19': reverseCallFunc19,
	'CallFunc20': reverseCallFunc20,
	'CallFunc21': reverseCallFunc21,
	'CallFunc22': reverseCallFunc22,
	'CallFunc23': reverseCallFunc23,
	'CallFunc24': reverseCallFunc24,
	'CallFunc25': reverseCallFunc25,
	'CallFunc26': reverseCallFunc26,
	'CallFunc27': reverseCallFunc27,
	'CallFunc28': reverseCallFunc28,
	'CallFunc29': reverseCallFunc29,
	'CallFunc30': reverseCallFunc30,
	'CallFunc31': reverseCallFunc31,
	'CallFunc32': reverseCallFunc32,
	'CallFunc33': reverseCallFunc33,
	'CallFuncEnum': reverseCallFuncEnum,

    'ClassBasicLifecycle': basicLifecycle,
    'ClassStateManagement': stateManagement,
    'ClassMultipleInstances': multipleInstances,
    'ClassCounterWithoutDestructor': counterWithoutDestructor,
    'ClassStaticMethods': staticMethods,
    'ClassMemoryLeakDetection': memoryLeakDetection,
    'ClassExceptionHandling': exceptionHandling,
    'ClassOwnershipTransfer': ownershipTransfer,
};

export function reverseCall(test) {
    const result = reverseTest[test]();
    if (result !== undefined) {
        master.ReverseReturn(result);
    }
};
