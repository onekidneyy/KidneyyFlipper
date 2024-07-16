//Randomly prints a number between 1 - 100
let Math = require("math");

function getRandomNumber(min, max) {
    return min + Math.floor(Math.random() * (max - min + 1));
}

function main() {
    let min = 1;
    let max = 100;
    let randomNumber = getRandomNumber(min, max);
    print("Random number: " + to_string(randomNumber));
}

main();

