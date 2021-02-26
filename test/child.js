async function main() {
  require('..');

  const durationMs = parseInt(process.argv[2], 10);

  burnFor(durationMs);

  await new Promise((resolve) => setImmediate(resolve));

  burnFor(durationMs);
}

function burnFor(durationMs) {
  const start = Date.now();

  while (Date.now() - start < durationMs) {
    let msg = '';
    for (let i = 0; i < 1000000; ++i) {
      msg += i;
    }
    if (msg.includes('potato')) {
      return;
    }
  }
}

main().catch((err) => {
  console.error(err);
  process.exit(2);
});
