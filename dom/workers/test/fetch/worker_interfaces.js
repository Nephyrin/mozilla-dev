function ok(a, msg) {
  dump("OK: " + !!a + "  =>  " + a + " " + msg + "\n");
  postMessage({type: 'status', status: !!a, msg: a + ": " + msg });
}

function is(a, b, msg) {
  dump("IS: " + (a===b) + "  =>  " + a + " | " + b + " " + msg + "\n");
  postMessage({type: 'status', status: a === b, msg: a + " === " + b + ": " + msg });
}

function isnot(a, b, msg) {
  dump("ISNOT: " + (a!==b) + "  =>  " + a + " | " + b + " " + msg + "\n");
  postMessage({type: 'status', status: a !== b, msg: a + " !== " + b + ": " + msg });
}

onmessage = function() {
  ok(typeof HeaderMap === "function", "HeaderMap should be defined");
  ok(typeof Request === "function", "Request should be defined");
  postMessage({ type: 'finish' });
}
