//This script ask you to enter your name on a keyboard and then print Hello + name or if no name entered Hello World is printed
let keyboard = require("keyboard");

keyboard.setHeader("Enter you name");

let name = keyboard.text(20, "", true);

if (name !== undefined) {
    print("Hello " + name);
}
else {
    print("Hello World");
}










