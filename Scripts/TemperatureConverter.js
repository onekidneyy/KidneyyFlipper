//Not working yet in progress
let keyboard = require("keyboard");
let Math = require("math");

function main() {
    keyboard.setHeader("Enter temperature value");
    let tempString = keyboard.text(10, "", true);

    keyboard.setHeader("Convert to (C/F)");
    let unit = keyboard.text(10, "", true);

    if (unit === "C") {
        let convertedTemp = (tempString - 32) * 5 / 9;
        print(tempString + "F is " + Math.round(convertedTemp * 100) / 100 + "C");
    } else if (unit === "F") {
        let convertedTemp = (tempString * 9 / 5) + 32;
        print(tempString + "C is " + Math.round(convertedTemp * 100) / 100 + "F");
    } else {
        print("Invalid unit input. Please enter 'C' for Celsius or 'F' for Fahrenheit.");
    }
}

main();






