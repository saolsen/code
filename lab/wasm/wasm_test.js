// @ts-check

// I should prototype the sharing of data between js and wasm first.
// I do think I'll still do it with emscripten.

/**
 * @param {number} bar
 * @returns {number}
 */
function foo(bar) {
    console.log(bar);
    return bar + 1;
}

console.log("hello");
foo(1);

Module['onRuntimeInitialized'] = () => {
    const result = Module.ccall('foo', 'number', ['number'], [1]);
    console.log('Result:', result);
};
