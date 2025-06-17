const { spawn } = require('child_process');


function dispense(coffee, sugar, creamer, water) {
    // Adjust the path to your 3motor binary as needed
    const proc = spawn('../RaspberryPi-5-hx711-cpp-/3motor');

    // Pipe values to the C++ program's stdin
    proc.stdin.write(`${coffee} ${sugar} ${creamer} ${water}\n`);
    proc.stdin.end();

    proc.stdout.on('data', data => process.stdout.write(data));
    proc.stderr.on('data', data => process.stderr.write(data));

    proc.on('close', code => {
        console.log('done');
    });
}

// Example usage:
dispense(45, 30, 28, 200);